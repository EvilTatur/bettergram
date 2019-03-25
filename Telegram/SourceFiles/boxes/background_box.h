/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "boxes/abstract_box.h"

namespace Data {
class WallPaper;
} // namespace Data

class BackgroundBox : public BoxContent {
public:
	BackgroundBox(QWidget*);

protected:
	void prepare() override;

private:
	class Inner;

	void removePaper(const Data::WallPaper &paper);

	QPointer<Inner> _inner;

};
