#pragma once

#include "base/observer.h"

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

	static void showBettergramTabs();

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
	QString settingsPath(const QString &name) const;
	QSettings settings(const QString &name) const;

	QSettings bettergramSettings() const;
	QSettings pricesSettings() const;

	/// Port settings files from the first Bettergram version.
	/// At the first version of the Bettergram we save settings at the QSettings() instance,
	/// on Windows it means that the settings are stored to Windows Registry.
	/// At the next releases we store Bettergram settings in separated .ini files
	/// into the `tdata/bettergram` directory.
	void portSettingsFiles();

	/// Download and parse crypto price values.
	/// If crypto price names are not fetched we fetch it now.
	/// We should call this every minute while the crypto price tab is shown
	void getCryptoPriceValues(int offset, int count);
	void getCryptoPriceValues(const QStringList &shortNames);

	/// Download and parse all RSS feeds
	void getRssChannelList();

	/// Download and parse all Video feeds
	void getVideoChannelList();

	/// Download and parse resource group list
	void getResourceGroupList();

public slots:

signals:
	void isPaidChanged();
	void billingPlanChanged();
	void needToShowBettergramTabs();

protected:
	void timerEvent(QTimerEvent *timerEvent) override;

private:
	static BettergramService *_instance;
	static const QString _defaultLastUpdateString;
	static const int _networkTimeout;
	static const int _checkForFirstUpdatesDelay;
	static const int _checkForUpdatesPeriod;
	static const int _updateCryptoPriceNamesPeriod;

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
	bool _isWindowActive = true;
	std::function<void()> _isWindowActiveHandler = nullptr;

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

	void getCryptoPriceValues(const QUrl &url);

	void getRssFeeds(RssChannelList *rssChannelList, const QSharedPointer<RssChannel> &channel);

private slots:
	void onUpdateRssChannelList();
	void onUpdateVideoChannelList();

	void onGetCryptoPriceNamesFinished();
	void onGetCryptoPriceNamesSslFailed(QList<QSslError> errors);

	void onGetCryptoPriceValuesFinished();
	void onGetCryptoPriceValuesSslFailed(QList<QSslError> errors);

	void onGetNextAdFinished();
	void onGetNextAdSslFailed(QList<QSslError> errors);

	void onGetResourceGroupListFinished();
	void onGetResourceGroupListSslFailed(QList<QSslError> errors);
};

} // namespace Bettergram
