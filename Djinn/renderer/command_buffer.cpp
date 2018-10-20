#include "command_buffer.h"
#include "../display/display.h"

namespace djinn::renderer {
	CommandBuffer::CommandBuffer(
		Display*               display,
		vk::CommandPool        pool,
		bool                   startImmediately,
		vk::QueueFlagBits      type,
		vk::CommandBufferLevel level
	):
		m_Display(display),
		m_Device (display->getVkDevice())
	{
		vk::CommandBufferAllocateInfo info;

		info
			.setCommandBufferCount(1)
			.setCommandPool       (pool)
			.setLevel             (level);

		auto bufferList = m_Device.allocateCommandBuffersUnique(info);
		assert(bufferList.size() == 1);

		m_CommandBuffer = std::move(bufferList.back());

		if (startImmediately)
			start();
	}

	void CommandBuffer::start(vk::CommandBufferUsageFlags usage) {
		vk::CommandBufferBeginInfo info;

		info.setFlags(usage);

		m_CommandBuffer->begin(info);
		m_IsRunning = true;
	}

	void CommandBuffer::stop() {
		m_CommandBuffer->end();
		m_IsRunning = false;
	}

	bool CommandBuffer::isRunning() const {
		return m_IsRunning;
	}

	void CommandBuffer::submit(
		vk::Semaphore signal,
		vk::Fence     fence,
		bool          createFence
	) {
		auto queue = getQueue();

		vk::SubmitInfo info;

		info
			.setCommandBufferCount(1)
			.setPCommandBuffers   (&*m_CommandBuffer);

		if (signal) {
			info
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores   (&signal);
		}

		bool fence_created = false;
		vk::UniqueFence unique_fence;

		if (!fence && createFence) {
			vk::FenceCreateInfo fci;

			unique_fence = m_Device.createFenceUnique()
		}
	}

	vk::CommandBuffer CommandBuffer::getHandle() const {
		return *m_CommandBuffer;
	}

	vk::Queue CommandBuffer::getQueue() const {
		switch (m_QueueType) {
		case vk::QueueFlagBits::eGraphics: return m_Display->getGraphicsQueue();
		case vk::QueueFlagBits::eCompute:  return m_Display->getComputeQueue();

		default:
			return vk::Queue();
		}
	}
}