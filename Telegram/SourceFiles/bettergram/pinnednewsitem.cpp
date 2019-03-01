#include "pinnednewsitem.h"

namespace Bettergram {

PinnedNewsItem::PinnedNewsItem(const QString &title,
							   const QString &description,
							   const QUrl &link,
							   const QUrl &imageLink,
							   const QDateTime &publishDate,
							   const QDateTime &endDate,
							   int position,
							   int iconWidth,
							   int iconHeight)
	: BaseArticlePreviewItem(title,
							 description,
							 link,
							 imageLink,
							 true,
							 publishDate,
							 iconWidth,
							 iconHeight),
	  _position(position),
	  _endDate(endDate)
{
}

int PinnedNewsItem::position() const
{
	return _position;
}

} // namespace Bettergram
