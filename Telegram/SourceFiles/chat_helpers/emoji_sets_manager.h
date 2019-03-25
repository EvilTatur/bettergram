/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "boxes/abstract_box.h"

namespace Ui {
namespace Emoji {

class ManageSetsBox : public BoxContent {
public:
	explicit ManageSetsBox(QWidget*);

protected:
	void prepare() override;

};

} // namespace Emoji
} // namespace Ui
