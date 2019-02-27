#pragma once

#include <QObject>

namespace Bettergram {

/**
 * @brief The AbstractRemoteFile class is used to download and use remote files.
 */
class AbstractRemoteFile : public QObject {
	Q_OBJECT

public:
	explicit AbstractRemoteFile(QObject *parent);
	explicit AbstractRemoteFile(const QUrl &link, QObject *parent);
	explicit AbstractRemoteFile(const QUrl &link, bool isNeedDownload, QObject *parent);

	const QUrl &link() const;
	void setLink(const QUrl &link);

	const QDateTime &lastDownloadTime() const;

	/// Call this method only when you load data from persistent storage
	void setLastDownloadTime(QDateTime lastDownloadTime);

	bool isNeedToDownload() const;

	void downloadIfNeeded();
	void forceDownload();

public slots:

signals:
	void linkChanged();
	void downloaded();

protected:
	virtual bool customIsNeedToDownload() const = 0;
	virtual void dataDownloaded(const QByteArray &data) = 0;
	virtual void resetData() = 0;

	virtual bool checkLink(const QUrl &link);

	void download();
	void stopDownloadLaterTimer();

	void timerEvent(QTimerEvent *timerEvent) override;

private:
	QUrl _link;
	QDateTime _lastDownloadTime;
	int _failedCount = 0;
	bool _isDownloading = false;
	int _downloadLaterTimerId = 0;

	void downloadLater();
};

} // namespace Bettergram
