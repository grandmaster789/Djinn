#include "swapchain.h"
#include "graphics.h"
#include "util/algorithm.h"

namespace djinn::graphics {
    Swapchain::Swapchain(
        vk::Device         device,
        vk::PhysicalDevice physical,
        vk::SurfaceKHR     surface,
        vk::Format         imageFormat,
        vk::Format         depthFormat,
        uint32_t           presentFamilyIdx,
        Swapchain*         oldSwapchain):
        m_ImageFormat(imageFormat),
        m_DepthFormat(depthFormat) {
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

        // allocate depth buffer
        {
            auto props = physical.getFormatProperties(m_DepthFormat);

            vk::ImageCreateInfo ici;

            if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                ici.setTiling(vk::ImageTiling::eLinear);
            else if (
                props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                ici.setTiling(vk::ImageTiling::eOptimal);
            else
                throw std::runtime_error("Depth format is unsupported");

            vk::Extent3D iext(m_Extent.width, m_Extent.height, 1);

            ici.setImageType(vk::ImageType::e2D)
                .setFormat(m_DepthFormat)
                .setExtent(iext)
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setQueueFamilyIndexCount(0)
                .setPQueueFamilyIndices(nullptr)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);

            m_DepthImage = device.createImageUnique(ici);

            vk::ImageSubresourceRange range;
            range.setAspectMask(vk::ImageAspectFlagBits::eDepth)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            // if the format includes stencil bits, add that aspect
            switch (m_DepthFormat) {
            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
                range.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }

            vk::ComponentMapping components;
            components.setR(vk::ComponentSwizzle::eR)
                .setG(vk::ComponentSwizzle::eG)
                .setB(vk::ComponentSwizzle::eB)
                .setA(vk::ComponentSwizzle::eA);

            vk::ImageViewCreateInfo ivci;
            ivci.setImage(*m_DepthImage)
                .setFormat(m_DepthFormat)
                .setSubresourceRange(range)
                .setComponents(components)
                .setViewType(vk::ImageViewType::e2D);

            auto reqs = device.getImageMemoryRequirements(*m_DepthImage);

            vk::MemoryAllocateInfo mai;

            uint32_t memoryType = selectMemoryTypeIndex(
                physical, reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

            mai.setAllocationSize(reqs.size).setMemoryTypeIndex(memoryType);

            m_DepthBuffer = device.allocateMemoryUnique(mai);

            device.bindImageMemory(*m_DepthImage, *m_DepthBuffer, 0);
            m_DepthView = device.createImageViewUnique(ivci);
        }
    }

    vk::SwapchainKHR Swapchain::getHandle() const {
        return *m_Handle;
    }

    vk::Extent2D Swapchain::getExtent() const noexcept {
        return m_Extent;
    }

}  // namespace djinn::graphics
