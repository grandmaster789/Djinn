#include "stdafx.h"
#include "indicator.h"
#include <ostream>

Indicator::Indicator(const Indicator&) noexcept:
    m_Val(2)
{
}

Indicator& Indicator::operator = (const Indicator&) noexcept {
    m_Val = 3;
    return *this;
}

Indicator::Indicator(Indicator&& i) noexcept :
    m_Val(4)
{
    i.m_Val = 5;
}

Indicator& Indicator::operator = (Indicator&& i) noexcept {
    m_Val = 6;
    i.m_Val = 5;
    return *this;
}

bool Indicator::isDefaultConstructed() const noexcept { 
    return m_Val == 1; 
}

bool Indicator::isCopyConstructed() const noexcept { 
    return m_Val == 2; 
}

bool Indicator::isCopyAssigned() const noexcept { 
    return m_Val == 3; 
}

bool Indicator::isMoveConstructed() const noexcept { 
    return m_Val == 4; 
}

bool Indicator::isMovedFrom() const noexcept {
    return m_Val == 5; 
}

bool Indicator::isMoveAssigned() const noexcept { 
    return m_Val == 6; 
}

std::ostream& operator << (std::ostream& os, const Indicator& i) {
    switch (i.m_Val) {
    case 1: os << "1"; break; // default constructed
    case 2: os << "2"; break; // copy constructed
    case 3: os << "3"; break; // copy assigned
    case 4: os << "4"; break; // move constructed
    case 5: os << "5"; break; // moved from husk
    case 6: os << "6"; break; // move assigned

    default:
        os << "<< UNKNOWN STATE >>";
    }

    return os;
}
