/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "settings/settings_common.h"

namespace Settings {

class PrivacySecurity : public Section {
public:
	PrivacySecurity(QWidget *parent, not_null<UserData*> self);

private:
	void setupContent();

	not_null<UserData*> _self;

};

} // namespace Settings
