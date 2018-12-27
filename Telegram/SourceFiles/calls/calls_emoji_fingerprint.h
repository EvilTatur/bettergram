/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Calls {

class Call;

std::vector<EmojiPtr> ComputeEmojiFingerprint(not_null<Call*> call);

} // namespace Calls
