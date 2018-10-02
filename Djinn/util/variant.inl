#pragma once

#include "variant.h"

namespace djinn::util {
    template <typename...Lambdas>
    template <typename...Ts>
    constexpr OverloadSet<Lambdas...>::OverloadSet(Ts&&...ts):
        Ts{ std::forward<Ts>(ts) }...
    {
    }

    template <typename...Lambdas>
    auto match(Lambdas&&... lambdas) {
        return [visitor = OverloadSet(std::forward<Lambdas>(lambdas)...)] (auto&& variant) {
            return std::visit(visitor, std::forward<decltype(variant)>(variant));
        };
    }
}