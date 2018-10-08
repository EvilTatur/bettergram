#pragma once

#include <QObject>

namespace Bettergram {

/**
 * @brief The AbstractRemoteFile class is used to download and use remote files.
 */
class AbstractRemoteFile : public QObject {
	Q_OBJECT

public:
	explicit AbstractRemoteFile(QObject *parent = nullptr);
	explicit AbstractRemoteFile(const QUrl &link, bool isNeedDownload, QObject *parent = nullptr);

	const QUrl &link() const;
	void setLink(const QUrl &link);

	bool isNeedToDownload() const;

	void downloadIfNeeded();
	void forceDownload();

public slots:

signals:
	void linkChanged();

protected:
	virtual bool customIsNeedToDownload() const = 0;
	virtual void dataDownloaded(const QByteArray &data) = 0;
	virtual void resetData() = 0;

	virtual bool checkLink(const QUrl &link);

	void download();

private:
	QUrl _link;
	bool _isDownloading = false;

	void downloadLater();
};

} // namespace Bettergram
