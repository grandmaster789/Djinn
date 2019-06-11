#pragma once

#include <ostream>

namespace djinn::util::reflect {
	// most of this is based around the talk given by Antony Polukhin, enhanced with various C++17 things

	// Major caveat - doesn't seem to work correctly with private/protected fields...
	// Minor caveat - if a constructor is specified with more arguments than there are members,
	//                this will also fail.
	// Minor caveat - current implementation supports up to 20 members

	// I'm seriously considering using macros here, there is some repetitive copy-pasta going on here that
	// I don't believe I can replace with metaprogramming...

	namespace detail {
		struct Wildcard {
			// provide the signature for converting to *any* type
			// [NOTE] this may have to be restricted somewhat
			template <typename T>
			operator T() const;
		};

		template <size_t N = 0>
		static constexpr const Wildcard g_Wildcard{};
	}  // namespace detail

	template <size_t N>
	using NumFields = std::integral_constant<size_t, N>;  // maybe make this a strong type?

	// --------------- toTuple -------------------------
	// The resulting tuple will contain references to the object fields
	// [NOTE] could also do copies or pointers I guess, but references
	//        seem most general-purpose
	template <typename T>
	auto toTuple(T& obj) noexcept;

	// ---------------- forEachField ---------------------
	template <typename T, typename Fn>
	void forEachField(T& obj, Fn&& callback);

	template <typename T, typename Fn>
	void forEachField(Fn&& callback);  // default-constructs an object to apply the callback to
}  // namespace djinn::util::reflect

#include "reflect.inl"
