#include "vk_command_manager.h"
#include "vk_command_buffer.h"
#include <cassert>

namespace djinn::graphics {
    VkCommandManager::VkCommandManager(
        vk::Device         device,
        QueueFamilyIndices families,
        size_t             threadCount
    ):
        m_Device(device)
    {
        // create graphics command pools
        m_Pools.resize(threadCount);

        for (auto& pool : m_Pools) {
            vk::CommandPoolCreateInfo info = {};

            info
                .setQueueFamilyIndex(families.m_GraphicsFamilyIdx)
                .setFlags           (vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            pool = m_Device.createCommandPoolUnique(info);

            if (!pool)
                throw std::runtime_error("Failed to create graphics command pool");
        }

        // transfer command pool
        {
            vk::CommandPoolCreateInfo info = {};

            info
                .setQueueFamilyIndex(families.m_TransferFamilyIdx)
                .setFlags           (vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            m_TransferPool = m_Device.createCommandPoolUnique(info);

            if (!m_TransferPool)
                throw std::runtime_error("Failed to create transfer command pool");
        }

        // single use command pool
        {
            vk::CommandPoolCreateInfo info = {};

            info
                .setQueueFamilyIndex(families.m_TransferFamilyIdx)
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            m_SingleUsePool = m_Device.createCommandPoolUnique(info);

            if (!m_SingleUsePool)
                throw std::runtime_error("Failed to create single use command pool");
        }
    }

    size_t VkCommandManager::getNumPools() const {
        return m_Pools.size();
    }

    vk::CommandPool VkCommandManager::getPool(size_t idx) const {
        assert(idx < m_Pools.size());
        return *m_Pools[idx];
    }

    vk::CommandPool VkCommandManager::getTransferPool() const {
        return *m_TransferPool;
    }

    vk::CommandPool VkCommandManager::getSingleUsePool() const {
        return *m_SingleUsePool;
    }

    VkCommandBuffer VkCommandManager::createCommandBuffer(vk::CommandBufferLevel level) {
        auto result = VkCommandBuffer(this, m_Device, level, m_NextPoolIdx);

        (++m_NextPoolIdx) %= m_Pools.size();

        return result;
    }
}