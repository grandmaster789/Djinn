#include "vk_device.h"
#include "util/algorithm.h"
#include <set>

namespace djinn::graphics {
    VkDevice::VkDevice(
        const vk::Instance&             instance,
        const vk::SurfaceKHR&           surface,
        const std::vector<const char*>& requiredLayers,
        const std::vector<const char*>& requiredDeviceExtensions
    ):
        m_Instance(instance),
        m_Surface(surface)
    {
        // pick a physical device
        {
            auto allPhysicalDevices = instance.enumeratePhysicalDevices();
            if (allPhysicalDevices.empty())
                throw std::runtime_error("No vulkan-capable physical devices were found");

            uint32_t highscore = 0;

            auto supportsExtensions = [&](const vk::PhysicalDevice& gpu) {
                auto availableExtensions = gpu.enumerateDeviceExtensionProperties();

                for (const auto& needle: requiredDeviceExtensions)
                    if (!util::contains_if(
                        availableExtensions, 
                        [&](const vk::ExtensionProperties& extension) {
                            return std::string(extension.extensionName) == std::string(needle);
                        }
                    ))
                        return false;

                return true;
            };

            for (const auto& gpu : allPhysicalDevices) {
                uint32_t score = 0;

                auto properties = gpu.getProperties();
                auto features   = gpu.getFeatures();

                if (!supportsExtensions(gpu))
                    continue; // current GPU doesn't support a required extension, don't even consider it

                switch (properties.deviceType) {
                case vk::PhysicalDeviceType::eDiscreteGpu:   
                    score += 1000; 
                    break;

                case vk::PhysicalDeviceType::eIntegratedGpu: 
                    score += 500;  
                    break;

                default:
                    score += 100; // CPU, Virtual GPU or 'other'
                }

                if (features.geometryShader)
                    score += 100; // nice to have feature

                score += properties.limits.maxImageDimension2D; // image dimension is a big factor

                // require swapchain support
                auto formats = gpu.getSurfaceFormatsKHR(m_Surface);
                auto modes   = gpu.getSurfacePresentModesKHR(m_Surface);

                if (formats.empty() || modes.empty())
                    continue;

                // tally the score, keep it if it is the best so far
                if (score > highscore) {
                    highscore = score;
                    m_PhysicalDevice = gpu;
                }
            }

            if (!m_PhysicalDevice)
                throw std::runtime_error("Failed to pick a suitable physical device");
        }

        // find queue family indices
        {
			m_QueueFamilyIndices = QueueFamilyIndices::findQueueFamilyIndices(m_PhysicalDevice, m_Surface);

            // verify that we have all required queue types available
            if (!m_QueueFamilyIndices.isComplete())
                throw std::runtime_error("Not all required queue families were found");
        }

        // set up logical device & queues themselves
        std::vector<vk::DeviceQueueCreateInfo> queue_infos;
        std::vector<int> unique_families = {
            m_QueueFamilyIndices.m_GraphicsFamilyIdx,
            m_QueueFamilyIndices.m_TransferFamilyIdx,
            m_QueueFamilyIndices.m_PresentFamilyIdx
        };

        util::unique(unique_families);

        float priority = 1.0f;
        for (int family : unique_families) {
            vk::DeviceQueueCreateInfo info = {};

            info
                .setQueueFamilyIndex(family)
                .setQueueCount(1)
                .setPQueuePriorities(&priority);

            queue_infos.push_back(std::move(info));
        }

        auto allDeviceFeatures = m_PhysicalDevice.getFeatures();

        vk::DeviceCreateInfo dci = {};

        dci
            .setQueueCreateInfoCount   (static_cast<uint32_t>(queue_infos.size()))
            .setPQueueCreateInfos      (queue_infos.data())
            .setEnabledExtensionCount  (static_cast<uint32_t>(requiredDeviceExtensions.size()))
            .setPpEnabledExtensionNames(requiredDeviceExtensions.data())
            .setPEnabledFeatures       (&allDeviceFeatures)
            .setEnabledLayerCount      (static_cast<uint32_t>(requiredLayers.size()))
            .setPpEnabledLayerNames    (requiredLayers.data());

        m_LogicalDevice = m_PhysicalDevice.createDeviceUnique(dci);

        if (!m_LogicalDevice)
            throw std::runtime_error("Failed to create a logical vulkan device");

        m_GraphicsQueue = m_LogicalDevice->getQueue(static_cast<uint32_t>(m_QueueFamilyIndices.m_GraphicsFamilyIdx), 0);
        m_PresentQueue  = m_LogicalDevice->getQueue(static_cast<uint32_t>(m_QueueFamilyIndices.m_PresentFamilyIdx),  0);
        m_TransferQueue = m_LogicalDevice->getQueue(static_cast<uint32_t>(m_QueueFamilyIndices.m_TransferFamilyIdx), 0);
    }

    const vk::PhysicalDevice& VkDevice::getPhysicalDevice() const {
        return m_PhysicalDevice;
    }

    vk::Device VkDevice::get() {
        return m_LogicalDevice.get();
    }

    vk::Queue& VkDevice::getGraphcisQueue() {
        return m_GraphicsQueue;
    }

    vk::Queue& VkDevice::getPresentQueue() {
        return m_PresentQueue;
    }

    vk::Queue& VkDevice::getTransferQueue() {
        return m_TransferQueue;
    }

    QueueFamilyIndices VkDevice::getQueueFamilyIndices() const {
        return m_QueueFamilyIndices;
    }
}