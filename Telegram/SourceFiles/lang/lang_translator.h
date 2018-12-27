/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Lang {

class Translator : public QTranslator {
public:
	QString translate(const char *context, const char *sourceText, const char *disambiguation = 0, int n = -1) const override;

};

} // namespace Lang
