#pragma once

#include <QObject>

namespace Bettergram {

class PinnedNewsItem;

/**
 * @brief The PinnedNewsList class contains list of pinned news and videos
 */
class PinnedNewsList : public QObject {
	Q_OBJECT

public:
	PinnedNewsList(QObject *parent);

	int freq() const;
	void setFreq(int freq);

	QList<QSharedPointer<PinnedNewsItem>> news() const;
	QList<QSharedPointer<PinnedNewsItem>> videos() const;

	bool parse(const QByteArray &byteArray);

signals:
	void freqChanged();
	void imageChanged();

	void updated();

private:
	/// Default frequency of updates in seconds.
	/// Default value is 1 hour
	static const int _defaultFreq = 60 * 60;

	QList<QSharedPointer<PinnedNewsItem>> _news;
	QList<QSharedPointer<PinnedNewsItem>> _videos;

	/// Frequency of updates in seconds
	int _freq;
	QDateTime _lastUpdate;
	QByteArray _lastSourceHash;

	bool parse(const QJsonObject &json);
	bool parseItemList(const QJsonArray &jsonArray,
					   QList<QSharedPointer<PinnedNewsItem>> &list,
					   int iconWidth,
					   int iconHeight);
};

} // namespace Bettergram

