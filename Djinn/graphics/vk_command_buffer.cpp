#include "vk_command_buffer.h"
#include "vk_command_manager.h"

namespace djinn::graphics {
    VkCommandBuffer::VkCommandBuffer(
        VkCommandManager*      manager,
        vk::Device             device,
        vk::CommandBufferLevel level,
        int                    threadID
    ):
        m_Manager (manager),
        m_Device  (device),
        m_Level   (level),
        m_ThreadID(threadID)
    {
        vk::CommandBufferAllocateInfo info = {};

        info.commandPool        = m_Manager->getPool(threadID);
        info.level              = m_Level;
        info.commandBufferCount = 1;

        m_Buffer = std::move(m_Device.allocateCommandBuffersUnique(info)[0]);

        if (!m_Buffer)
            throw std::runtime_error("Failed to allocate command buffer");
    }

    VkCommandManager& VkCommandBuffer::getManager() {
        return *m_Manager;
    }

    const VkCommandManager& VkCommandBuffer::getManager() const {
        return *m_Manager;
    }

    vk::CommandBufferLevel VkCommandBuffer::getLevel() const {
        return m_Level;
    }

    int VkCommandBuffer::getThreadID() const {
        return m_ThreadID;
    }
}