#include "remotetempdata.h"

namespace Bettergram {

RemoteTempData::RemoteTempData(QObject *parent) :
	AbstractRemoteFile(parent)
{
}

RemoteTempData::RemoteTempData(const QUrl &link, QObject *parent) :
	AbstractRemoteFile(link, parent)
{
}

bool RemoteTempData::customIsNeedToDownload() const {
	return true;
}

void RemoteTempData::dataDownloaded(const QByteArray &data)
{
	emit downloaded(data);
}

void RemoteTempData::resetData()
{
}

} // namespace Bettergrams
