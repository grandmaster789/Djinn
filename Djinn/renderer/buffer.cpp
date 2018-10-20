#include "buffer.h"
#include <cassert>

namespace djinn::renderer {
	uint32_t Buffer::findMemoryType(
		vk::PhysicalDevice      gpu,
		uint32_t                typeFilter,
		vk::MemoryPropertyFlags requiredProperties
	) {
		auto mem_props = gpu.getMemoryProperties();

		for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
			uint32_t bitMask = 1 << i;
			auto     flags   = mem_props.memoryTypes[i].propertyFlags;

			bool isCorrectType        = typeFilter & bitMask;
			bool hasCorrectProperties = ((flags & requiredProperties) == flags);

			if (isCorrectType && hasCorrectProperties)
				return i;
		}

		// detect the failure case in debug builds
		assert(false);
		return 0;
	}
}