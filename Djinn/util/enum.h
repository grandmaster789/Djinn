#pragma once

#include <tuple>

namespace djinn::util {
    /*
        This allows for [index, value] ranged iteration over a collection. 
        All template arguments are deduced, just point it at some STL-compatible
        collection and it should work, with no unnecessary overhead.

        Given something iterable, a ranged for-loop using enumerate will 
        yield [index, element] tuples, which can be picked apart with a structured
        binding, resulting in a very compact usage syntax:

            vector<string> v = { "foo", "bar", "baz" };
            for (auto [index, value]: enumerate(v))
                std::cout << index << " -> " << value << "\n"

        yields:

            0 -> foo
            1 -> bar
            2 -> baz

        See unit test for example usage
    */

    template <
        typename T,
        typename tIterator = decltype(std::begin(std::declval<T>())),
        typename           = decltype(std::end(  std::declval<T>()))
    >
    constexpr auto enumerate(T&& iterable);
}

#include "enum.inl"