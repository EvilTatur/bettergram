#include "pinnednewsitem.h"

namespace Bettergram {

PinnedNewsItem::PinnedNewsItem(const QString &title,
							   const QString &description,
							   const QUrl &link,
							   const QUrl &imageLink,
							   const QDateTime &publishDate,
							   const QDateTime &endDate,
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
	  _endDate(endDate)
{
}

} // namespace Bettergram
