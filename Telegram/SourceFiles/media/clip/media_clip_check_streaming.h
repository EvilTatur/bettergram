/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

namespace Media {
namespace Clip {

bool CheckStreamingSupport(
	const FileLocation &location,
	QByteArray data);

} // namespace Clip
} // namespace Media
