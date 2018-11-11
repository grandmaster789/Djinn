#include "stdafx.h"
#include "indicator.h"
#include <ostream>

Indicator::Indicator(const Indicator&):
    m_Val(2)
{
}

Indicator& Indicator::operator = (const Indicator&) {
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
    case 1: os << "Default constructed"; break;
    case 2: os << "Copy constructed";    break;
    case 3: os << "Copy assigned";       break;
    case 4: os << "Move constructed";    break;
    case 5: os << "Moved from husk";     break;
    case 6: os << "Move assigned";       break;

    default:
        os << "<< UNKNOWN STATE >>";
    }

    return os;
}
