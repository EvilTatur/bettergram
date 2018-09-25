#include "cryptopricelist.h"
#include "cryptoprice.h"

#include <bettergram/bettergramservice.h>
#include <logs.h>

#include <QJsonDocument>
#include <QJsonObject>

namespace Bettergram {

const int CryptoPriceList::_defaultFreq = 60;

CryptoPriceList::CryptoPriceList(QObject *parent) :
	QObject(parent),
	_freq(_defaultFreq),
	_lastUpdateString(BettergramService::defaultLastUpdateString())
{
}

double CryptoPriceList::marketCap() const
{
	return _marketCap;
}

QString CryptoPriceList::marketCapString() const
{
	QString result;
	qint64 value = qAbs(qRound64(_marketCap));

	if (!value) {
		result = "0";
		return result;
	}

	while (true) {
		qint64 temp = value % 1000;
		value /= 1000;

		if (!result.isEmpty()) {
			result.prepend(",");
		}

		result.prepend(QString("%1").arg(temp, 3, 10, QLatin1Char('0')));

		if (!value) {
			break;
		}
	}

	return result;
}

void CryptoPriceList::setMarketCap(double marketCap)
{
	if (_marketCap != marketCap) {
		_marketCap = marketCap;
		emit marketCapChanged();
	}
}

int CryptoPriceList::freq() const
{
	return _freq;
}

void CryptoPriceList::setFreq(int freq)
{
	if (freq <= 0) {
		freq = _defaultFreq;
	}

	if (_freq != freq) {
		_freq = freq;
		emit freqChanged();
	}
}

QDateTime CryptoPriceList::lastUpdate() const
{
	return _lastUpdate;
}

QString CryptoPriceList::lastUpdateString() const
{
	return _lastUpdateString;
}

void CryptoPriceList::setLastUpdate(const QDateTime &lastUpdate)
{
	if (_lastUpdate != lastUpdate) {
		_lastUpdate = lastUpdate;

		_lastUpdateString = BettergramService::generateLastUpdateString(_lastUpdate, true);
	}
}

void CryptoPriceList::addPrivate(CryptoPrice *price)
{
	connect(price, &CryptoPrice::iconChanged, this, &CryptoPriceList::updated);

	_list.push_back(price);
}

CryptoPriceList::const_iterator CryptoPriceList::begin() const
{
	return _list.begin();
}

CryptoPriceList::const_iterator CryptoPriceList::end() const
{
	return _list.end();
}

CryptoPrice *CryptoPriceList::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		return nullptr;
	}

	return _list.at(index);
}

int CryptoPriceList::count() const
{
	return _list.count();
}

CryptoPriceList::SortOrder CryptoPriceList::sortOrder() const
{
	return _sortOrder;
}

void CryptoPriceList::setSortOrder(const SortOrder &sortOrder)
{
	if (_sortOrder != sortOrder) {
		_sortOrder = sortOrder;

		sort();
		emit sortOrderChanged();
	}
}

bool CryptoPriceList::areNamesFetched() const
{
	return _areNamesFetched;
}

void CryptoPriceList::setAreNamesFetched(bool areNamesFetched)
{
	if (_areNamesFetched != areNamesFetched) {
		_areNamesFetched = areNamesFetched;
	}
}

void CryptoPriceList::parseNames(const QByteArray &byteArray)
{
	setAreNamesFetched(false);

	if (byteArray.isEmpty()) {
		LOG(("Can not get crypto price names. Response is emtpy"));
		return;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get crypto price names. Response is wrong. %1 (%2). Response: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));
		return;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get crypto price names. Response is emtpy or wrong"));
		return;
	}

	bool success = json.value("success").toBool();

	if (!success) {
		QString errorMessage = json.value("message").toString("Unknown error");
		LOG(("Can not get crypto price names. %1").arg(errorMessage));
		return;
	}

	QJsonArray priceListJson = json.value("data").toArray();

	if (priceListJson.isEmpty()) {
		LOG(("Can not get crypto price names. The 'data' list is empty"));
		return;
	}

	QList<CryptoPrice> priceList;
	bool isAdded = false;

	for (QJsonValue jsonValue : priceListJson) {
		QJsonObject priceJson = jsonValue.toObject();

		if (priceJson.isEmpty()) {
			LOG(("Price json is empty"));
			continue;
		}

		QString url = priceJson.value("url").toString();
		if (url.isEmpty()) {
			continue;
		}

		QString iconUrl = priceJson.value("icon").toString();
		if (iconUrl.isEmpty()) {
			continue;
		}

		QString name = priceJson.value("name").toString();
		if (name.isEmpty()) {
			continue;
		}

		QString shortName = priceJson.value("code").toString();
		if (shortName.isEmpty()) {
			continue;
		}

		CryptoPrice cryptoPrice(url, iconUrl, name, shortName, false);

		priceList.push_back(cryptoPrice);

		isAdded = true;
	}

	mergeCryptoPriceList(priceList);

	if (!_list.isEmpty() && isAdded) {
		setAreNamesFetched(true);
	}
}

void CryptoPriceList::parseValues(const QByteArray &byteArray)
{
	if (byteArray.isEmpty()) {
		LOG(("Can not get crypto price values. Response is emtpy"));
		return;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get crypto price values. Response is wrong. %1 (%2). Response: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));
		return;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get crypto price values. Response is emtpy or wrong"));
		return;
	}

	bool success = json.value("success").toBool();

	if (!success) {
		QString errorMessage = json.value("message").toString("Unknown error");
		LOG(("Can not get crypto price values. %1").arg(errorMessage));
		return;
	}

	double marketCap = json.value("cap").toDouble();

	// It is optionally parameter.
	// This parameter may contain number of seconds for the next update
	// (5, 60, 90 seconds and etc.).
	int freq = qAbs(json.value("freq").toInt());

	// Now we support two types of requests to server:
	// 1. Enumerate required prices
	// 2. Sort prices at the server side and request them at offset and count
	//
	// At first case we get results at the `favorites` property,
	// at the second case we get results at the `list` property.

	QJsonArray favoriteListJson = json.value("data").toObject().value("favorites").toArray();

	if (favoriteListJson.isEmpty) {
		QJsonArray priceListJson = json.value("data").toObject().value("list").toArray();

		parsePriceListValues(priceListJson);
	} else {
		parsePriceListValues(favoriteListJson);
	}

	updateData(marketCap, freq);
}

void CryptoPriceList::parsePriceListValues(const QJsonArray &priceListJson)
{
	int i = 0;

	for (QJsonValue jsonValue : priceListJson) {
		QJsonObject priceJson = jsonValue.toObject();

		if (priceJson.isEmpty()) {
			LOG(("Price json is empty"));
			continue;
		}

		QString shortName = priceJson.value("code").toString();
		if (shortName.isEmpty()) {
			LOG(("Price code is empty"));
			continue;
		}

		QJsonObject deltaJson = priceJson.value("delta").toObject();

		if (deltaJson.isEmpty()) {
			LOG(("Price delta is empty"));
			continue;
		}

		int rank = priceJson.contains("rank") ? priceJson.value("rank").toInt() : i;
		double currentPrice = priceJson.value("price").toDouble();

		double changeFor24Hours = deltaJson.value("day").toDouble();
		double changeForMinute = deltaJson.value("minute").toDouble();

		CryptoPrice *price = findByShortName(shortName);

		if (!price) {
			LOG(("Can not find price for crypto currency '%1'").arg(shortName));
			continue;
		}

		price->setRank(rank);
		price->setCurrentPrice(currentPrice);
		price->setChangeFor24Hours(changeFor24Hours);
		price->setMinuteDirection(CryptoPrice::countDirection(changeForMinute));

		i++;
	}
}

void CryptoPriceList::save() const
{
	QSettings settings = BettergramService::instance()->pricesSettings();

	settings.beginGroup("metadata");

	settings.setValue("marketCap", marketCap());
	settings.setValue("lastUpdate", lastUpdate());

	if (freq() == _defaultFreq) {
		settings.remove("freq");
	} else {
		settings.setValue("freq", freq());
	}

	settings.endGroup();

	settings.beginGroup("prices");
	settings.beginWriteArray("prices", _list.size());

	for (int i = 0; i < _list.size(); ++i) {
		settings.setArrayIndex(i);
		_list.at(i)->save(settings);
	}

	settings.endArray();
	settings.endGroup();
}

void CryptoPriceList::load()
{
	QSettings settings = BettergramService::instance()->pricesSettings();

	settings.beginGroup("metadata");

	setMarketCap(settings.value("marketCap").toDouble());
	setFreq(qAbs(settings.value("freq").toInt()));
	setLastUpdate(settings.value("lastUpdate").toDateTime());

	settings.endGroup();

	settings.beginGroup("prices");
	int size = settings.beginReadArray("prices");

	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);

		CryptoPrice *price = CryptoPrice::load(settings);

		if (price) {
			addPrivate(price);
		}
	}

	settings.endArray();
	settings.endGroup();
}

void CryptoPriceList::updateData(double marketCap, int freq)
{
	setMarketCap(marketCap);
	setFreq(freq);
	setLastUpdate(QDateTime::currentDateTime());
}

void CryptoPriceList::mergeCryptoPriceList(const QList<CryptoPrice> &priceList)
{
	// Remove old crypto prices
	for (QList<CryptoPrice*>::iterator it = _list.begin(); it != _list.end();) {
		CryptoPrice *price = *it;

		if (containsName(priceList, price->name())) {
			++it;
		} else {
			price->deleteLater();
			it = _list.erase(it);
		}
	}

	// Update existed crypto prices and add new ones
	for (const CryptoPrice &price : priceList) {
		CryptoPrice *existedPrice = findByName(price.name(), price.shortName());

		if (existedPrice) {
			existedPrice->updateData(price);
		} else {
			addPrivate(new CryptoPrice(price, this));
		}
	}

	sort();
}

CryptoPrice *CryptoPriceList::findByName(const QString &name, const QString &shortName)
{
	for (CryptoPrice *price : _list) {
		if (price->name() == name && price->shortName() == shortName) {
			return price;
		}
	}

	return nullptr;
}

CryptoPrice *CryptoPriceList::findByShortName(const QString &shortName)
{
	for (CryptoPrice *price : _list) {
		if (price->shortName() == shortName) {
			return price;
		}
	}

	return nullptr;
}

bool CryptoPriceList::containsName(const QList<CryptoPrice> &priceList, const QString &name)
{
	for (const CryptoPrice &price : priceList) {
		if (price.name() == name) {
			return true;
		}
	}

	return false;
}

bool CryptoPriceList::sortByRankAsc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->rank() == price2->rank()) {
		return sortByNameAsc(price1, price2);
	}

	return price1->rank() < price2->rank();
}

bool CryptoPriceList::sortByRankDesc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->rank() == price2->rank()) {
		return sortByNameAsc(price1, price2);
	}

	return price2->rank() < price1->rank();
}

bool CryptoPriceList::sortByNameAsc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	return QString::compare(price1->name(), price2->name(), Qt::CaseInsensitive) < 0;
}

bool CryptoPriceList::sortByNameDesc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	return QString::compare(price2->name(), price1->name(), Qt::CaseInsensitive) < 0;
}

bool CryptoPriceList::sortByPriceAsc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->currentPrice() == price2->currentPrice()) {
		return sortByNameAsc(price1, price2);
	}

	return price1->currentPrice() < price2->currentPrice();
}

bool CryptoPriceList::sortByPriceDesc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->currentPrice() == price2->currentPrice()) {
		return sortByNameAsc(price1, price2);
	}

	return price2->currentPrice() < price1->currentPrice();
}

bool CryptoPriceList::sortBy24hAsc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->changeFor24Hours() == price2->changeFor24Hours()) {
		return sortByNameAsc(price1, price2);
	}

	return price1->changeFor24Hours() < price2->changeFor24Hours();
}

bool CryptoPriceList::sortBy24hDesc(const CryptoPrice *price1, const CryptoPrice *price2)
{
	if (price1->changeFor24Hours() == price2->changeFor24Hours()) {
		return sortByNameAsc(price1, price2);
	}

	return price2->changeFor24Hours() < price1->changeFor24Hours();
}

void CryptoPriceList::sort()
{
	switch (_sortOrder) {
	case SortOrder::Rank:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortByRankAsc);
		break;
	case SortOrder::NameAscending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortByNameAsc);
		break;
	case SortOrder::NameDescending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortByNameDesc);
		break;
	case SortOrder::PriceAscending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortByPriceAsc);
		break;
	case SortOrder::PriceDescending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortByPriceDesc);
		break;
	case SortOrder::ChangeFor24hAscending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortBy24hAsc);
		break;
	case SortOrder::ChangeFor24hDescending:
		std::sort(_list.begin(), _list.end(), &CryptoPriceList::sortBy24hDesc);
		break;
	default:
		break;
	}

	emit updated();
}

void CryptoPriceList::createTestData()
{
	clear();

	addTestData(QUrl("https://www.livecoinwatch.com/price/Bitcoin-BTC"), QUrl("https://www.livecoinwatch.com/images/icons32/btc.png"), "Bitcoin", "BTC", 0, 7935.96, -3.22, CryptoPrice::Direction::Down);
	addTestData(QUrl("https://www.livecoinwatch.com/price/Ethereum-ETH"), QUrl("https://www.livecoinwatch.com/images/icons32/eth.png"), "Ethereum", "ETH", 3, 625.64, -8.43, CryptoPrice::Direction::Down);
	addTestData(QUrl("https://www.livecoinwatch.com/price/Ripple-XRP"), QUrl("https://www.livecoinwatch.com/images/icons32/xrp.png"), "Ripple", "XRP", 2, 0.6246, -6.59, CryptoPrice::Direction::Up);
	addTestData(QUrl("https://www.livecoinwatch.com/price/BitcoinCash-BCH"), QUrl("https://www.livecoinwatch.com/images/icons32/bch.png"), "Bitcoin Cash", "BCH", 1, 10640.2856, 9.45, CryptoPrice::Direction::Up);
	addTestData(QUrl("https://www.livecoinwatch.com/price/EOS-EOS"), QUrl("https://www.livecoinwatch.com/images/icons32/eos.png"), "EOS", "EOS", 4, 11.5, -100.6358, CryptoPrice::Direction::None);
	addTestData(QUrl("https://www.livecoinwatch.com/price/Litecoin-LTC"), QUrl("https://www.livecoinwatch.com/images/icons32/ltc.png"), "Litecoin", "LTC", 5, 125.46, -5.28, CryptoPrice::Direction::Up);
	addTestData(QUrl("https://www.livecoinwatch.com/price/Cardano-ADA"), QUrl("https://www.livecoinwatch.com/images/icons32/ada.png"), "Cardano", "ADA", 6, 0.2151, -9.72, CryptoPrice::Direction::None);
	addTestData(QUrl("https://www.livecoinwatch.com/price/Stellar-XLM"), QUrl("https://www.livecoinwatch.com/images/icons32/xlm.png"), "Stellar", "XLM", 7, 0.2882, -6.81, CryptoPrice::Direction::None);
	addTestData(QUrl("https://www.livecoinwatch.com/price/TRON-TRX"), QUrl("https://www.livecoinwatch.com/images/icons32/trx.png"), "TRON", "TRX", 8, 0.071258, -8.4789, CryptoPrice::Direction::Down);
	addTestData(QUrl("https://www.livecoinwatch.com/price/IOTA-MIOTA"), QUrl("https://www.livecoinwatch.com/images/icons32/miota.png"), "IOTA", "MIOTA", 9, 1.55, 8.77, CryptoPrice::Direction::Up);
}

void CryptoPriceList::addTestData(const QUrl &url,
								  const QUrl &iconUrl,
								  const QString &name,
								  const QString &shortName,
								  int rank,
								  double currentPrice,
								  double changeFor24Hours,
								  CryptoPrice::Direction minuteDirection)
{
	CryptoPrice *price = new CryptoPrice(url,
										 iconUrl,
										 name,
										 shortName,
										 rank,
										 currentPrice,
										 changeFor24Hours,
										 minuteDirection,
										 true,
										 this);

	_list.push_back(price);
}

void CryptoPriceList::clear()
{
	while (!_list.isEmpty()) {
		_list.takeFirst()->deleteLater();
	}
}

} // namespace Bettergrams
