#include "basearticlepreviewitem.h"
#include "bettergramservice.h"

namespace Bettergram {

BaseArticlePreviewItem::BaseArticlePreviewItem(int iconWidth,
											   int iconHeight,
											   QObject *parent)
	: QObject(parent),
	  _image(iconWidth, iconHeight)
{
	connect(&_image, &RemoteImage::imageChanged, this, &BaseArticlePreviewItem::imageChanged);
}

BaseArticlePreviewItem::BaseArticlePreviewItem(const QString &title,
											   const QString &description,
											   const QUrl &link,
											   const QDateTime &publishDate,
											   int iconWidth,
											   int iconHeight,
											   QObject *parent)
	: QObject(parent),
	  _title(title),
	  _description(description),
	  _link(link),
	  _publishDate(publishDate),
	  _image(iconWidth, iconHeight)
{
	connect(&_image, &RemoteImage::imageChanged, this, &BaseArticlePreviewItem::imageChanged);
}

const QString &BaseArticlePreviewItem::title() const
{
	return _title;
}

void BaseArticlePreviewItem::setTitle(const QString &title)
{
	_title = title;
}

const QString &BaseArticlePreviewItem::description() const
{
	return _description;
}

void BaseArticlePreviewItem::setDescription(const QString &description)
{
	_description = description;
}

const QUrl &BaseArticlePreviewItem::link() const
{
	return _link;
}

void BaseArticlePreviewItem::setLink(const QUrl &link)
{
	_link = link;
}

const QDateTime &BaseArticlePreviewItem::publishDate() const
{
	return _publishDate;
}

void BaseArticlePreviewItem::setPublishDate(const QDateTime &publishDate)
{
	if (_publishDate != publishDate) {
		_publishDate = publishDate;

		_publishDateString =
				BettergramService::generateLastUpdateString(_publishDate.toLocalTime(), false);
	}
}

const QString &BaseArticlePreviewItem::publishDateString() const
{
	return _publishDateString;
}

QPixmap BaseArticlePreviewItem::image() const
{
	return _image.image();
}

void BaseArticlePreviewItem::setImageLink(const QUrl &url)
{
	_image.setLink(url);
}

bool BaseArticlePreviewItem::isImageLinkValid() const
{
	return _image.link().isValid();
}

bool BaseArticlePreviewItem::isRead() const
{
	return _isRead;
}

bool BaseArticlePreviewItem::isValid() const
{
	return !_link.isEmpty() && !_title.isEmpty() && !_publishDate.isNull();
}

void BaseArticlePreviewItem::setIsRead(bool isRead)
{
	if (_isRead != isRead) {
		_isRead = isRead;
		emit isReadChanged();
	}
}

void BaseArticlePreviewItem::markAsRead()
{
	setIsRead(true);
}

void BaseArticlePreviewItem::markAsUnRead()
{
	setIsRead(false);
}

bool BaseArticlePreviewItem::equalsToBaseItem(const QSharedPointer<BaseArticlePreviewItem> &item)
{
	return _link == item->link();
}

void BaseArticlePreviewItem::updateBaseItem(const QSharedPointer<BaseArticlePreviewItem> &item)
{
	_title = item->_title;
	_description = item->_description;
	_link = item->_link;
	_publishDate = item->_publishDate;
	_publishDateString = item->_publishDateString;
	_image.setLink(item->_image.link());
}

void BaseArticlePreviewItem::load(QSettings &settings)
{
	_title = settings.value("title").toString();
	_description = settings.value("description").toString();

	_link = settings.value("link").toUrl();

	setPublishDate(settings.value("publishDate").toDateTime());
	_image.setLink(settings.value("imageLink").toString());

	setIsRead(settings.value("isRead").toBool());
}

void BaseArticlePreviewItem::save(QSettings &settings)
{
	settings.setValue("title", title());
	settings.setValue("description", description());

	// We have to save QUrl as QString due bug on macOS
	settings.setValue("link", link().toString());

	settings.setValue("publishDate", publishDate());
	settings.setValue("imageLink", _image.link().toString());

	settings.setValue("isRead", isRead());
}

} // namespace Bettergram
