#pragma once

#include <QObject>

#include "remoteimage.h"

namespace Bettergram {

/**
 * @brief The BaseArticleGroupPreviewItem class is used to show news and video channels and pinned groups of news.
 */
class BaseArticleGroupPreviewItem : public QObject
{
    Q_OBJECT

public:
	explicit BaseArticleGroupPreviewItem(int imageWidth, int imageHeight);

	const QString &title() const;
	void setTitle(const QString &title);

	const QString &description() const;
	void setDescription(const QString &description);

	const QUrl &link() const;
	void setLink(const QUrl &link);

	const QPixmap &icon() const;
	void setIcon(const QPixmap &icon);

	const QUrl &iconLink() const;
	void setIconLink(const QUrl &iconLink);

	int iconWidth() const;
	int iconHeight() const;

	virtual void markAsRead();

public slots:

signals:
	void iconChanged();

private:
	QString _title;
	QString _description;

	QUrl _link;

	RemoteImage _icon;
};

} // namespace Bettergram
