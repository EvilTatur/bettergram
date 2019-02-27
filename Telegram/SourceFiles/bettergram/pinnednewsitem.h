#pragma once

#include "basearticlepreviewitem.h"

namespace Bettergram {

/**
 * @brief The PinnedNewsItem class
 */
class PinnedNewsItem : public BaseArticlePreviewItem {
	Q_OBJECT

public:
	explicit PinnedNewsItem(const QString &title,
							const QString &description,
							const QUrl &link,
							const QUrl &imageLink,
							const QDateTime &publishDate,
							const QDateTime &endDate,
							int iconWidth,
							int iconHeight);

private:
	QDateTime _endDate;
};

} // namespace Bettergram

