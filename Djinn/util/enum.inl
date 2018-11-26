#pragma once

#include "enum.h"

namespace djinn::util {
    template <
        typename T,
        typename It,
        typename
    >
    constexpr auto enumerate(T&& iterable) {
        struct Iterator {
            bool operator != (const Iterator& it) const {
                return m_Iterator != it.m_Iterator;
            }

            void operator ++ () {
                ++m_Index;
                ++m_Iterator;
            }

            auto operator* () const {
                return std::tie(m_Index, *m_Iterator);
            }

            size_t m_Index;
            It     m_Iterator;
        };

        struct IteratorWrapper {
            T m_Iterable;

            auto begin() {
                return Iterator{ 0, std::begin(m_Iterable) };
            }

            auto end() {
                return Iterator{ 0, std::end(m_Iterable) };
            }
        };

        return IteratorWrapper{
            std::forward<T>(iterable)
        };
    }
}