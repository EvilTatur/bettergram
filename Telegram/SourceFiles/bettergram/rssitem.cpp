#include "rssitem.h"
#include "rsschannel.h"
#include "imagefromsite.h"

#include <QXmlStreamReader>

namespace Bettergram {

const qint64 RssItem::_maxLastHoursInMs = 24 * 60 * 60 * 1000;

RssItem::RssItem(RssChannel *channel) :
	BaseArticlePreviewItem(channel->iconWidth(), channel->iconHeight()),
	_channel(channel)
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	connect(_channel, &RssChannel::destroyed, this, &RssItem::onChannelDestroyed);
}

RssItem::RssItem(const QString &guid,
				 const QString &title,
				 const QString &description,
				 const QString &author,
				 const QStringList &categoryList,
				 const QUrl &link,
				 const QUrl &commentsLink,
				 const QDateTime &publishDate,
				 RssChannel *channel) :
	BaseArticlePreviewItem(title,
						   description,
						   link,
						   publishDate,
						   channel->iconWidth(),
						   channel->iconHeight()),
	_channel(channel),
	_guid(guid),
	_author(author),
	_categoryList(categoryList),
	_commentsLink(commentsLink)
{
	if (!_channel) {
		throw std::invalid_argument("RSS Channel is null");
	}

	connect(_channel, &RssChannel::destroyed, this, &RssItem::onChannelDestroyed);
}

const QString &RssItem::guid() const
{
	return _guid;
}

const QString &RssItem::author() const
{
	return _author;
}

const QStringList &RssItem::categoryList() const
{
	return _categoryList;
}

const QUrl &RssItem::commentsLink() const
{
	return _commentsLink;
}

QPixmap RssItem::image() const
{
	if (!BaseArticlePreviewItem::image().isNull()) {
		return BaseArticlePreviewItem::image();
	}

	if (_imageFromSite && !_imageFromSite->isNull()) {
		return _imageFromSite->image();
	}

	if (!_channel) {
		qWarning() << "RSS Channel is null";
		return QPixmap();
	}

	return _channel->icon();
}

bool RssItem::isOld(const QDateTime &now) const
{
	return now.msecsTo(publishDate()) < -_maxLastHoursInMs;
}

void RssItem::tryToGetImageLink(const QString &text)
{
	if (isImageLinkValid()) {
		return;
	}

	int imgTagIndex = text.indexOf("<img");

	if (imgTagIndex == -1) {
		return;
	}

	int srcAttributeStartIndex = text.indexOf("src=\"", imgTagIndex + 5);

	if (srcAttributeStartIndex == -1) {
		return;
	}

	int srcAttributeEndIndex = text.indexOf("\"", srcAttributeStartIndex + 6);

	if (srcAttributeEndIndex == -1) {
		return;
	}

	srcAttributeStartIndex += 5;

	QString urlString = text.mid(srcAttributeStartIndex,
								 srcAttributeEndIndex - srcAttributeStartIndex);

	if (urlString.isEmpty()) {
		return;
	}

	QUrl url(urlString);

	if (url.isValid()) {
		setImageLink(url);
	}
}

void RssItem::markAllNewsAtSiteAsRead()
{
	if (!_channel) {
		qWarning() << "RSS Channel is null";
		return;
	}

	_channel->markAsRead();
}

bool RssItem::isExistAtLastFeeds() const
{
	return _isExistAtLastFeeds;
}

void RssItem::setIsExistAtLastFeeds(bool isExistAtLastFeeds)
{
	_isExistAtLastFeeds = isExistAtLastFeeds;
}

bool RssItem::equalsTo(const QSharedPointer<RssItem> &item)
{
	return equalsToBaseItem(item);
}

void RssItem::update(const QSharedPointer<RssItem> &item)
{
	updateBaseItem(item);

	_guid = item->_guid;
	_author = item->_author;
	_categoryList = item->_categoryList;
	_commentsLink = item->_commentsLink;

	if (_imageFromSite && item->_imageFromSite) {
		_imageFromSite->setLink(item->_imageFromSite->link());
	} else if (!_imageFromSite && item->_imageFromSite) {
		createImageFromSite();

		_imageFromSite->setLink(item->_imageFromSite->link());
	}

	_isExistAtLastFeeds = true;

	// We do not change _isRead field in this method
}

void RssItem::parse(QXmlStreamReader &xml)
{
	_categoryList.clear();

	while (xml.readNextStartElement()) {
		if (!xml.prefix().isEmpty()) {
			if (xml.name() == QLatin1String("encoded")
					&& xml.namespaceUri() == "http://purl.org/rss/1.0/modules/content/") {
				tryToGetImageLink(xml.readElementText());
				continue;
			}

			xml.skipCurrentElement();
			continue;
		}

		QStringRef xmlName = xml.name();

		if (xmlName == QLatin1String("guid")) {
			_guid = xml.readElementText();
		} else if (xmlName == QLatin1String("title")) {
			const QString elementText = xml.readElementText();

			tryToGetImageLink(elementText);

			setTitle(removeHtmlTags(elementText));
		} else if (xmlName == QLatin1String("description")) {
			const QString elementText = xml.readElementText();

			tryToGetImageLink(elementText);

			setDescription(removeHtmlTags(elementText));
		} else if (xmlName == QLatin1String("author")) {
			_author = xml.readElementText();
		} else if (xmlName == QLatin1String("category")) {
			_categoryList.push_back(xml.readElementText());
		} else if (xmlName == QLatin1String("link")) {
			setLink(xml.readElementText());
		} else if (xmlName == QLatin1String("comments")) {
			_commentsLink = xml.readElementText();
		} else if (xmlName == QLatin1String("pubDate")) {
			setPublishDate(QDateTime::fromString(xml.readElementText(), Qt::RFC2822Date));
		} else if (xmlName == QLatin1String("enclosure")) {
			QUrl url = QUrl(xml.attributes().value("url").toString());

			if (url.isValid()) {
				if (xml.attributes().value("type").contains("image")) {
					setImageLink(url);
				}
			}
			xml.skipCurrentElement();
		} else {
			xml.skipCurrentElement();
		}
	}

	if (!isImageLinkValid()) {
		createImageFromSite();
		_imageFromSite->setLink(link());
	}
}

void RssItem::parseAtom(QXmlStreamReader &xml)
{
	_categoryList.clear();

	while (xml.readNextStartElement()) {
		QStringRef xmlName = xml.name();
		QStringRef xmlNamespace = xml.namespaceUri();

		if (xmlNamespace.isEmpty() || xmlNamespace == "http://www.w3.org/2005/Atom") {
			if (xmlName == QLatin1String("id")) {
				_guid = xml.readElementText();
			} else if (xmlName == QLatin1String("title")) {
				setTitle(removeHtmlTags(xml.readElementText()));
			} else if (xmlName == QLatin1String("category")) {
				_categoryList.push_back(xml.attributes().value("term").toString());
				xml.skipCurrentElement();
			} else if (xmlName == QLatin1String("link")) {
				setLink(QUrl(xml.attributes().value("href").toString()));
				xml.skipCurrentElement();
			} else if (xmlName == QLatin1String("category")) {
				_categoryList.push_back(xml.attributes().value("term").toString());
			} else if (xmlName == QLatin1String("published")) {
				if (publishDate().isValid()) {
					xml.skipCurrentElement();
				} else {
					setPublishDate(QDateTime::fromString(xml.readElementText(), Qt::ISODate));
				}
			} else if (xmlName == QLatin1String("updated")) {
				setPublishDate(QDateTime::fromString(xml.readElementText(), Qt::ISODate));
			} else {
				xml.skipCurrentElement();
			}
		} else if (xmlNamespace  == "http://search.yahoo.com/mrss/") {
			if (xmlName == QLatin1String("group")) {
				parseAtomMediaGroup(xml);
			}
		} else {
			xml.skipCurrentElement();
		}
	}

	if (!isImageLinkValid()) {
		createImageFromSite();
		_imageFromSite->setLink(link());
	}
}

void RssItem::parseAtomMediaGroup(QXmlStreamReader &xml)
{
	while (xml.readNextStartElement()) {
		QStringRef xmlName = xml.name();
		QStringRef xmlNamespace = xml.namespaceUri();

		if (xmlNamespace != "http://search.yahoo.com/mrss/") {
			xml.skipCurrentElement();
			continue;
		}

		if (xmlName == QLatin1String("description")) {
			setDescription(xml.readElementText());
		} else if (xmlName == QLatin1String("thumbnail")) {
			setImageLink(QUrl(xml.attributes().value("url").toString()));
			xml.skipCurrentElement();
		} else {
			xml.skipCurrentElement();
		}
	}
}

void RssItem::load(QSettings &settings)
{
	BaseArticlePreviewItem::load(settings);

	_guid =  settings.value("guid").toString();
	_author =  settings.value("author").toString();
	_categoryList = settings.value("categoryList").toStringList();

	_commentsLink = settings.value("commentsLink").toUrl();

	if (!isImageLinkValid() && link().isValid()) {
		createImageFromSite();

		_imageFromSite->setLink(link());
	}
}

void RssItem::save(QSettings &settings)
{
	BaseArticlePreviewItem::save(settings);

	settings.setValue("guid", guid());
	settings.setValue("author", author());
	settings.setValue("categoryList", categoryList());

	settings.setValue("commentsLink", commentsLink().toString());
}

QString RssItem::removeHtmlTags(const QString &text)
{
	QTextDocument textDocument;
	textDocument.setHtml(text);
	return textDocument.toPlainText();
}

void RssItem::createImageFromSite()
{
	if (_imageFromSite) {
		return;
	}

	if (!_channel) {
		qWarning() << "RSS Channel is null";
		return;
	}

	_imageFromSite = new ImageFromSite(_channel->iconWidth(), _channel->iconHeight(), this);

	connect(_imageFromSite, &ImageFromSite::imageChanged, this, &RssItem::imageChanged);
}

void RssItem::onChannelDestroyed()
{
	_channel = nullptr;
}

} // namespace Bettergrams
