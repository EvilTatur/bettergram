#pragma once

#include "basearticlepreviewitem.h"

namespace Bettergram {

/**
 * @brief The PinnedNewsItem class
 */
class PinnedNewsItem : public BaseArticlePreviewItem {
	Q_OBJECT

public:
	explicit PinnedNewsItem(QObject *parent = nullptr);
};

} // namespace Bettergram

