#include "abstractremotefile.h"
#include "bettergramservice.h"

#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Bettergram {

AbstractRemoteFile::AbstractRemoteFile(QObject *parent) :
	QObject(parent)
{
}

AbstractRemoteFile::AbstractRemoteFile(const QUrl &link, QObject *parent) :
	QObject(parent),
	_link(link)
{
}

AbstractRemoteFile::AbstractRemoteFile(const QUrl &link, bool isNeedDownload, QObject *parent) :
	QObject(parent),
	_link(link)
{
	if (isNeedDownload) {
		download();
	}
}

const QUrl &AbstractRemoteFile::link() const
{
	return _link;
}

void AbstractRemoteFile::setLink(const QUrl &link)
{
	if (_link != link) {
		if (!checkLink(link)) {
			return;
		}

		_link = link;

		stopDownloadLaterTimer();
		download();
		emit linkChanged();
	}
}

const QDateTime &AbstractRemoteFile::lastDownloadTime() const
{
	return _lastDownloadTime;
}

void AbstractRemoteFile::setLastDownloadTime(QDateTime lastDownloadTime)
{
	_lastDownloadTime = lastDownloadTime;
}

bool AbstractRemoteFile::isNeedToDownload() const
{
	return customIsNeedToDownload();
}

void AbstractRemoteFile::downloadIfNeeded()
{
	if (isNeedToDownload()) {
		download();
	}
}

void AbstractRemoteFile::forceDownload()
{
	stopDownloadLaterTimer();
	download();
}

void AbstractRemoteFile::download()
{
	if (!_link.isValid()) {
		resetData();
		return;
	}

	if (_isDownloading) {
		return;
	}

	if (_downloadLaterTimerId) {
		return;
	}

	_isDownloading = true;

	QNetworkAccessManager *networkManager = new QNetworkAccessManager();

	QNetworkRequest request;
	request.setUrl(_link);

	QNetworkReply *reply = networkManager->get(request);

	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		_isDownloading = false;

		if(reply->error() == QNetworkReply::NoError) {
			_failedCount = 0;
			dataDownloaded(reply->readAll());
			_lastDownloadTime = QDateTime::currentDateTime();
			emit downloaded();
		} else {
			LOG(("Can not download file at %1. %2 (%3)")
				.arg(_link.toString())
				.arg(reply->errorString())
				.arg(reply->error()));

			// If the file does not exist on the server then
			// there is no any reason to try download it soon
			if (reply->error() == QNetworkReply::ContentNotFoundError) {
				_failedCount = 10000;
			} else {
				_failedCount++;
			}

			downloadLater();
		}
	});

	connect(reply, &QNetworkReply::finished, [networkManager, reply]() {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	connect(this, &AbstractRemoteFile::destroyed, networkManager, [networkManager, reply] {
		reply->deleteLater();
		networkManager->deleteLater();
	});

	QTimer::singleShot(BettergramService::networkTimeout(), Qt::VeryCoarseTimer, networkManager,
					   [networkManager, reply, this] {
		_isDownloading = false;
		_failedCount++;

		reply->deleteLater();
		networkManager->deleteLater();

		LOG(("Can not download file at %1 due timeout")
			.arg(_link.toString()));

		downloadLater();
	});

	connect(reply, &QNetworkReply::sslErrors, this, [](QList<QSslError> errors) {
		for(const QSslError &error : errors) {
			LOG(("%1").arg(error.errorString()));
		}
	});
}

void AbstractRemoteFile::timerEvent(QTimerEvent *timerEvent)
{
	stopDownloadLaterTimer();
	download();
}

void AbstractRemoteFile::stopDownloadLaterTimer()
{
	if (_downloadLaterTimerId) {
		killTimer(_downloadLaterTimerId);
		_downloadLaterTimerId = 0;
	}
}

void AbstractRemoteFile::downloadLater()
{
	if (_downloadLaterTimerId) {
		return;
	}

	qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch() / (5 * 1000 * 1000)));

	// Increase the timeout after each call of this method
	int timeout = qMin(5 * 60 * 60 * 1000, _failedCount * 5000) + (qrand() % 3000);

	_downloadLaterTimerId = startTimer(timeout, Qt::VeryCoarseTimer);
}

bool AbstractRemoteFile::checkLink(const QUrl &link)
{
	Q_UNUSED(link);

	return true;
}

} // namespace Bettergrams
