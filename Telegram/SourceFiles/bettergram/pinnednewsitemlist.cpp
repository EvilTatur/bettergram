#include "pinnednewsitemlist.h"
#include "pinnednewsitem.h"
#include "bettergramservice.h"

#include <styles/style_chat_helpers.h>
#include <logs.h>

#include <QJsonDocument>

namespace Bettergram {

PinnedNewsItemList::PinnedNewsItemList(QObject *parent)
	: QObject(parent),
	  _freq(_defaultFreq)
{
}

int PinnedNewsItemList::freq() const
{
	return _freq;
}

void PinnedNewsItemList::setFreq(int freq)
{
	if (freq == 0) {
		freq = _defaultFreq;
	}

	if (_freq != freq) {
		_freq = freq;
		emit freqChanged();
	}
}

QList<QSharedPointer<PinnedNewsItem>> PinnedNewsItemList::news() const
{
	return _news;
}

QList<QSharedPointer<PinnedNewsItem>> PinnedNewsItemList::videos() const
{
	return _videos;
}

bool PinnedNewsItemList::parse(const QByteArray &byteArray)
{
	// Update only if it has been changed
	QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);

	if (hash == _lastSourceHash) {
		_lastUpdate = QDateTime::currentDateTime();
		return false;
	}

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);

	if (!doc.isObject()) {
		LOG(("Can not get pinned news. Data is wrong. %1 (%2). Data: %3")
			.arg(parseError.errorString())
			.arg(parseError.error)
			.arg(QString::fromUtf8(byteArray)));

		return false;
	}

	QJsonObject json = doc.object();

	if (json.isEmpty()) {
		LOG(("Can not get pinned news. Data is emtpy or wrong"));
		return false;
	}

	if (!parse(json)) {
		return false;
	}

	_lastSourceHash = hash;

	return true;
}

bool PinnedNewsItemList::parse(const QJsonObject &json)
{
	if (json.isEmpty()) {
		return false;
	}

	if (json.contains("success")) {
		if (!json.value("success").toBool()) {
			return false;
		}
	}

	if (json.contains("freq")) {
		setFreq(json.value("freq").toInt());
	}

	if (json.contains("news")) {
		parseItemList(json.value("news").toArray(),
					  _news,
					  st::newsPanImageWidth,
					  st::newsPanImageHeight);
	}

	if (json.contains("videos")) {
		parseItemList(json.value("videos").toArray(),
					  _videos,
					  st::videosPanImageWidth,
					  st::videosPanImageHeight);
	}

	_lastUpdate = QDateTime::currentDateTime();
	emit updated();

	return true;
}

bool PinnedNewsItemList::parseItemList(const QJsonArray &jsonArray,
									   QList<QSharedPointer<PinnedNewsItem>> &list,
									   int iconWidth,
									   int iconHeight)
{
	list.clear();

	for (const QJsonValue jsonValue : jsonArray) {
		if (!jsonValue.isObject()) {
			LOG(("Unable to get json object for pinned news"));
			continue;
		}

		QJsonObject json = jsonValue.toObject();

		const QString title = json.value("title").toString();
		const QString description = json.value("description").toString();

		const QUrl url = QUrl(json.value("url").toString());
		const QUrl imageUrl = QUrl(json.value("imageUrl").toString());

		const QDateTime date = QDateTime::fromString(json.value("date").toString(), Qt::ISODate);
		const QDateTime endDate = QDateTime::fromString(json.value("endDate").toString(), Qt::ISODate);

		const int pos = json.value("pos").toInt();

		if (title.isEmpty() || !url.isValid() || !imageUrl.isValid() || !date.isValid()) {
			LOG(("Unable to parse pinned news item, because item is invalid"));
			continue;
		}

		QSharedPointer<PinnedNewsItem> item(new PinnedNewsItem(title,
															   description,
															   url,
															   imageUrl,
															   date,
															   endDate,
															   iconWidth,
															   iconHeight));

		connect(item.data(), &PinnedNewsItem::imageChanged, this, &PinnedNewsItemList::imageChanged);

		list.push_back(item);
	}

	return true;
}

} // namespace Bettergram
