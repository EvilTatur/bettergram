/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Support {

bool ValidateAccount(const MTPUser &self);

enum class SwitchSettings {
	None,
	Next,
	Previous,
};

Qt::KeyboardModifiers SkipSwitchModifiers();
bool HandleSwitch(Qt::KeyboardModifiers modifiers);
FnMut<bool()> GetSwitchMethod(SwitchSettings value);

} // namespace Support
