/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

class HistoryItem;

namespace Data {
struct Group;
} // namespace Data

TextWithEntities HistoryItemText(not_null<HistoryItem*> item);
TextWithEntities HistoryGroupText(not_null<const Data::Group*> group);
