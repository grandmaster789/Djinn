#pragma once

#include <type_traits>

namespace djinn::util {
    /*
        This allows ranged iteration over enum classes

        Caveat: the values must be fully sequential
    */
    template <
        typename Enum, 
        Enum beginValue,
        Enum endValue
    >
    class EnumIterator {
    public:
        using value_type = std::underlying_type_t<Enum>;

        EnumIterator();
        EnumIterator(const Enum& e);

        EnumIterator& operator ++ ();

        Enum operator * () const;

        static EnumIterator begin();
        static EnumIterator end();

        bool operator == (const EnumIterator& ei);
        bool operator != (const EnumIterator& ei);

    private:
        int m_Cursor;
    };
}

#include "enum.inl"