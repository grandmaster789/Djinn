#pragma once

#include "variant.h"

namespace djinn::util {
	template <typename... Lambdas>
	template <typename... Ts>
	constexpr OverloadSet<Lambdas...>::OverloadSet(Ts&&... ts) noexcept:
	    Ts{std::forward<Ts>(ts)}... {}

	template <typename... Lambdas>
	constexpr auto match(Lambdas&&... lambdas) {
		return [visitor = OverloadSet(std::forward<Lambdas>(lambdas)...)](auto&&... variants) {
			return std::visit(visitor, std::forward<decltype(variants)>(variants)...);
		};
	}
}  // namespace djinn::util
