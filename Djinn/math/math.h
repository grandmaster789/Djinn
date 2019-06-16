#pragma once

#include <type_traits>

namespace djinn::math {
	template <typename T>
	constexpr T alignToSmaller(T value, T alignment);

	template <typename T>
	constexpr T alignToLarger(T value, T alignment);

	template <typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
	constexpr int roundToInt(T value);  // NOTE disregards special floats like NaN and inf

	template <typename T, typename U>
	constexpr auto max(const T& v0, const U& v1);

	template <typename T, typename... Ts>
	constexpr auto max(const T& v0, const Ts&... vs);

	template <typename T, typename U>
	constexpr auto min(const T& v0, const U& v1);

	template <typename T, typename... Ts>
	constexpr auto min(const T& v0, const Ts&... vs);
}  // namespace djinn::math

#include "math.inl"