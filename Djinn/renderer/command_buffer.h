#pragma once

#include "dependencies.h"

namespace djinn {
	class Display;
}

namespace djinn::renderer {
	class CommandBuffer {
	public:
		CommandBuffer(
			Display*               display,
			vk::CommandPool        pool,
			bool                   startImmediately = true,
			vk::QueueFlagBits      type             = vk::QueueFlagBits::eGraphics,
			vk::CommandBufferLevel level            = vk::CommandBufferLevel::ePrimary
		);

		void start(vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		void stop();
		bool isRunning() const;

		void submit(
			vk::Semaphore signal      = vk::Semaphore(),
			vk::Fence     fence       = vk::Fence(),
			bool          createFence = false
		);

		vk::CommandBuffer getHandle() const;

	private:
		vk::Queue getQueue() const;

		Display*        m_Display;
		vk::Device      m_Device;

		vk::QueueFlagBits       m_QueueType; // [NOTE] not a mask, just 1 queue plx
		vk::CommandBufferLevel  m_Level;
		vk::UniqueCommandBuffer m_CommandBuffer;
		bool                    m_IsRunning = false;
	};
}
