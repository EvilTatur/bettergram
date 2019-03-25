/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "boxes/abstract_box.h"

namespace Ui {
class Radiobutton;
} // namespace Ui

class AutoLockBox : public BoxContent {
public:
	AutoLockBox(QWidget*) {
	}

protected:
	void prepare() override;

private:
	void durationChanged(int seconds);

	std::vector<object_ptr<Ui::Radiobutton>> _options;

};
