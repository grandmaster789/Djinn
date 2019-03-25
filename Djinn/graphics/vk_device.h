#pragma once

#include "queueFamilyIndices.h"
#include "swapchainDetails.h"
#include "third_party.h"

namespace djinn::graphics {
    /*
        Grouped together typical device-related resources
    */
    class VkDevice {
    public:
        VkDevice(
            const vk::Instance&             instance,
            const vk::SurfaceKHR&           surface,
            const std::vector<const char*>& requiredLayers,
            const std::vector<const char*>& requiredDeviceExtensions
        );

        const vk::PhysicalDevice& getPhysicalDevice() const;

        vk::Device get();

        vk::Queue& getGraphcisQueue();
        vk::Queue& getPresentQueue();
        vk::Queue& getTransferQueue();

        QueueFamilyIndices getQueueFamilyIndices() const;
        SwapchainDetails getSwapchainDetails() const;

    private:
        vk::Instance   m_Instance;
        vk::SurfaceKHR m_Surface;

        vk::PhysicalDevice m_PhysicalDevice;
        vk::UniqueDevice   m_LogicalDevice;
        
        QueueFamilyIndices m_QueueFamilyIndices;

        vk::Queue m_GraphicsQueue;
        vk::Queue m_PresentQueue;
        vk::Queue m_TransferQueue;
    };
}
