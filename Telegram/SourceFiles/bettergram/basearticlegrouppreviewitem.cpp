#include "basearticlegrouppreviewitem.h"

namespace Bettergram {

BaseArticleGroupPreviewItem::BaseArticleGroupPreviewItem(int imageWidth, int imageHeight)
	: QObject(nullptr),
	  _icon(imageWidth, imageHeight, nullptr)
{
	connect(&_icon, &RemoteImage::imageChanged, this, &BaseArticleGroupPreviewItem::iconChanged);
}

const QString &BaseArticleGroupPreviewItem::title() const
{
	return _title;
}

void BaseArticleGroupPreviewItem::setTitle(const QString &title)
{
	_title = title;
}

const QString &BaseArticleGroupPreviewItem::description() const
{
	return _description;
}

void BaseArticleGroupPreviewItem::setDescription(const QString &description)
{
	_description = description;
}

const QUrl &BaseArticleGroupPreviewItem::link() const
{
	return _link;
}

void BaseArticleGroupPreviewItem::setLink(const QUrl &link)
{
	_link = link;
}

const QPixmap &BaseArticleGroupPreviewItem::icon() const
{
	return _icon.image();
}

void BaseArticleGroupPreviewItem::setIcon(const QPixmap &icon)
{
	_icon.setImage(icon);
}

const QUrl &BaseArticleGroupPreviewItem::iconLink() const
{
	return _icon.link();
}

void BaseArticleGroupPreviewItem::setIconLink(const QUrl &iconLink)
{
	_icon.setLink(iconLink);
}

int BaseArticleGroupPreviewItem::iconWidth() const
{
	return _icon.scaledWidth();
}

int BaseArticleGroupPreviewItem::iconHeight() const
{
	return _icon.scaledHeight();
}

void BaseArticleGroupPreviewItem::markAsRead()
{
	// Do nothing here
}

} // namespace Bettergram
