#pragma once

#include "third_party.h"
#include "window.h"

namespace djinn::graphics {
    class Swapchain
    {
    public:
        Swapchain(
            vk::Device         device,
            vk::PhysicalDevice physical,
            vk::SurfaceKHR     surface,
            vk::Format         imageFormat,
            uint32_t           presentFamilyIdx,
            Swapchain*         oldSwapchain = nullptr);

        vk::SwapchainKHR getHandle() const;
        vk::Extent2D     getExtent() const noexcept;

    private:
        vk::Extent2D m_Extent;

        vk::UniqueSwapchainKHR m_Handle;

        std::vector<vk::Image>           m_ColorImages;
        std::vector<vk::UniqueImageView> m_ColorViews;
    };
}  // namespace djinn::graphics
