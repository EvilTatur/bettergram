/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include <rpl/producer.h>

namespace rpl {

template <typename Value = empty_value, typename Error = no_error>
inline auto never() {
	return make_producer<Value, Error>([](const auto &consumer) {
		return lifetime();
	});
}

} // namespace rpl
