#pragma once

namespace djinn::math {
	template <class T>
	constexpr T pi = T(3.1415926535897932385L);

	template <typename T>
	constexpr T deg2rad(T degrees);

	template <typename T>
	constexpr T rad2deg(T radians);
}  // namespace djinn::math

#include "trigonometry.inl"
