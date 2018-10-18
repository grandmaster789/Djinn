#include "render_manager.h"
#include "core/engine.h"
#include "display/display.h"

namespace djinn {
	RenderManager::RenderManager():
        System("RenderManager")
    {
        addDependency("Display");
    }

    void RenderManager::init() {
        System::init();

        auto display = m_Engine->get<Display>();

        m_Instance       = display->getVkInstance();
        m_PhysicalDevice = display->getVkPhysicalDevice();
        m_Device         = display->getVkDevice();

        m_Swapchain = std::make_unique<Swapchain>(*display->getWindow(), display, this);

        createFences();
        createCommandPool();
        createPipelineCache();
    }

    void RenderManager::update() {
    }

    void RenderManager::shutdown() {
        System::shutdown();

        auto display = m_Engine->get<Display>();
        
        display->getGraphicsQueue().waitIdle();

        m_PipelineCache.reset();
        m_SwapchainImageFence.reset();
        m_Semaphore.reset();
        m_CommandPool.reset();

        m_Swapchain.reset();
    }

    void RenderManager::unittest() {
    }

	RenderManager::Swapchain* RenderManager::getSwapchain() const {
        return m_Swapchain.get();
    }

    void RenderManager::createFences() {
        auto display = m_Engine->get<Display>();
        auto device  = display->getVkDevice();

        m_SwapchainImageFence = device.createFenceUnique(vk::FenceCreateInfo());
        m_Semaphore           = device.createSemaphoreUnique(vk::SemaphoreCreateInfo());
    }

    void RenderManager::createCommandPool() {
        auto display           = m_Engine->get<Display>();
        auto device            = display->getVkDevice();
        auto graphicsFamilyIdx = display->getGraphicsFamilyIdx();

        vk::CommandPoolCreateInfo info;

        info
            .setQueueFamilyIndex(graphicsFamilyIdx)
            .setFlags(
                vk::CommandPoolCreateFlagBits::eTransient |
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer
            );

        m_CommandPool = device.createCommandPoolUnique(info);
        // ~~~~~~~~~~~~~~~~~~~~ TBA command buffer init !
    }

    void RenderManager::createPipelineCache() {
        auto display = m_Engine->get<Display>();
        auto device  = display->getVkDevice();

        m_PipelineCache = device.createPipelineCacheUnique(vk::PipelineCacheCreateInfo());
    }
}