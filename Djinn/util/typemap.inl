#pragma once

#include "typemap.h"
#include <stdexcept>

namespace djinn::util {
	template <typename T>
	void TypeMap::insert(T* ptr) {
		std::type_index info(typeid(T));

		if (m_Storage.find(info) == m_Storage.end())
			m_Storage[info] = ptr;
		else
			throw std::runtime_error("Cannot store multiple pointers of the same type");
	}

	template <typename T>
	void TypeMap::remove(T* ptr) {
		if (!ptr)
			m_Storage.erase(typeid(T));
		else {
			auto it = m_Storage.find(typeid(T));
			if (it->second == ptr)
				m_Storage.erase(it);
			else
				throw std::runtime_error(
				    "The supplied pointer does not match the stored pointer");
		}
	}

	template <typename T>
	T* TypeMap::get() const {
		auto it = m_Storage.find(typeid(T));

		if (it == m_Storage.end())
			return nullptr;
		else
			return static_cast<T*>(it->second);
	}
}  // namespace djinn::util
