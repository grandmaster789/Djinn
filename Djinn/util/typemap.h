#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>

namespace djinn {
	namespace util {
		// May store pointers to any unique type, and queried by the type
		// [NOTE] the pointers stored are in their raw form, thus pretty weak!
		// [NOTE] this is not threadsafe, but it's probably not an issue (I expect a very low rate of data mutation)
		class TypeMap {
		public:
			template <typename T>
			void insert(T* ptr); // will throw a runtime error if the type is already present

			template <typename T>
			void remove(T* ptr = nullptr); // interestingly, the value of the argument is optional here; this syntax allows both explicit templates without a parameter and argument type deduction for the template type

			template <typename T>
			T* get() const; // will return nullptr if the type is not contained

            void clear();

		private:
			std::unordered_map<std::type_index, void*> m_Storage;
		};
	}
}

#include "typemap.inl"
