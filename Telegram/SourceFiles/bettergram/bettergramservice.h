#pragma once

#include <base/observer.h>

#include <QObject>
#include <QSettings>

#include <functional>

namespace Bettergram {

class CryptoPriceList;
class RssChannelList;
class RssChannel;
class ResourceGroupList;
class AdItem;

/**
 * @brief The BettergramService class contains Bettergram specific classes and settings
 */
class BettergramService : public QObject {
	Q_OBJECT

public:
	enum class BillingPlan {
		Unknown,
		Monthly,
		Quarterly,
		Yearly
	};

	static BettergramService *init();
	static BettergramService *instance();

	static int networkTimeout();

	static const QString &defaultLastUpdateString();
	static QString generateLastUpdateString(const QDateTime &dateTime, bool isShowSeconds);

	static void openUrl(const QUrl &url);

	static bool isBettergramTabsShowed();
	static void toggleBettergramTabs();

	bool isPaid() const;
	BillingPlan billingPlan() const;

	CryptoPriceList *cryptoPriceList() const;
	RssChannelList *rssChannelList() const;
	RssChannelList *videoChannelList() const;
	ResourceGroupList *resourceGroupList() const;
	AdItem *currentAd() const;

	bool isWindowActive() const;
	void setIsWindowActive(bool isWindowActive);

	base::Observable<void> &isPaidObservable();
	base::Observable<void> &billingPlanObservable();

	QString settingsDirPath() const;
	QString cacheDirPath() const;
	QString pricesCacheDirPath() const;
	QString pricesIconsCacheDirPath() const;
	QString resourcesCachePath() const;
	QString settingsPath(const QString &name) const;

	QSettings settings(const QString &name) const;
	QSettings bettergramSettings() const;
	QSettings pricesSettings() const;
	QSettings pricesCacheSettings() const;

	/// Port settings files from the first Bettergram version.
	/// At the first version of the Bettergram we save settings at the QSettings() instance,
	/// on Windows it means that the settings are stored to Windows Registry.
	/// At the next releases we store Bettergram settings in separated .ini files
	/// into the `tdata/bettergram` directory.
	void portSettingsFiles();

	/// Download and parse crypto price values.
	/// If crypto price names are not fetched we fetch it now.
	/// We should call this every minute while the crypto price tab is shown
	QUrl getCryptoPriceValues(int offset, int count);
	QUrl getCryptoPriceValues(const QStringList &shortNames);

	/// Download and parse resource group list
	void getResourceGroupList();

public slots:

signals:
	void isPaidChanged();
	void billingPlanChanged();

	void needToToggleBettergramTabs();

protected:
	void timerEvent(QTimerEvent *timerEvent) override;

private:
	static BettergramService *_instance;
	static const QString _defaultLastUpdateString;

	static const int _networkTimeout;

	/// We check for new updates in 2 minutes after application startup
	static const int _checkForFirstUpdatesDelay;

	/// We check for new updates every 10 hours
	static const int _checkForUpdatesPeriod;

	/// We update crypto price names every 3 days
	static const int _updateCryptoPriceNamesPeriod;

	/// We save crypto prices every 2 hours
	static const int _saveCryptoPricesPeriod;

	/// We update rss channel list every 2 hours
	static const int _updateRssChannelListPeriod;

	/// We update video channel list every 2 hours
	static const int _updateVideoChannelListPeriod;

	/// We display deprecated API messages no more than once per 2 hours
	static const int _deprecatedApiMessagePeriod;

	bool _isSettingsPorted = false;
	bool _isPaid = false;
	BillingPlan _billingPlan = BillingPlan::Unknown;

	CryptoPriceList *_cryptoPriceList = nullptr;
	RssChannelList *_rssChannelList = nullptr;
	RssChannelList *_videoChannelList = nullptr;
	ResourceGroupList *_resourceGroupList = nullptr;
	AdItem *_currentAd = nullptr;
	int _checkForUpdatesTimerId = 0;
	int _updateCryptoPriceNamesTimerId = 0;
	int _saveCryptoPricesTimerId = 0;
	int _updateRssChannelListTimerId = 0;
	int _updateVideoChannelListTimerId = 0;
	bool _isWindowActive = true;
	std::function<void()> _isWindowActiveHandler = nullptr;

	QDateTime _lastTimeOfShowingDeprecatedApiMessage = QDateTime();
	bool _isDeprecatedApiMessageShown = false;

	static void checkForNewUpdates();

	base::Observable<void> _isPaidObservable;
	base::Observable<void> _billingPlanObservable;

	explicit BettergramService(QObject *parent = nullptr);

	void portSettingsFiles(QSettings &oldSettings, QSettings &newSettings);

	void setIsPaid(bool isPaid);
	void setBillingPlan(BillingPlan billingPlan);

	void getIsPaid();
	void getNextAd(bool reset);
	void getNextAdLater(bool reset = false);

	bool parseNextAd(const QByteArray &byteArray);

	/// Download and parse crypto price names, without actual price values.
	/// We should call this at each startup and in every 3 days.
	void getCryptoPriceNames();

	void getCryptoPriceValues(const QUrl &url, const QStringList &shortNames);

	/// Download and parse RSS channel list from Bettergram servers
	void getRssChannelList();

	/// Download and parse Video channel list from Bettergram servers
	void getVideoChannelList();

	/// Download and parse all RSS feeds
	void getRssFeedsContent();

	/// Download and parse all Video feeds
	void getVideoFeedsContent();

	void getRssFeeds(RssChannelList *rssChannelList, const QSharedPointer<RssChannel> &channel);

	/// Check response for 410 (Gone) HTTP status.
	/// If the reply has this status we show message box that the user should update the application.
	/// @return true if the reply has 410 (Gone) HTTP status or when reply is null, false otherwise
	bool isApiDeprecated(const QNetworkReply *reply);
	void showDeprecatedApiMessage();

private slots:
	void onUpdateRssFeedsContent();
	void onUpdateVideoFeedsContent();

	void onGetCryptoPriceNamesFinished();
	void onGetCryptoPriceNamesSslFailed(QList<QSslError> errors);

	void onGetCryptoPriceValuesSslFailed(QList<QSslError> errors);

	void onGetNextAdFinished();
	void onGetNextAdSslFailed(QList<QSslError> errors);

	void onGetResourceGroupListFinished();
	void onGetResourceGroupListSslFailed(QList<QSslError> errors);

	void onGetRssChannelListFinished();
	void onGetRssChannelListSslFailed(QList<QSslError> errors);

	void onGetVideoChannelListFinished();
	void onGetVideoChannelListSslFailed(QList<QSslError> errors);
};

} // namespace Bettergram
