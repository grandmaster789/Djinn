#pragma once

#include "dependencies.h"

namespace djinn::renderer {
	class Buffer {
	public:
		Buffer(
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags properties
		);

		static uint32_t findMemoryType(
			vk::PhysicalDevice      gpu,
			uint32_t                typeFilter,
			vk::MemoryPropertyFlags requiredProperties
		);

	protected:
		vk::DeviceSize         m_Size;
		vk::UniqueBuffer       m_Buffer;
		vk::UniqueDeviceMemory m_Memory;
	};
}