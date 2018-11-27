#include "command_buffer.h"

namespace djinn::display {
    CommandBuffer::CommandBuffer() {
    }

    vk::CommandBuffer CommandBuffer::getHandle() const {
    }

    bool CommandBuffer::init(vk::Device device, uint32_t queueFamilyIndex) {
    }

    bool CommandBuffer::start() {
    }

    bool CommandBuffer::stop() {
    }

    bool CommandBuffer::run() {
    }

    bool CommandBuffer::wait() {
    }

    bool CommandBuffer::reset() {
    }

    void CommandBuffer::setState(eState s) {
    }

    CommandBuffer::eState CommandBuffer::getState() const {
    }

    std::ostream& operator << (std::ostream& os, const CommandBuffer::eState& state) {
    }

    std::ostream& operator << (std::ostream& os, const CommandBuffer& cb) {
    }
}