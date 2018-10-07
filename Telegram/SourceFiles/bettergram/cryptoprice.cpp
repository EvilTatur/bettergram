#include "cryptoprice.h"
#include "remoteimage.h"

#include <styles/style_chat_helpers.h>

#include <QSettings>

namespace Bettergram {

CryptoPrice::CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 bool isNeedDownloadIcon,
						 QObject *parent) :
	QObject(parent),
	_url(url),
	_icon(new RemoteImage(iconUrl,
						  st::pricesPanTableImageSize,
						  st::pricesPanTableImageSize,
						  isNeedDownloadIcon)),
	_name(name),
	_shortName(shortName)
{
	updateCurrentPriceString();
	updateChangeFor24HoursString();

	connect(_icon.data(), &RemoteImage::imageChanged, this, &CryptoPrice::iconChanged);
}

CryptoPrice::CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 int rank,
						 double currentPrice,
						 double changeFor24Hours,
						 Direction minuteDirection,
						 bool isNeedDownloadIcon,
						 QObject *parent) :
	QObject(parent),
	_url(url),
	_icon(new RemoteImage(iconUrl,
						  st::pricesPanTableImageSize,
						  st::pricesPanTableImageSize,
						  isNeedDownloadIcon)),
	_name(name),
	_shortName(shortName),
	_rank(rank),
	_currentPrice(currentPrice),
	_changeFor24Hours(changeFor24Hours),
	_minuteDirection(minuteDirection),
	_dayDirection(countDirection(_changeFor24Hours))
{
	updateCurrentPriceString();
	updateChangeFor24HoursString();

	connect(_icon.data(), &RemoteImage::imageChanged, this, &CryptoPrice::iconChanged);
}

CryptoPrice::CryptoPrice(const CryptoPrice &price, QObject *parent) :
	QObject(parent),
	_url(price._url),
	_icon(price._icon),
	_name(price._name),
	_shortName(price._shortName),
	_rank(price._rank),
	_currentPrice(price._currentPrice),
	_currentPriceString(price._currentPriceString),
	_changeFor24Hours(price._changeFor24Hours),
	_changeFor24HoursString(price._changeFor24HoursString),
	_minuteDirection(price._minuteDirection),
	_dayDirection(price._dayDirection)
{
	connect(_icon.data(), &RemoteImage::imageChanged, this, &CryptoPrice::iconChanged);
}

CryptoPrice &CryptoPrice::operator=(const CryptoPrice &price)
{
	setUrl(price._url);
	setIcon(price._icon);
	setName(price._name);
	setShortName(price._shortName);
	setRank(price._rank);
	setCurrentPrice(price._currentPrice);
	setChangeFor24Hours(price._changeFor24Hours);
	setMinuteDirection(price._minuteDirection);

	return *this;
}

const QUrl &CryptoPrice::url() const
{
	return _url;
}

void CryptoPrice::setUrl(const QUrl &url)
{
	_url = url;
}

void CryptoPrice::setIcon(const QSharedPointer<RemoteImage> &icon)
{
	_icon = icon;
}

const QUrl &CryptoPrice::iconUrl() const
{
	return _icon->link();
}

const QPixmap &CryptoPrice::icon() const
{
	return _icon->image();
}

const QString &CryptoPrice::name() const
{
	return _name;
}

void CryptoPrice::setName(const QString &name)
{
	_name = name;
}

const QString &CryptoPrice::shortName() const
{
	return _shortName;
}

void CryptoPrice::setShortName(const QString &shortName)
{
	_shortName = shortName;
}

int CryptoPrice::rank() const
{
	return _rank;
}

void CryptoPrice::setRank(int rank)
{
	if (_rank != rank) {
		_rank = rank;
		emit rankChanged();
	}
}

double CryptoPrice::currentPrice() const
{
	return _currentPrice;
}

QString CryptoPrice::currentPriceString() const
{
	return _currentPriceString;
}

void CryptoPrice::setCurrentPrice(double currentPrice)
{
	if (_currentPrice != currentPrice) {
		_currentPrice = currentPrice;
		updateCurrentPriceString();

		emit currentPriceChanged();
	}
}

void CryptoPrice::updateCurrentPriceString()
{
	if (std::isnan(_currentPrice)) {
		_currentPriceString = QStringLiteral("N/A");
	} else if (_currentPrice < 1.0) {
		_currentPriceString = QStringLiteral("$%1").arg(_currentPrice, 0, 'f', 4);
	} else {
		_currentPriceString = QStringLiteral("$%1").arg(_currentPrice, 0, 'f', 2);
	}
}

double CryptoPrice::changeFor24Hours() const
{
	return _changeFor24Hours;
}

QString CryptoPrice::changeFor24HoursString() const
{
	return _changeFor24HoursString;
}

void CryptoPrice::setChangeFor24Hours(double changeFor24Hours)
{
	if (_changeFor24Hours != changeFor24Hours) {
		_changeFor24Hours = changeFor24Hours;
		updateChangeFor24HoursString();

		setDayDirection(countDirection(_changeFor24Hours));
		emit changeFor24HoursChanged();
	}
}

void CryptoPrice::updateChangeFor24HoursString()
{
	if (std::isnan(_changeFor24Hours)) {
		_changeFor24HoursString = QStringLiteral("N/A");
	} else {
		_changeFor24HoursString = QStringLiteral("%1%").arg(_changeFor24Hours, 0, 'f', 2);
	}

}

CryptoPrice::Direction CryptoPrice::minuteDirection() const
{
	return _minuteDirection;
}

void CryptoPrice::setMinuteDirection(Direction minuteDirection)
{
	if (_minuteDirection != minuteDirection) {
		_minuteDirection = minuteDirection;
		emit minuteDirectionChanged();
	}
}

CryptoPrice::Direction CryptoPrice::dayDirection() const
{
	return _dayDirection;
}

void CryptoPrice::setDayDirection(Direction dayDirection)
{
	if (_dayDirection != dayDirection) {
		_dayDirection = dayDirection;
		emit dayDirectionChanged();
	}
}

bool CryptoPrice::isEmpty() const
{
	return !_url.isValid() || !_icon->link().isValid() || _name.isEmpty() || _shortName.isEmpty();
}

void CryptoPrice::updateData(const CryptoPrice &price)
{
	setCurrentPrice(price.currentPrice());
	setChangeFor24Hours(price.changeFor24Hours());
	setMinuteDirection(price.minuteDirection());
}

void CryptoPrice::resetValues()
{
	setCurrentPrice(std::numeric_limits<double>::quiet_NaN());
	setChangeFor24Hours(std::numeric_limits<double>::quiet_NaN());
}

void CryptoPrice::save(QSettings &settings) const
{
	settings.setValue("link", url().toString());
	settings.setValue("iconLink", iconUrl().toString());
	settings.setValue("name", name());
	settings.setValue("code", shortName());
	settings.setValue("rank", rank());
	settings.setValue("price", currentPrice());
	settings.setValue("changeForDay", changeFor24Hours());
	settings.setValue("minuteDirection", static_cast<int>(minuteDirection()));
}

QSharedPointer<CryptoPrice> CryptoPrice::load(const QSettings &settings)
{
	QString name = settings.value("name").toString();
	if (name.isEmpty()) {
		LOG(("Price name is empty"));
		return QSharedPointer<CryptoPrice>(nullptr);
	}

	QString shortName = settings.value("code").toString();
	if (shortName.isEmpty()) {
		LOG(("Price code is empty"));
		return QSharedPointer<CryptoPrice>(nullptr);
	}

	QString url = settings.value("link").toString();
	if (url.isEmpty()) {
		LOG(("Price url is empty"));
		return QSharedPointer<CryptoPrice>(nullptr);
	}

	QString iconUrl = settings.value("iconLink").toString();
	if (iconUrl.isEmpty()) {
		LOG(("Price icon url is empty"));
		return QSharedPointer<CryptoPrice>(nullptr);
	}

	int rank = settings.value("rank").toInt();
	double price = settings.value("price").toDouble();
	double changeFor24Hours = settings.value("changeForDay").toDouble();

	Direction minuteDirection = Direction::None;

	switch (settings.value("minuteDirection").toInt()) {
	case(static_cast<int>(Direction::Up)):
		minuteDirection = Direction::Up;
		break;
	case(static_cast<int>(Direction::Down)):
		minuteDirection = Direction::Down;
		break;
	default:
		minuteDirection = Direction::None;
	}

	return QSharedPointer<CryptoPrice>(new CryptoPrice(url,
													   iconUrl,
													   name,
													   shortName,
													   rank,
													   price,
													   changeFor24Hours,
													   minuteDirection,
													   false));
}

CryptoPrice::Direction CryptoPrice::countDirection(double value)
{
	if (std::isnan(value)) {
		return Direction::None;
	} else if (value > 0.0) {
		return Direction::Up;
	} else if (value < 0.0) {
		return Direction::Down;
	} else {
		return Direction::None;
	}
}

} // namespace Bettergrams
