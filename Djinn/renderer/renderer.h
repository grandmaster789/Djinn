#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"
#include "util/flat_map.h"
#include "swapchain.h"

#include <memory>

namespace djinn {
    //
    // interface for a vulkan graphics queue
    //
    class Renderer:
        public core::System
    {
    public:
        using Swapchain = renderer::Swapchain;

        Renderer();

        virtual void init() override;
        virtual void update() override;
        virtual void shutdown() override;

        virtual void unittest() override;

        Swapchain* getSwapchain() const;

    private:
        void createFences();
        void createCommandPool();
        void createPipelineCache();

        std::unique_ptr<Swapchain> m_Swapchain; // unique_ptr might not be the most suitable (but it's not horrible either)

        // non-owning handles to the instance+physical+logical device
        vk::Instance            m_Instance;
        vk::PhysicalDevice      m_PhysicalDevice;
        vk::Device              m_Device;

        // owning resources
        vk::UniqueFence         m_SwapchainImageFence;
        vk::UniqueSemaphore     m_Semaphore;
        vk::UniqueCommandPool   m_CommandPool;
        vk::UniquePipelineCache m_PipelineCache;
    };
}
