#pragma once

#include "third_party.h"
#include <cstdint>

namespace djinn::context {
    class Swapchain {
    public:
        Swapchain(
            vk::Device         device,
            vk::PhysicalDevice physical,
            vk::SurfaceKHR     surface,
            vk::Format         imageFormat,
            uint32_t           width,
            uint32_t           height,
            uint32_t           graphicsFamilyIdx,
            vk::RenderPass     renderpass,
            Swapchain*         oldSwapchain = nullptr
        );

        vk::SwapchainKHR getHandle() const;
        vk::Framebuffer  getFramebuffer(uint32_t idx) const;
        vk::Format       getImageFormat() const;
        vk::Extent2D     getExtent() const;

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

        uint32_t     m_ActiveImage;
        vk::Format   m_ImageFormat;
        vk::Extent2D m_Extent;
    };
}
