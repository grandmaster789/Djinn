#pragma once

#include <utility>
#include <tuple>
#include <type_traits>

#include "reflect.h"

namespace djinn::util::reflect {
	namespace detail {
		// is_brace_constructible<T, N>() is a traits function that will inform us if it is possible
		// to initialize an object T with N number of arguments using aggregate initialization
		//

		template <typename T, size_t...Is>
		constexpr auto is_brace_constructible_impl(std::index_sequence<Is...>, T*)
			-> decltype(T{ g_Wildcard<Is>... }, std::true_type{}); // comma expression, yield std::true_type iff the first part compiles

		template <size_t...Is>
		constexpr std::false_type is_brace_constructible_impl(std::index_sequence<Is...>, ...); // use ... to force the lowest matching priority in the overload set

		// this will end up either as std::true_type or std::false_type
		template <typename T, size_t N>
		constexpr auto is_brace_constructible()
			-> decltype(is_brace_constructible_impl(
				std::make_index_sequence<N>(),
				static_cast<T*>(nullptr)
			))
		{
			return {};
		}
	}

	// --------------- toTuple -------------------------

	// count upwards and try to find the maximum number where brace construction would fail
	// NOTE maybe this could have better heuristics by using std::is_constructible as well
	// NOTE currently choosing to use a reference as the parameter, 
	// NOTE this is a prime candidate for macro's

	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  0>() && !detail::is_brace_constructible<T,  1>()>> constexpr NumFields< 0> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  1>() && !detail::is_brace_constructible<T,  2>()>> constexpr NumFields< 1> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  2>() && !detail::is_brace_constructible<T,  3>()>> constexpr NumFields< 2> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  3>() && !detail::is_brace_constructible<T,  4>()>> constexpr NumFields< 3> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  4>() && !detail::is_brace_constructible<T,  5>()>> constexpr NumFields< 4> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  5>() && !detail::is_brace_constructible<T,  6>()>> constexpr NumFields< 5> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  6>() && !detail::is_brace_constructible<T,  7>()>> constexpr NumFields< 6> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  7>() && !detail::is_brace_constructible<T,  8>()>> constexpr NumFields< 7> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  8>() && !detail::is_brace_constructible<T,  9>()>> constexpr NumFields< 8> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T,  9>() && !detail::is_brace_constructible<T, 10>()>> constexpr NumFields< 9> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 10>() && !detail::is_brace_constructible<T, 11>()>> constexpr NumFields<10> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 11>() && !detail::is_brace_constructible<T, 12>()>> constexpr NumFields<11> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 12>() && !detail::is_brace_constructible<T, 13>()>> constexpr NumFields<12> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 13>() && !detail::is_brace_constructible<T, 14>()>> constexpr NumFields<13> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 14>() && !detail::is_brace_constructible<T, 15>()>> constexpr NumFields<14> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 15>() && !detail::is_brace_constructible<T, 16>()>> constexpr NumFields<15> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 16>() && !detail::is_brace_constructible<T, 17>()>> constexpr NumFields<16> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 17>() && !detail::is_brace_constructible<T, 18>()>> constexpr NumFields<17> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 18>() && !detail::is_brace_constructible<T, 19>()>> constexpr NumFields<18> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 19>() && !detail::is_brace_constructible<T, 20>()>> constexpr NumFields<19> getFieldCount(const T&) { return {}; }
	template <typename T, typename = std::enable_if_t<detail::is_brace_constructible<T, 20>() && !detail::is_brace_constructible<T, 21>()>> constexpr NumFields<20> getFieldCount(const T&) { return {}; }

	template <typename T> auto toTuple(T&, NumFields< 0>) {
		return std::tie();
	}

	template <typename T> auto toTuple(T& obj, NumFields< 1>) {
		auto&          [f0] = obj;
		return std::tie(f0);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 2>) {
		auto&          [f0, f1] = obj;
		return std::tie(f0, f1);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 3>) {
		auto&          [f0, f1, f2] = obj;
		return std::tie(f0, f1, f2);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 4>) {
		auto&          [f0, f1, f2, f3] = obj;
		return std::tie(f0, f1, f2, f3);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 5>) {
		auto&          [f0, f1, f2, f3, f4] = obj;
		return std::tie(f0, f1, f2, f3, f4);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 6>) {
		auto&          [f0, f1, f2, f3, f4, f5] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 7>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 8>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7);
	}

	template <typename T> auto toTuple(T& obj, NumFields< 9>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8);
	}

	template <typename T> auto toTuple(T& obj, NumFields<10>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9);
	}

	template <typename T> auto toTuple(T& obj, NumFields<11>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);
	}

	template <typename T> auto toTuple(T& obj, NumFields<12>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11);
	}

	template <typename T> auto toTuple(T& obj, NumFields<13>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);
	}

	template <typename T> auto toTuple(T& obj, NumFields<14>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);
	}

	template <typename T> auto toTuple(T& obj, NumFields<15>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14);
	}

	template <typename T> auto toTuple(T& obj, NumFields<16>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15);
	}

	template <typename T> auto toTuple(T& obj, NumFields<17>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16);
	}

	template <typename T> auto toTuple(T& obj, NumFields<18>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17);
	}

	template <typename T> auto toTuple(T& obj, NumFields<19>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18);
	}

	template <typename T> auto toTuple(T& obj, NumFields<20>) {
		auto&          [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19] = obj;
		return std::tie(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19);
	}

	template <typename T> auto toTuple(T& obj) {
		return toTuple(obj, getFieldCount(obj));
	}

	// ---------------- forEachField ---------------------
	template <typename T, typename Fn>
	void forEachField(T& obj, Fn&& callback) {
		if constexpr (std::is_pointer_v<T>) {
			// dereference the pointer and try again
			if (obj != nullptr)
				forEachField(*obj, std::forward<Fn>(callback));
			else
				throw std::runtime_error("reflect::forEachField() - Cannot apply callback to nullptr");
		}
		else if constexpr (std::is_scalar_v<T>) {
			callback(obj); // no need to convert to a tuple if it's just a single value
		}
		else {
			// convert to a tuple and apply the callback to each of the fields
			std::apply(
				[&](auto&&...args) {
					(callback(args), ...);
				},
				toTuple(obj)
			);
		}
	}

	template <typename T, typename Fn>
	void forEachField(Fn&& callback) {
		// default construct an instance of T and apply the callback to that object
		T obj;
		forEachField(obj, std::forward<Fn>(callback));
	}
}