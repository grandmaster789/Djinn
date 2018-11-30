#pragma once

#include "third_party.h"

namespace djinn::display {
    class CommandBuffer {
    public:
        enum class eState {
            undefined,
            initialized,
            recording,
            executable,
            running
        };

        CommandBuffer();

        CommandBuffer             (const CommandBuffer&) = delete;
        CommandBuffer& operator = (const CommandBuffer&) = delete;
        CommandBuffer             (CommandBuffer&&)      = default;
        CommandBuffer& operator = (CommandBuffer&&)      = default;

        vk::CommandBuffer getHandle() const;

        void init(vk::Device device, uint32_t queueFamilyIndex); // TODO: make this typesafe

        void begin();
        void end();
        void run(vk::Queue queue);
        bool wait();
        void reset();

        void setState(eState s);
        eState getState() const;

    private:
        eState m_State = eState::undefined;

        vk::UniqueFence         m_ExecutionFence;
        vk::UniqueCommandBuffer m_Handle;
        vk::UniqueCommandPool   m_Pool;

		vk::Device m_Device;
    };

    std::ostream& operator << (std::ostream& os, const CommandBuffer::eState& state);
    std::ostream& operator << (std::ostream& os, const CommandBuffer& cb);
}
