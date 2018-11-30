#include "command_buffer.h"
#include "core/logger.h"

namespace djinn::display {
    CommandBuffer::CommandBuffer() {
    }

    vk::CommandBuffer CommandBuffer::getHandle() const {
		return *m_Handle;
    }

    void CommandBuffer::init(vk::Device device, uint32_t queueFamilyIndex) {
		m_Device = device;

		{
			vk::CommandPoolCreateInfo info;

			info
				.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
				.setQueueFamilyIndex(queueFamilyIndex);

			m_Pool = device.createCommandPoolUnique(info);
		}

		{
			vk::CommandBufferAllocateInfo info;

			info
				.setCommandPool       (*m_Pool)
				.setLevel             (vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

			auto buffers = device.allocateCommandBuffersUnique(info);
			m_Handle = std::move(buffers.back());
		}

		{
			vk::FenceCreateInfo info;

			m_ExecutionFence = device.createFenceUnique(info);
		}

		setState(eState::initialized);
    }

    void CommandBuffer::begin() {
		assert(m_State == eState::initialized);

		vk::CommandBufferBeginInfo info;
		m_Handle->begin(info);

		setState(eState::recording);
    }

    void CommandBuffer::end() {
		assert(m_State == eState::recording);
		
		m_Handle->end();
		
		setState(eState::executable);
    }

    void CommandBuffer::run(vk::Queue queue) {
		assert(m_State == eState::executable);

		vk::SubmitInfo info;

		info
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*m_Handle);

		queue.submit(1, &info, *m_ExecutionFence);

		setState(eState::running);
    }

    bool CommandBuffer::wait() {
		// if we're waiting while we're not running just return
		// [NOTE] maybe this should also be an assert
		if (m_State != eState::running)
			return true;

		// 5 retries, timouts at 1s
		constexpr uint64_t timeoutNanos = 1'000'000'000; // 1s

		for (int i = 0; i < 5; ++i) {
			if (m_Device.waitForFences(
				1,						// just 1 fence
				&*m_ExecutionFence,		// this one
				VK_TRUE,				// wait for all
				timeoutNanos			// timeout in nanoseconds
			) == vk::Result::eSuccess) {
				setState(eState::executable); // allow multiple executions of the commands
				return true;
			}

			gLogDebug << "Command buffer wait timeout... retrying";
		}

		return false;
    }

    void CommandBuffer::reset() {
		// just allow returning from executable to initialized state

		if (m_State != eState::initialized) {
			assert(m_State == eState::executable);

			vk::CommandBufferResetFlags flags;

			m_Device.resetFences(1, &*m_ExecutionFence);
			m_Handle->reset(flags);

			setState(eState::initialized);
		}
    }

    void CommandBuffer::setState(eState s) {
		m_State = s;
    }

    CommandBuffer::eState CommandBuffer::getState() const {
		return m_State;
    }

    std::ostream& operator << (std::ostream& os, const CommandBuffer::eState& state) {
		switch (state) {
		case CommandBuffer::eState::executable:  os << "executable";  break;
		case CommandBuffer::eState::initialized: os << "initialized"; break;
		case CommandBuffer::eState::recording:   os << "recording";   break;
		case CommandBuffer::eState::running:     os << "running";     break;
		case CommandBuffer::eState::undefined:   os << "undefined";   break;
		default:
			throw std::runtime_error("unsupported command buffer state");
		}

		return os;
    }

    std::ostream& operator << (std::ostream& os, const CommandBuffer& cb) {
		os << "CommandBuffer(" << cb.getState() << ")";

		return os;
    }
}