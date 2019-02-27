#pragma once

#include "remoteimage.h"

#include <QObject>

namespace Bettergram {

/**
 * @brief The BaseArticlePreviewItem class is used to show news, videos and pinned news and videos.
 */
class BaseArticlePreviewItem : public QObject {
	Q_OBJECT

public:
	explicit BaseArticlePreviewItem(int iconWidth,
									int iconHeight,
									QObject *parent = nullptr);

	explicit BaseArticlePreviewItem(const QString &title,
									const QString &description,
									const QUrl &link,
									const QDateTime &publishDate,
									int iconWidth,
									int iconHeight,
									QObject *parent = nullptr);

	const QString &title() const;
	const QString &description() const;

	const QUrl &link() const;

	const QDateTime &publishDate() const;
	const QString &publishDateString() const;

	virtual QPixmap image() const;

	/// Return true if user marks this news as read
	bool isRead() const;
	void markAsRead();
	void markAsUnRead();

	bool isValid() const;

public slots:

signals:
	void isReadChanged();
	void imageChanged();

protected:
	void setTitle(const QString &title);
	void setDescription(const QString &description);
	void setLink(const QUrl &link);
	void setPublishDate(const QDateTime &publishDate);
	void setImageLink(const QUrl &url);

	bool isImageLinkValid() const;

	bool equalsToBaseItem(const QSharedPointer<BaseArticlePreviewItem> &item);
	void updateBaseItem(const QSharedPointer<BaseArticlePreviewItem> &item);

	void load(QSettings &settings);
	void save(QSettings &settings);

private:
	QString _title;
	QString _description;

	QUrl _link;

	QDateTime _publishDate;
	QString _publishDateString;

	RemoteImage _image;

	bool _isRead = false;

	void setIsRead(bool isRead);
};

} // namespace Bettergram
