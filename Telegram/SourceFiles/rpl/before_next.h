/*
This file is part of Bettergram.

For license and copyright information please follow this link:
https://github.com/bettergram/bettergram/blob/master/LEGAL
*/
#pragma once

#include <rpl/producer.h>
#include <rpl/filter.h>

namespace rpl {

template <typename SideEffect>
inline auto before_next(SideEffect &&method) {
	return filter([method = std::forward<SideEffect>(method)](
			const auto &value) {
		method(value);
		return true;
	});
}

} // namespace rpl
