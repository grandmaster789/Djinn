#pragma once

#include <utility>
#include <tuple>
#include <type_traits>

#include "reflect.h"

namespace djinn::util::reflect {
	namespace detail {
		template <class T, class... Vs>
		auto is_constructible_with_impl(T*)
			-> decltype(T{ std::declval<Vs>()... }, std::true_type{});

		template <class, class...>
		auto is_constructible_with_impl(...)
			->std::false_type;

		template <class T, class... TArgs>
		using is_constructible_with = decltype(is_constructible_with_impl<T, TArgs...>(nullptr));

		// class that can convert to anything
		// (intentionally only specify signature)
		struct Wildcard {
			template <class T>
			constexpr operator T();
		};
	}

	template <class T>
	auto to_tuple(T&& object) noexcept {
		using DecayedType = std::decay_t<T>;
		using Q           = detail::Wildcard;

		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12, v13, v14
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12, v13, v14
			);
		} else 
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12, v13
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12, v13
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q,
			Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11, v12
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q,
			Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10, v11
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q,
			Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9,
				v10
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8, v9
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7, v8
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6, v7
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6, v7
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5, v6
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5, v6
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q,
			Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4,
				v5
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4,
				v5
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3, v4
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3, v4
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2, v3
			] = object;

			return std::make_tuple(
				v0, v1, v2, v3
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q, Q
		>()) {
			auto&&[
				v0, v1, v2
			] = object;

			return std::make_tuple(
				v0, v1, v2
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q, Q
		>()) {
			auto&&[
				v0, v1
			] = object;

			return std::make_tuple(
				v0, v1
			);
		} else
		if constexpr (detail::is_constructible_with<DecayedType,
			Q
		>()) {
			auto&&[
				v0
			] = object;

			return std::make_tuple(
				v0
			);
		}
		else
		{
			return std::make_tuple();
		}
	}

	// -------- ostream& to_ostream(ostream, T) ------------------
	namespace detail {
		template <int I, typename...Ts>
		auto stream(std::ostream& os, const std::tuple<Ts...>& tup)
			-> typename std::enable_if<(I == sizeof...(Ts))>::type
		{
		}

		template <int I, typename...Ts>
		auto stream(std::ostream& os, const std::tuple<Ts...>& tup)
			-> typename std::enable_if<(I == (sizeof...(Ts) - 1))>::type
		{
			os << std::get<I>(tup);
		}

		template <int I, typename...Ts>
		typename std::enable_if<(I < (sizeof...(Ts) - 1))>::type 
			stream(std::ostream& os, const std::tuple<Ts...>& tup) 
		{
			os << std::get<I>(tup) << ", ";
			stream<I + 1>(os, tup);
		}
	}

	template <class T>
	std::ostream& to_ostream(std::ostream& os, const T& object) {
		auto tup = to_tuple(object);
		
		os << '(';
		detail::stream<0>(os, tup);
		os << ')';

		return os;
	}
}