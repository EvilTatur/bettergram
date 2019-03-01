#pragma once

#include "basearticlepreviewitem.h"

#include <QObject>

class QXmlStreamReader;

namespace Bettergram {

class RssChannel;
class ImageFromSite;

/**
 * @brief The RssItem class contains information from a RSS item.
 */
class RssItem : public BaseArticlePreviewItem {
	Q_OBJECT

public:
	explicit RssItem(RssChannel *channel);

	explicit RssItem(const QString &guid,
					 const QString &title,
					 const QString &description,
					 const QString &author,
					 const QStringList &categoryList,
					 const QUrl &link,
					 const QUrl &commentsLink,
					 const QDateTime &publishDate,
					 RssChannel *channel);

	const QString &guid() const;
	const QString &author() const;
	const QStringList &categoryList() const;
	const QUrl &commentsLink() const;
	QPixmap image() const override;

	bool isOld(const QDateTime &now = QDateTime::currentDateTime()) const;

	void markAllNewsAtSiteAsRead() override;

	bool isExistAtLastFeeds() const;
	void setIsExistAtLastFeeds(bool isExistAtLastFeeds);

	bool equalsTo(const QSharedPointer<RssItem> &item);
	void update(const QSharedPointer<RssItem> &item);

	void parse(QXmlStreamReader &xml);
	void parseAtom(QXmlStreamReader &xml);

	void load(QSettings &settings);
	void save(QSettings &settings);

public slots:

signals:

protected:

private:
	/// Keep news only for the last hours
	static const qint64 _maxLastHoursInMs;

	RssChannel *_channel = nullptr;

	QString _guid;
	QString _author;
	QStringList _categoryList;

	QUrl _commentsLink;

	/// We try to get _imageLink from <enclosure url="link-to-image" type="image/..."/>,
	/// or from <description>Text <img src="link-to-image"></description>,
	/// or from <title>Text <img src="link-to-image"></title> tags,
	/// or try to get the largest image from the site content
	ImageFromSite *_imageFromSite = nullptr;

	/// True if this item exists at the last feeds from sites.
	bool _isExistAtLastFeeds = true;

	static QString removeHtmlTags(const QString &text);

	void tryToGetImageLink(const QString &text);

	void parseAtomMediaGroup(QXmlStreamReader &xml);

	void createImageFromSite();

private slots:
	void onChannelDestroyed();
};

} // namespace Bettergram
