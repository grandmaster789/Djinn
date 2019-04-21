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
            vk::ImageView      depthView,
            uint32_t           graphicsFamilyIdx,
            vk::RenderPass     renderpass,
            Swapchain*         oldSwapchain = nullptr
        );

        vk::SwapchainKHR getHandle() const;
        vk::Framebuffer  getFramebuffer(uint32_t idx) const;
        vk::Format       getImageFormat() const noexcept;
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
        vk::UniqueSemaphore m_ImageAvailableSemaphore;
        vk::UniqueSemaphore m_PresentCompletedSemaphore;

        vk::UniqueSwapchainKHR             m_Handle;

        std::vector<vk::Image>             m_Images;
        std::vector<vk::UniqueImageView>   m_ImageViews;
        std::vector<vk::UniqueFramebuffer> m_Framebuffers;

        uint32_t     m_ActiveImage = 0;
        vk::Format   m_ImageFormat = vk::Format::eB8G8R8A8Unorm;
        vk::Extent2D m_Extent      = vk::Extent2D(0, 0);
    };
}
