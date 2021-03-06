/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "codegen/common/logging.h"
#include "codegen/emoji/data.h"
#include <QtCore/QVector>

namespace codegen {
namespace emoji {

struct Replace {
	Id id;
	QString replacement;
	QVector<QString> words;
};

struct Replaces {
	Replaces(const QString &filename) : filename(filename) {
	}
	QString filename;
	QVector<Replace> list;
};

Replaces PrepareReplaces(const QString &filename);
bool CheckAndConvertReplaces(Replaces &replaces, const Data &data);

} // namespace emoji
} // namespace codegen
