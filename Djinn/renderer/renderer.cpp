#include "renderer.h"
#include "core/engine.h"
#include "display/display.h"

namespace djinn {
    Renderer::Renderer():
        System("Renderer")
    {
        addDependency("Display");
    }

    void Renderer::init() {
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

    void Renderer::update() {
    }

    void Renderer::shutdown() {
        System::shutdown();

        auto display = m_Engine->get<Display>();
        
        display->getGraphicsQueue().waitIdle();

        m_PipelineCache.reset();
        m_SwapchainImageFence.reset();
        m_Semaphore.reset();
        m_CommandPool.reset();

        m_Swapchain.reset();
    }

    void Renderer::unittest() {
    }

    Renderer::Swapchain* Renderer::getSwapchain() const {
        return m_Swapchain.get();
    }

    void Renderer::createFences() {
        auto display = m_Engine->get<Display>();
        auto device  = display->getVkDevice();

        m_SwapchainImageFence = device.createFenceUnique(vk::FenceCreateInfo());
        m_Semaphore           = device.createSemaphoreUnique(vk::SemaphoreCreateInfo());
    }

    void Renderer::createCommandPool() {
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

    void Renderer::createPipelineCache() {
        auto display = m_Engine->get<Display>();
        auto device  = display->getVkDevice();

        m_PipelineCache = device.createPipelineCacheUnique(vk::PipelineCacheCreateInfo());
    }
}