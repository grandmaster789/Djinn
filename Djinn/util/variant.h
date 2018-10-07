#pragma once

#include <variant>

namespace djinn::util {
    template <typename...Lambdas>
    struct OverloadSet: 
        Lambdas... 
    {
        using Lambdas::operator()...;

        // Jason Turner indicated that this constructor is not needed in
        //  C++ Weekly #134: https://www.youtube.com/watch?v=EsUmnLgz8QY
        // but I found that with the current version of MSVC this will
        // cause memory corruption issues. 
        // Just keeping the constructor will make it work mostly as intended, 
        // although still not in a constexpr context.

        template <typename...Ts>
        constexpr OverloadSet(Ts&&... ts);
    };

    template <typename...Ts>
    OverloadSet(Ts...) -> OverloadSet<Ts...>;

    /*
        Note that the lambda signatures for match() should follow multimethod
        conventions, not sequenced(accumulate-like).
        (see unit tests for samples)
    */
    template <typename...Lambdas>
    auto match(Lambdas&&... lambdas);
}

#include "variant.inl"