#pragma once

#include "enum.h"

namespace djinn::util {
    template <typename E, E a, E z>
    EnumIterator<E, a, z>::EnumIterator():
        m_Cursor(static_cast<value_type>(a))
    {
    }

    template <typename E, E a, E z>
    EnumIterator<E, a, z>::EnumIterator(const E& e):
        m_Cursor(static_cast<int>(e))
    {
    }

    template <typename E, E a, E z>
    EnumIterator<E, a, z>& EnumIterator<E, a, z>::operator ++ () {
        ++m_Cursor;
        return *this;
    }

    template <typename E, E a, E z>
    E EnumIterator<E, a, z>::operator * () const {
        return static_cast<E>(m_Cursor);
    }

    template <typename E, E a, E z>
    EnumIterator<E, a, z> EnumIterator<E, a, z>::begin() {
        return EnumIterator(a);
    }

    template <typename E, E a, E z>
    EnumIterator<E, a, z> EnumIterator<E, a, z>::end() {
        return ++EnumIterator(z);
    }

    template <typename E, E a, E z>
    bool EnumIterator<E, a, z>::operator == (const EnumIterator& ei) {
        return ei.m_Cursor == m_Cursor;
    }

    template <typename E, E a, E z>
    bool EnumIterator<E, a, z>::operator != (const EnumIterator& ei) {
        return !(*this == ei);
    }
}