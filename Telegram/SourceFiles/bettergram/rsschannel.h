#pragma once

#include "basearticlegrouppreviewitem.h"

class QXmlStreamReader;

namespace Bettergram {

class RssItem;

/**
 * @brief The RssChannel class contains information from a RSS channel.
 */
class RssChannel : public BaseArticleGroupPreviewItem {
	Q_OBJECT

public:
	typedef QList<QSharedPointer<RssItem>>::const_iterator const_iterator;
	typedef QList<QSharedPointer<RssItem>>::iterator iterator;

	static void sort(QList<QSharedPointer<RssItem>> &items);

	explicit RssChannel(int imageWidth, int imageHeight);
	explicit RssChannel(const QUrl &feedLink, int imageWidth, int imageHeight);

	const QString &language() const;
	void setLanguage(const QString &language);

	const QString &copyright() const;
	void setCopyright(const QString &copyright);

	const QString &editorEmail() const;
	void setEditorEmail(const QString &editorEmail);

	const QString &webMasterEmail() const;
	void setWebMasterEmail(const QString &webMasterEmail);

	const QStringList &categoryList() const;
	void setCategoryList(const QStringList &categoryList);

	const QDateTime &publishDate() const;
	void setPublishDate(const QDateTime &publishDate);

	const QDateTime &lastBuildDate() const;
	void setLastBuildDate(const QDateTime &lastBuildDate);

	const QString &skipHours() const;
	void setSkipHours(const QString &skipHours);

	const QString &skipDays() const;
	void setSkipDays(const QString &skipDays);

	const QUrl &feedLink() const;
	void setFeedLink(const QUrl &link);

	bool isFetching() const;
	bool isFailed() const;

	const_iterator begin() const;
	const_iterator end() const;

	const QSharedPointer<RssItem> &at(int index) const;

	const QList<QSharedPointer<RssItem>> &getAllItems() const;
	QList<QSharedPointer<RssItem>> getAllUnreadItems() const;

	int count() const;
	int countUnread() const;

	bool isMayFetchNewData() const;

	void markAsRead() override;

	void startFetching();
	void fetchingSucceed(const QByteArray &source);
	void fetchingFailed();

	/// Parse fetched source xml data and return true only when the data is changed
	bool parse();

	void load(QSettings &settings);
	void save(QSettings &settings);

public slots:

signals:
	void isReadChanged();
	void updated();

protected:

private:
	QString _language;
	QString _copyright;
	QString _editorEmail;
	QString _webMasterEmail;
	QStringList _categoryList;

	//TODO: bettergram: use publish date and last build date properties
	QDateTime _publishDate;
	QDateTime _lastBuildDate;

	//TODO: bettergram: use skip hours and skip days properties
	QString _skipHours;
	QString _skipDays;

	QUrl _feedLink;

	QByteArray _source;
	QByteArray _lastSourceHash;
	bool _isFetching = false;
	bool _isFailed = false;

	QList<QSharedPointer<RssItem>> _list;

	static bool compare(const QSharedPointer<RssItem> &a, const QSharedPointer<RssItem> &b);

	void setIsFetching(bool isFetching);
	void setIsFailed(bool isFailed);

	QByteArray countSourceHash(const QByteArray &source) const;

	void removeOldItems();

	void parseRss(QXmlStreamReader &xml);
	void parseAtomFeed(QXmlStreamReader &xml);
	void parseChannel(QXmlStreamReader &xml);
	void parseChannelImage(QXmlStreamReader &xml);
	void parseItem(QXmlStreamReader &xml);
	void parseAtomEntry(QXmlStreamReader &xml);

	QSharedPointer<RssItem> find(const QSharedPointer<RssItem> &item);
	void merge(const QSharedPointer<RssItem> &item);
	void add(const QSharedPointer<RssItem> &item);
};

} // namespace Bettergram
