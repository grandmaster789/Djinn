#pragma once

#include "third_party.h"
#include <cstdint>

namespace djinn::graphics {
    class Swapchain {
    public:
        Swapchain(
            vk::Device         device,
            vk::PhysicalDevice physical,
            vk::SurfaceKHR     surface,
            vk::Format         imageFormat,
            vk::Format         depthFormat,
            uint32_t           graphicsFamilyIdx,
            vk::RenderPass     renderpass,
            Swapchain*         oldSwapchain = nullptr
        );

        vk::SwapchainKHR getHandle() const;
        vk::Framebuffer  getFramebuffer(uint32_t idx) const;
        vk::Format       getImageFormat() const noexcept;
        vk::Format       getDepthFormat() const noexcept;
        vk::Extent2D     getExtent()      const noexcept;

        uint32_t acquireNextImage(vk::Device device);
        
        vk::ImageMemoryBarrier imageBarrier(
            uint32_t           imageIndex,
            vk::AccessFlagBits srcAccess,
            vk::ImageLayout    srcLayout,
            vk::AccessFlagBits dstAccess,
            vk::ImageLayout    dstLayout
        ) const;
        
        void present(
            vk::Device        device,
            vk::Queue         graphicsQueue,
            vk::CommandBuffer graphicsCommands
        ) const;

    private:
        void createDepthBuffer(
            vk::Device         device,
            vk::PhysicalDevice gpu,
            uint32_t           graphicsFamilyIdx
        );

        vk::UniqueSemaphore m_ImageAvailableSemaphore;
        vk::UniqueSemaphore m_PresentCompletedSemaphore;

        vk::UniqueSwapchainKHR             m_Handle;

        std::vector<vk::Image>             m_ColorImages;
        std::vector<vk::UniqueImageView>   m_ColorViews;
        vk::UniqueImage                    m_DepthImage;
        vk::UniqueDeviceMemory             m_DepthBuffer;
        vk::UniqueImageView                m_DepthView;
        
        std::vector<vk::UniqueFramebuffer> m_Framebuffers;

        uint32_t     m_ActiveImage = 0;
        vk::Format   m_ImageFormat = vk::Format::eB8G8R8A8Unorm;
        vk::Format   m_DepthFormat = vk::Format::eD24UnormS8Uint;
        vk::Extent2D m_Extent      = vk::Extent2D(0, 0);
    };
}
