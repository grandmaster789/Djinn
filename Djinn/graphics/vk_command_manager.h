#pragma once

#include "third_party.h"
#include "queueFamilyIndices.h"

namespace djinn::graphics {
    class VkCommandBuffer;

    class VkCommandManager {
    public:
        VkCommandManager() = default;
        
        VkCommandManager(
            vk::Device         device,
            QueueFamilyIndices families,
            size_t             threadCount
        );

        size_t getNumPools() const;

        vk::CommandPool getPool(size_t idx) const;
        vk::CommandPool getTransferPool() const;
        vk::CommandPool getSingleUsePool() const;

        VkCommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

    private:
        vk::Device m_Device;
        
        std::vector<vk::UniqueCommandPool> m_Pools; // all of these are for graphics

        vk::UniqueCommandPool m_TransferPool;
        vk::UniqueCommandPool m_SingleUsePool;

        size_t m_NextPoolIdx = 0; // non-atomic, so there may be spillage! might require atomic/mutexes
    };
}
