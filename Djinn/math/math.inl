#pragma once

#include "math.h"

namespace djinn::math {
	template <typename T>
	constexpr T alignToSmaller(T value, T alignment) {
		return (value / alignment) * alignment;
	}

	template <typename T>
	constexpr T alignToLarger(T value, T alignment) {
		return ((value + alignment - 1) / alignment) * alignment;
	}

	template <typename T, typename>
	constexpr int roundToInt(T value) {
		if (value < 0)
			return static_cast<int>(value - T(0.5));
		else
			return static_cast<int>(value + T(0.5));
	}

	template <typename T, typename U>
	constexpr auto max(const T& v0, const U& v1) {
		if (v0 < v1)
			return v1;
		else
			return v0;
	}

	template <typename T, typename...Ts>
	constexpr auto max(const T& v0, const Ts&... vs) {
		return max(v0, max(vs...));
	}

	template <typename T, typename U>
	constexpr auto min(const T& v0, const U& v1) {
		if (v0 < v1)
			return v0;
		else
			return v1;
	}

	template <typename T, typename...Ts>
	constexpr auto min(const T& v0, const Ts&... vs) {
		return min(v0, min(vs...));
	}
}
