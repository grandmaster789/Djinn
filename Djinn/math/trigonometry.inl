#pragma once

#include "trigonometry.h"

namespace djinn::math {
    template <typename T>
    constexpr T deg2rad(T degrees) {
        return degrees * pi<T> / static_cast<T>(180.0);
    }

    template <typename T>
    constexpr T rad2deg(T radians) {
        return radians * static_cast<T>(180.0) / pi<T>;
    }
}  // namespace djinn::math
