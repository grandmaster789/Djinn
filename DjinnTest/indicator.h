#pragma once

#include <iosfwd>

struct Indicator {
    Indicator() = default; // -> m_Val == 1
    ~Indicator() = default;

    Indicator             (const Indicator&) noexcept;
    Indicator& operator = (const Indicator&) noexcept; 
    Indicator             (Indicator&& i) noexcept;
    Indicator& operator = (Indicator&& i) noexcept;

    bool isDefaultConstructed() const noexcept;
    bool isCopyConstructed()    const noexcept;
    bool isCopyAssigned()       const noexcept;
    bool isMoveConstructed()    const noexcept;
    bool isMovedFrom()          const noexcept;
    bool isMoveAssigned()       const noexcept;

    int m_Val = 1;
};

std::ostream& operator << (std::ostream& os, const Indicator& i);
