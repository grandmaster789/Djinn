#pragma once

#include "third_party.h"

namespace djinn::graphics {
    class VkCommandManager;

    class VkCommandBuffer {
    public:
        VkCommandBuffer() = default;
        
        VkCommandBuffer(
            VkCommandManager*      manager,
            vk::Device             device,
            vk::CommandBufferLevel level    = vk::CommandBufferLevel::ePrimary,
            int                    threadID = 0
        );

        VkCommandBuffer             (const VkCommandBuffer& other) = default;
        VkCommandBuffer& operator = (const VkCommandBuffer& other) = default;
        VkCommandBuffer             (VkCommandBuffer&&)            = default;
        VkCommandBuffer& operator = (VkCommandBuffer&&)            = default;
        
              VkCommandManager& getManager();
        const VkCommandManager& getManager() const;

        vk::CommandBufferLevel getLevel() const;

        int getThreadID() const;

    private:
        VkCommandManager*       m_Manager = nullptr;
        vk::Device              m_Device;
        vk::UniqueCommandBuffer m_Buffer;
        vk::CommandBufferLevel  m_Level = vk::CommandBufferLevel::ePrimary;

        int m_ThreadID = 0;
    };
}
