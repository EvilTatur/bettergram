#include "bettergramservice.h"
#include "cryptopricelist.h"
#include "cryptoprice.h"
#include "rsschannellist.h"
#include "rsschannel.h"
#include "resourcegrouplist.h"
#include "aditem.h"

#include <messenger.h>
#include <settings.h>
#include <core/update_checker.h>
#include <core/click_handler_types.h>
#include <lang/lang_keys.h>
#include <styles/style_chat_helpers.h>
#include <platform/platform_specific.h>

#include <QCoreApplication>
#include <QTimer>
#include <QTimerEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

BettergramService *BettergramService::_instance = nullptr;
const QString BettergramService::_defaultLastUpdateString = "...";

const int BettergramService::_networkTimeout = 10 * 1000;

// We check for new updates in 2 minutes after application startup
const int BettergramService::_checkForFirstUpdatesDelay = 2 * 60 * 1000;

// We check for new updates every 10 hours
const int BettergramService::_checkForUpdatesPeriod = 10 * 60 * 60 * 1000;

// We update crypto price names every 3 days
const int BettergramService::_updateCryptoPriceNamesPeriod = 3 * 24 * 60 * 60 * 1000;

// We save crypto prices every 2 hours
const int BettergramService::_saveCryptoPricesPeriod = 2 * 60 * 60 * 1000;

BettergramService *BettergramService::init()
{
	return instance();
}

BettergramService *BettergramService::instance()
{
	if (!_instance) {
		new BettergramService();
	}

	return _instance;
}

int BettergramService::networkTimeout()
{
	return _networkTimeout;
}

const QString &BettergramService::defaultLastUpdateString()
{
	return _defaultLastUpdateString;
}

QString BettergramService::generateLastUpdateString(const QDateTime &dateTime, bool isShowSeconds)
{
	if (dateTime.isNull()) {
		return _defaultLastUpdateString;
	}

	qint64 daysBefore = QDateTime::currentDateTime().daysTo(dateTime);

	const QString timeFormat = isShowSeconds ? "hh:mm:ss" : "hh:mm";
	const QString timeString = dateTime.toString(timeFormat);

	if (daysBefore == 0) {
		return lng_player_message_today(lt_time, timeString);
	} else if (daysBefore == -1) {
		return lng_player_message_yesterday(lt_time, timeString);
	} else {
		return lng_player_message_date(lt_date,
									   langDayOfMonthFull(dateTime.date()),
									   lt_time,
									   timeString);
	}
}

void BettergramService::openUrl(const QUrl &url)
{
	QString urlString = url.toString();
	UrlClickHandler::Open(urlString);
}

void BettergramService::showBettergramTabs()
{
	Q_ASSERT(_instance);

	emit _instance->needToShowBettergramTabs();
}

Bettergram::BettergramService::BettergramService(QObject *parent) :
	QObject(parent),
	_cryptoPriceList(new CryptoPriceList(this)),
	_rssChannelList(new RssChannelList("news", st::newsPanImageWidth, st::newsPanImageHeight, this)),
	_videoChannelList(new RssChannelList("videos", st::videosPanImageWidth, st::videosPanImageHeight, this)),
	_resourceGroupList(new ResourceGroupList(this)),
	_currentAd(new AdItem(this))
{
	_instance = this;

	getIsPaid();
	getNextAd(true);

	_rssChannelList->load();

	if (_rssChannelList->isEmpty()) {
		_rssChannelList->add(QUrl("https://news.livecoinwatch.com/feed/"));
		_rssChannelList->add(QUrl("https://coincentral.com/feed/"));
		_rssChannelList->add(QUrl("https://www.coindesk.com/feed/"));
		_rssChannelList->add(QUrl("https://www.ccn.com/feed/"));
	}

	_videoChannelList->load();

	if (_videoChannelList->isEmpty()) {
		_videoChannelList->add(QUrl("https://www.youtube.com/feeds/videos.xml?channel_id=UCyC_4jvPzLiSkJkLIkA7B8g"));
	}

	connect(_rssChannelList, &RssChannelList::update,
			this, &BettergramService::onUpdateRssChannelList);

	connect(_videoChannelList, &RssChannelList::update,
			this, &BettergramService::onUpdateVideoChannelList);

	getRssChannelList();
	getVideoChannelList();

	_resourceGroupList->parseFile(":/bettergram/default-resources.json");
	getResourceGroupList();

	_cryptoPriceList->load();

	getCryptoPriceNames();

	_updateCryptoPriceNamesTimerId = startTimer(_updateCryptoPriceNamesPeriod, Qt::VeryCoarseTimer);
	_saveCryptoPricesTimerId = startTimer(_saveCryptoPricesPeriod, Qt::VeryCoarseTimer);

	connect(qApp, &QCoreApplication::aboutToQuit, this, [this] { _cryptoPriceList->save(); });

	QTimer::singleShot(_checkForFirstUpdatesDelay, Qt::VeryCoarseTimer,
					   this, [] { checkForNewUpdates(); });

	_checkForUpdatesTimerId = startTimer(_checkForUpdatesPeriod, Qt::VeryCoarseTimer);

	Platform::RegisterCustomScheme();

	_isSettingsPorted = bettergramSettings().value("isSettingsPorted").toBool();
}

void BettergramService::portSettingsFiles()
{
	if (_isSettingsPorted) {
		return;
	}

	QSettings oldSettings;
	QSettings newSettings = bettergramSettings();

	oldSettings.beginGroup(App::self()->phone());
	newSettings.beginGroup(App::self()->phone());

	oldSettings.beginGroup("favorites");
	newSettings.beginGroup("favorites");

	portSettingsFiles(oldSettings, newSettings);

	oldSettings.endGroup();
	newSettings.endGroup();

	oldSettings.beginGroup("pinned");
	newSettings.beginGroup("pinned");

	portSettingsFiles(oldSettings, newSettings);

	oldSettings.endGroup();
	newSettings.endGroup();

	newSettings.endGroup();

	newSettings.sync();

	oldSettings.remove("favorites");
	oldSettings.remove("pinned");

	oldSettings.endGroup();

	oldSettings.remove("last_tab");
	oldSettings.sync();

	newSettings.setValue("isSettingsPorted", true);
	newSettings.sync();

	_isSettingsPorted = true;
}

void BettergramService::portSettingsFiles(QSettings &oldSettings, QSettings &newSettings)
{
	for (const QString &key : oldSettings.allKeys()) {
		newSettings.setValue(key, oldSettings.value(key));
	}
}

bool BettergramService::isPaid() const
{
	return _isPaid;
}

void BettergramService::setIsPaid(bool isPaid)
{
	if (_isPaid != isPaid) {
		_isPaid = isPaid;

		emit isPaidChanged();
		_isPaidObservable.notify();
	}
}

BettergramService::BillingPlan BettergramService::billingPlan() const
{
	return _billingPlan;
}

void BettergramService::setBillingPlan(BillingPlan billingPlan)
{
	if (_billingPlan != billingPlan) {
		_billingPlan = billingPlan;

		emit billingPlanChanged();
		_billingPlanObservable.notify();
	}
}

CryptoPriceList *BettergramService::cryptoPriceList() const
{
	return _cryptoPriceList;
}

RssChannelList *BettergramService::rssChannelList() const
{
	return _rssChannelList;
}

RssChannelList *BettergramService::videoChannelList() const
{
	return _videoChannelList;
}

ResourceGroupList *BettergramService::resourceGroupList() const
{
	return _resourceGroupList;
}

AdItem *BettergramService::currentAd() const
{
	return _currentAd;
}

bool BettergramService::isWindowActive() const
{
	return _isWindowActive;
}

void BettergramService::setIsWindowActive(bool isWindowActive)
{
	if (_isWindowActive != isWindowActive) {
		_isWindowActive = isWindowActive;

		if (_isWindowActiveHandler) {
			_isWindowActiveHandler();
		}
	}
}

base::Observable<void> &BettergramService::isPaidObservable()
{
	return _isPaidObservable;
}

base::Observable<void> &BettergramService::billingPlanObservable()
{
	return _billingPlanObservable;
}

QString BettergramService::settingsDirPath() const
{
	return cWorkingDir() + QStringLiteral("tdata/bettergram/");
}

QString BettergramService::cacheDirPath() const
{
	return settingsDirPath() + QStringLiteral("cache/");
}

QString BettergramService::pricesCacheDirPath() const
{
	return cacheDirPath() + QStringLiteral("prices/");
}

QString BettergramService::pricesIconsCacheDirPath() const
{
	return pricesCacheDirPath() + QStringLiteral("icons/");
}

QString BettergramService::settingsPath(const QString &name) const
{
	return settingsDirPath() + name + QStringLiteral(".ini");
}

QSettings BettergramService::settings(const QString &name) const
{
	return QSettings(settingsPath(name), QSettings::IniFormat);
}

QSettings BettergramService::bettergramSettings() const
{
	return settings(QStringLiteral("bettergram"));
}

QSettings BettergramService::pricesCacheSettings() const
{
	return QSettings(pricesCacheDirPath() + QStringLiteral("prices.ini"));
}

void BettergramService::getIsPaid()
{
	//TODO: bettergram: ask server and get know if the instance is paid or not and the current billing plan.
	//					If the application is not paid then call getNextAd();
}

void BettergramService::getCryptoPriceNames()
{
	QUrl url("https://http-api.livecoinwatch.com/currencies");

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetCryptoPriceNamesFinished);

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(_networkTimeout, Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply] {
		reply->deleteLater();
		networkManager->deleteLater();

		LOG(("Can not get crypto price names due timeout"));
	});

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetCryptoPriceNamesSslFailed);
}

QUrl BettergramService::getCryptoPriceValues(int offset, int count)
{
	if (offset < 0 || offset >= _cryptoPriceList->count() || count <= 0) {
		return QUrl();
	}

	QUrl url(QStringLiteral("https://http-api.livecoinwatch.com/bettergram/coins?sort=%1&order=%2&offset=%3&limit=%4")
			 .arg(_cryptoPriceList->sortString())
			 .arg(_cryptoPriceList->orderString())
			 .arg(offset)
			 .arg(count));

	getCryptoPriceValues(url, QStringList());

	return url;
}

QUrl BettergramService::getCryptoPriceValues(const QStringList &shortNames)
{
	if (shortNames.isEmpty()) {
		return QUrl();
	}

	//TODO: bettergram: remove 'limit=1' as soon as the server side will be fixed

	QUrl url(QStringLiteral("https://http-api.livecoinwatch.com/bettergram/coins?limit=1&favorites=%1")
			 .arg(shortNames.join(QStringLiteral(","))));

	getCryptoPriceValues(url, shortNames);

	return url;
}

void BettergramService::getCryptoPriceValues(const QUrl &url, const QStringList &shortNames)
{
	if (!_cryptoPriceList->areNamesFetched()) {
		getCryptoPriceNames();
		return;
	}

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished,
			this, [this, url, shortNames] {
		QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

		if(reply->error() == QNetworkReply::NoError) {
			_cryptoPriceList->parseValues(reply->readAll(), url, shortNames);
		} else {
			LOG(("Can not get crypto price values. %1 (%2)")
				.arg(reply->errorString())
				.arg(reply->error()));
		}
	});

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(_networkTimeout, Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply] {
		reply->deleteLater();
		networkManager->deleteLater();

		LOG(("Can not get crypto price values due timeout"));
	});

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetCryptoPriceValuesSslFailed);
}

void BettergramService::getRssChannelList()
{
	for (const QSharedPointer<RssChannel> &channel : *_rssChannelList) {
		if (channel->isMayFetchNewData()) {
			getRssFeeds(_rssChannelList, channel);
		}
	}
}

void BettergramService::getVideoChannelList()
{
	for (const QSharedPointer<RssChannel> &channel : *_videoChannelList) {
		if (channel->isMayFetchNewData()) {
			getRssFeeds(_videoChannelList, channel);
		}
	}
}

void BettergramService::getRssFeeds(RssChannelList *rssChannelList,
									const QSharedPointer<RssChannel> &channel)
{
	channel->startFetching();

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(channel->feedLink());

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished, this, [rssChannelList, reply, channel] {
		if(reply->error() == QNetworkReply::NoError) {
			channel->fetchingSucceed(reply->readAll());
		} else {
			LOG(("Can not get RSS feeds from the channel %1. %2 (%3)")
				.arg(channel->feedLink().toString())
				.arg(reply->errorString())
				.arg(reply->error()));

			channel->fetchingFailed();
		}

		rssChannelList->parse();
	});

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(_networkTimeout, Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply, channel] {
		reply->deleteLater();
		networkManager->deleteLater();

		LOG(("Can not get RSS feeds from the channel %1 due timeout")
			.arg(channel->feedLink().toString()));

		channel->fetchingFailed();
	});

	connect(reply, &QNetworkReply::sslErrors, this, [channel] (QList<QSslError> errors) {
		LOG(("Got SSL errors in during getting RSS feeds from the channel: %1")
			.arg(channel->feedLink().toString()));

		for(const QSslError &error : errors) {
			LOG(("%1").arg(error.errorString()));
		}
	});
}

void BettergramService::getResourceGroupList()
{
	//TODO: bettergram: We should get resources from server when it will be ready on server side
#if 0
	QUrl url("https://api.bettergram.io/v1/resources");

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetResourceGroupListFinished);

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(_networkTimeout, Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply] {
		reply->deleteLater();
		networkManager->deleteLater();

		LOG(("Can not get resource group list due timeout"));
	});

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetResourceGroupListSslFailed);
#endif
}

void BettergramService::onGetCryptoPriceNamesFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		_cryptoPriceList->parseNames(reply->readAll());
	} else {
		LOG(("Can not get crypto price names. %1 (%2)")
			.arg(reply->errorString())
			.arg(reply->error()));
	}
}

void BettergramService::onGetCryptoPriceNamesSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::onGetCryptoPriceValuesSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::onGetResourceGroupListFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		_resourceGroupList->parse(reply->readAll());
	} else {
		LOG(("Can not get resource group list. %1 (%2)")
			.arg(reply->errorString())
			.arg(reply->error()));
	}
}

void BettergramService::onGetResourceGroupListSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::getNextAd(bool reset)
{
	if(_isPaid) {
		_currentAd->clear();
		return;
	}

	QString url = "https://api.bettergram.io/v1/ads/next";

	if (!reset && !_currentAd->isEmpty()) {
		url += "?last=";
		url += _currentAd->id();
	}

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(url);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished,
			this, &BettergramService::onGetNextAdFinished);

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	connect(this, &BettergramService::destroyed, networkManager, [networkManager, reply] {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(_networkTimeout, Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply, this] {
		reply->deleteLater();
		networkManager->deleteLater();

		getNextAdLater();
	});

	connect(reply, &QNetworkReply::sslErrors,
			this, &BettergramService::onGetNextAdSslFailed);
}

void BettergramService::getNextAdLater(bool reset)
{
	int delay = _currentAd->duration();

	if (delay <= 0) {
		delay = AdItem::defaultDuration();
	}

	QTimer::singleShot(delay * 1000, this, [this, reset]() {
		if (_isWindowActive) {
			_isWindowActiveHandler = nullptr;
			getNextAd(reset);
		} else {
			_isWindowActiveHandler = [this, reset]() {
				if (_isWindowActive) {
					_isWindowActiveHandler = nullptr;
					getNextAd(reset);
				}
			};
		}
	});
}

bool BettergramService::parseNextAd(const QByteArray &byteArray)
{
	if (byteArray.isEmpty()) {
		LOG(("Can not get next ad. Response is emtpy"));
		return false;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get next ad. Response is wrong. %1 (%2). Response: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));
		return false;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get next ad. Response is emtpy or wrong"));
		return false;
	}

	bool isSuccess = json.value("success").toBool();

	if (!isSuccess) {
		QString errorMessage = json.value("message").toString("Unknown error");
		LOG(("Can not get next ad. %1").arg(errorMessage));
		return false;
	}

	QJsonObject adJson = json.value("ad").toObject();

	if (adJson.isEmpty()) {
		LOG(("Can not get next ad. Ad json is empty"));
		return false;
	}

	QString id = adJson.value("_id").toString();
	if (id.isEmpty()) {
		LOG(("Can not get next ad. Id is empty"));
		return false;
	}

	QString text = adJson.value("text").toString();
	if (text.isEmpty()) {
		LOG(("Can not get next ad. Text is empty"));
		return false;
	}

	QString url = adJson.value("url").toString();
	if (url.isEmpty()) {
		LOG(("Can not get next ad. Url is empty"));
		return false;
	}

	int duration = adJson.value("duration").toInt(AdItem::defaultDuration());

	AdItem adItem(id, text, url, duration);

	_currentAd->update(adItem);

	return true;
}

void BettergramService::onGetNextAdFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if(reply->error() == QNetworkReply::NoError) {
		if (parseNextAd(reply->readAll())) {
			getNextAdLater();
		} else {
			// Try to get new ad without previous ad id
			getNextAdLater(true);
		}
	} else {
		//	LOG(("Can not get next ad item. %1 (%2)")
		//				  .arg(reply->errorString())
		//				  .arg(reply->error()));

		getNextAdLater();
	}
}

void BettergramService::onGetNextAdSslFailed(QList<QSslError> errors)
{
	for(const QSslError &error : errors) {
		LOG(("%1").arg(error.errorString()));
	}
}

void BettergramService::onUpdateRssChannelList()
{
	getRssChannelList();
}

void BettergramService::onUpdateVideoChannelList()
{
	getVideoChannelList();
}

void BettergramService::checkForNewUpdates()
{
	LOG(("Check for new updates"));

	// We got this code from UpdateStateRow::onCheck() slot

	if (!cAutoUpdate()) {
		return;
	}

	Core::UpdateChecker checker;

	cSetLastUpdateCheck(0);
	checker.start();
}

void BettergramService::timerEvent(QTimerEvent *timerEvent)
{
	if (timerEvent->timerId() == _checkForUpdatesTimerId) {
		checkForNewUpdates();
	} else if (timerEvent->timerId() == _updateCryptoPriceNamesTimerId) {
		getCryptoPriceNames();
	} else if (timerEvent->timerId() == _saveCryptoPricesTimerId) {
		_cryptoPriceList->save();
	}
}

} // namespace Bettergrams
