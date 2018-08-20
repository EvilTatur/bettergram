#include "rsschannellist.h"
#include "rsschannel.h"
#include <lang/lang_keys.h>
#include <logs.h>

namespace Bettergram {

const int RssChannelList::_defaultFreq = 60;
const QString RssChannelList::_defaultLastUpdateString = "...";

QString RssChannelList::countLastUpdateString(const QDateTime &dateTime)
{
	if (dateTime.isNull()) {
		return _defaultLastUpdateString;
	}

	qint64 daysBefore = QDateTime::currentDateTime().daysTo(dateTime);
	const QString timeString = dateTime.toString("hh:mm:ss");

	if (daysBefore == 0) {
		return lng_player_message_today(lt_time, timeString);
	} else if (daysBefore == -1) {
		return lng_player_message_yesterday(lt_time, timeString);
	} else {
		return lng_player_message_date(lt_date,
									   langDayOfMonthFull(dateTime.date()),
									   lt_time,
									   timeString);
	}
}

RssChannelList::RssChannelList(QObject *parent) :
	QObject(parent),
	_freq(_defaultFreq),
	_lastUpdateString(_defaultLastUpdateString)
{
}

int RssChannelList::freq() const
{
	return _freq;
}

void RssChannelList::setFreq(int freq)
{
	if (freq == 0) {
		freq = _defaultFreq;
	}

	if (_freq != freq) {
		_freq = freq;
		emit freqChanged();
	}
}

QDateTime RssChannelList::lastUpdate() const
{
	return _lastUpdate;
}

QString RssChannelList::lastUpdateString() const
{
	return _lastUpdateString;
}

RssChannelList::const_iterator RssChannelList::begin() const
{
	return _list.begin();
}

RssChannelList::const_iterator RssChannelList::end() const
{
	return _list.end();
}

const QSharedPointer<RssChannel> &RssChannelList::at(int index) const
{
	if (index < 0 || index >= _list.size()) {
		LOG(("Index is out of bounds"));
		throw std::out_of_range("rss channel index is out of range");
	}

	return _list.at(index);
}

int RssChannelList::count() const
{
	return _list.count();
}

int RssChannelList::countAllItems() const
{
	int result = 0;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result += channel->count();
	}

	return result;
}

int RssChannelList::countAllUnreadItems() const
{
	int result = 0;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result += channel->countUnread();
	}

	return result;
}

void RssChannelList::add(const QUrl &channelLink)
{
	if (channelLink.isEmpty()) {
		LOG(("Unable to add empty RSS channel"));
		return;
	}

	QSharedPointer<RssChannel> channel(new RssChannel(channelLink, nullptr));

	connect(channel.data(), &RssChannel::iconChanged, this, &RssChannelList::iconChanged);

	_list.push_back(channel);
}

QList<QSharedPointer<RssItem>> RssChannelList::getAllItems() const
{
	QList<QSharedPointer<RssItem>> result;
	result.reserve(countAllItems());

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result.append(channel->getAllItems());
	}

	return result;
}

QList<QSharedPointer<RssItem>> RssChannelList::getAllUnreadItems() const
{
	QList<QSharedPointer<RssItem>> result;
	result.reserve(countAllUnreadItems());

	for (const QSharedPointer<RssChannel> &channel : _list) {
		result.append(channel->getAllUnreadItems());
	}

	return result;
}

void RssChannelList::parse()
{
	for (const QSharedPointer<RssChannel> &channel : _list) {
		if (channel->isFetching()) {
			return;
		}
	}

	bool isChanged = false;
	bool isAtLeastOneUpdated = false;

	for (const QSharedPointer<RssChannel> &channel : _list) {
		if (!channel->isFailed()) {
			isAtLeastOneUpdated = true;

			if (channel->parse()) {
				isChanged = true;
			}
		}
	}

	if (isChanged) {
		emit updated();
	}

	if (isAtLeastOneUpdated) {
		_lastUpdate = QDateTime::currentDateTime();
		_lastUpdateString = countLastUpdateString(_lastUpdate);

		emit lastUpdateChanged();
	}
}

} // namespace Bettergrams
