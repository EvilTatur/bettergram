/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#include "mtproto/facade.h"

#include "storage/localstorage.h"
#include "messenger.h"

namespace MTP {
namespace internal {
namespace {

int PauseLevel = 0;

} // namespace

bool paused() {
	return PauseLevel > 0;
}

void pause() {
	++PauseLevel;
}

void unpause() {
	--PauseLevel;
	if (!PauseLevel) {
		if (auto instance = MainInstance()) {
			instance->unpaused();
		}
	}
}

} // namespace internal

Instance *MainInstance() {
	return Messenger::Instance().mtp();
}

} // namespace MTP
