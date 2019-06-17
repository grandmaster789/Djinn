#include "swapchain.h"
#include "util/algorithm.h"

namespace {
    uint32_t selectMemoryTypeIndex(
        vk::PhysicalDevice      gpu,
        uint32_t                typeBits,
        vk::MemoryPropertyFlags properties) {
        auto props = gpu.getMemoryProperties();

        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if ((typeBits & 1) == 1) {
                if ((props.memoryTypes[i].propertyFlags & properties) == properties) return i;
            }

            typeBits >>= 1;  // NOTE not entirely sure about this
        }

        return 0;
    }
}  // namespace

namespace djinn::graphics {
    Swapchain::Swapchain(
        vk::Device         device,
        vk::PhysicalDevice physical,
        vk::SurfaceKHR     surface,
        vk::Format         imageFormat,
        uint32_t           presentFamilyIdx,
        Swapchain*         oldSwapchain) {
        vk::SwapchainCreateInfoKHR info;

        auto caps = physical.getSurfaceCapabilitiesKHR(surface);

        m_Extent = caps.currentExtent;

        vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque)
            compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
            compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
        else
            throw std::runtime_error("No composite alpha is supported");

        auto preferredPresentMode = *util::prefer(
            physical.getSurfacePresentModesKHR(surface),
            vk::PresentModeKHR::eMailbox,
            vk::PresentModeKHR::eImmediate,
            vk::PresentModeKHR::eFifoRelaxed,
            vk::PresentModeKHR::eFifo);

        info.setSurface(surface)
            .setMinImageCount(std::max(caps.minImageCount, 2u))
            .setImageFormat(imageFormat)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(m_Extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices(&presentFamilyIdx)
            .setPresentMode(preferredPresentMode)
            .setOldSwapchain(oldSwapchain ? oldSwapchain->getHandle() : nullptr)
            .setCompositeAlpha(compositeAlpha)
            .setPreTransform(caps.currentTransform);

        m_Handle      = device.createSwapchainKHRUnique(info);
        m_ColorImages = device.getSwapchainImagesKHR(*m_Handle);

        // views for all of the swapchain images
        for (const auto& img : m_ColorImages) {
            vk::ImageSubresourceRange range;

            range.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setBaseMipLevel(0)
                .setLevelCount(1);

            // [NOTE] using default channel swizzles here
            vk::ImageViewCreateInfo view_info;

            view_info.setImage(img)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(imageFormat)
                .setSubresourceRange(range);

            m_ColorViews.push_back(device.createImageViewUnique(view_info));
        }
    }

    vk::SwapchainKHR Swapchain::getHandle() const {
        return *m_Handle;
    }

    vk::Extent2D Swapchain::getExtent() const noexcept {
        return m_Extent;
    }

}  // namespace djinn::graphics
