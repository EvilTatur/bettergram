/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include "platform/win/wrapper_windows_h.h"

namespace Platform {
namespace AppUserModelId {

void cleanupShortcut();
void checkPinned();

const WCHAR *getId();
bool validateShortcut();

const PROPERTYKEY &getKey();

} // namespace AppUserModelId
} // namespace Platform
