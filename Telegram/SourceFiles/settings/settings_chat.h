/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "settings/settings_common.h"

namespace Settings {

void SetupDataStorage(not_null<Ui::VerticalLayout*> container);
void SetupAutoDownload(not_null<Ui::VerticalLayout*> container);
void SetupDefaultThemes(not_null<Ui::VerticalLayout*> container);
void SetupSupport(not_null<Ui::VerticalLayout*> container);

class Chat : public Section {
public:
	Chat(QWidget *parent, not_null<UserData*> self);

private:
	void setupContent();

	not_null<UserData*> _self;

};

} // namespace Settings
