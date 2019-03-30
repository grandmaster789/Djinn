#pragma once

#include <cstdint>
#include "util/dynamicBitset.h"

namespace djinn::resources {
	using ResourceID = uint32_t;

	class AllocatorBase	{
	public:
		virtual ~AllocatorBase() = default;

		bool isAllocated(ResourceID id) const;

		virtual ResourceID allocate()                = 0;
		virtual void       allocateID(ResourceID id) = 0;
		virtual void       deallocate()              = 0;

		virtual void resize(size_t count) = 0;

		size_t getNumAllocated() const;
		size_t getMaxAllocated() const;

	protected:
		AllocatorBase(size_t capacity);

	};
}
