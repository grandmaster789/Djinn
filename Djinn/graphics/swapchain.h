#pragma once

#include "third_party.h"
#include "window.h"

namespace djinn::graphics {
    class Swapchain {
    public:
        Swapchain(
            vk::Device         device,
            vk::PhysicalDevice physical,
            vk::SurfaceKHR     surface,
            vk::Format         imageFormat,
            vk::Format         depthFormat,
            uint32_t           presentFamilyIdx,
            Swapchain*         oldSwapchain = nullptr);

        vk::SwapchainKHR getHandle() const;
        vk::Extent2D     getExtent() const noexcept;

        vk::ImageView getDepthView() const;
        vk::ImageView getColorView(size_t idx) const;
        size_t        getNumColorViews() const;

    private:
        vk::Extent2D m_Extent;
        vk::Format   m_ImageFormat;
        vk::Format   m_DepthFormat;

        vk::UniqueSwapchainKHR m_Handle;

        std::vector<vk::Image>           m_ColorImages;
        std::vector<vk::UniqueImageView> m_ColorViews;

        vk::UniqueDeviceMemory m_DepthBuffer;
        vk::UniqueImage        m_DepthImage;
        vk::UniqueImageView    m_DepthView;
    };
}  // namespace djinn::graphics
