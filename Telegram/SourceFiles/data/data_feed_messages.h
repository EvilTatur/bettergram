/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "data/data_feed.h"
#include "data/data_messages.h"

namespace Storage {
struct FeedMessagesKey;
} // namespace Storage

namespace Data {

rpl::producer<MessagesSlice> FeedMessagesViewer(
	Storage::FeedMessagesKey key,
	int limitBefore,
	int limitAfter);

} // namespace Data
