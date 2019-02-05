#pragma once

#include <ostream>

namespace djinn::util::reflect {
	// Major caveat - this will only work for aggregate types
	// Minor caveat - if a constructor is specified with more
	//                arguments than there are members, this
	//                will also fail.
	// Minor caveat - current implementation supports up to 15
	//                members

	// Side note - seriously considering using nontrivial macro's here

	template <class T>
	auto to_tuple(T&& object) noexcept;

	template <class T>
	std::ostream& to_ostream(std::ostream& os, const T& object); // dodge around making operator << ambiguous
}

#include "reflect.inl"