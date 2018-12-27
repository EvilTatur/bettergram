/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "boxes/abstract_box.h"

class ChangePhoneBox : public BoxContent {
public:
	ChangePhoneBox(QWidget*) {
	}

protected:
	void prepare() override;

	void paintEvent(QPaintEvent *e) override;

private:
	class EnterPhone;
	class EnterCode;

};

