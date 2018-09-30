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
    os << i.m_Val;
    return os;
}
