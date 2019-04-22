#include "swapchain.h"
#include "graphicsUtility.h"
#include "util/algorithm.h"

namespace djinn::graphics {
    Swapchain::Swapchain(
        vk::Device         device,
        vk::PhysicalDevice physical,
        vk::SurfaceKHR     surface,
        vk::Format         imageFormat,
        vk::Format         depthFormat,
        uint32_t           graphicsFamilyIdx,
        vk::RenderPass     renderpass,
        Swapchain*         oldSwapchain
    ):
        m_ImageFormat(imageFormat),
        m_DepthFormat(depthFormat)
    {
        m_ImageAvailableSemaphore   = device.createSemaphoreUnique({});
        m_PresentCompletedSemaphore = device.createSemaphoreUnique({});

        vk::SwapchainCreateInfoKHR info;

        auto caps = physical.getSurfaceCapabilitiesKHR(surface);

		m_Extent = caps.currentExtent;

        vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

             if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque)         compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)        compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)  compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
        else if (caps.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
        else
            throw std::runtime_error("No composite alpha is supported");

        auto preferredPresentMode = *util::prefer(
            physical.getSurfacePresentModesKHR(surface),
            vk::PresentModeKHR::eMailbox,
            vk::PresentModeKHR::eImmediate,
            vk::PresentModeKHR::eFifoRelaxed,
            vk::PresentModeKHR::eFifo
        );

        info
            .setSurface              (surface)
            .setMinImageCount        (std::max(caps.minImageCount, 2u))
            .setImageFormat          (imageFormat)
            .setImageColorSpace      (vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent          (m_Extent)
            .setImageArrayLayers     (1)
            .setImageUsage           (vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode     (vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices  (&graphicsFamilyIdx)
            .setPresentMode          (preferredPresentMode)
            .setOldSwapchain         (oldSwapchain ? oldSwapchain->getHandle() : nullptr)
            .setCompositeAlpha       (compositeAlpha)
            .setPreTransform         (caps.currentTransform);

        m_Handle      = device.createSwapchainKHRUnique(info);
        m_ColorImages = device.getSwapchainImagesKHR(*m_Handle);

        // views for all of the swapchain images
        for (const auto& img : m_ColorImages) {
            vk::ImageSubresourceRange range;

            range
                .setAspectMask    (vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount    (1)
                .setBaseMipLevel  (0)
                .setLevelCount    (1);

            vk::ImageViewCreateInfo view_info;

            view_info
                .setImage           (img)
                .setViewType        (vk::ImageViewType::e2D)
                .setFormat          (imageFormat)
                .setSubresourceRange(range);

            m_ColorViews.push_back(device.createImageViewUnique(view_info));
        }

        // create a depth buffer
        createDepthBuffer(
            device,
            physical,
            graphicsFamilyIdx
        );

        // framebuffers for all of the swapchain images
        for (const auto& colorView : m_ColorViews) {
            vk::FramebufferCreateInfo fb_info;

            const vk::ImageView colorDepth[] = {
                *colorView,
                *m_DepthView
            };

            fb_info
                .setWidth          (m_Extent.width)
                .setHeight         (m_Extent.height)
                .setLayers         (1)
                .setAttachmentCount(2) // color and depth
                .setPAttachments   (colorDepth)
                .setRenderPass     (renderpass);

            m_Framebuffers.push_back(device.createFramebufferUnique(fb_info));
        }
    }

    vk::SwapchainKHR Swapchain::getHandle() const {
        return *m_Handle;
    }

    vk::Framebuffer Swapchain::getFramebuffer(uint32_t idx) const {
        return *m_Framebuffers[idx];
    }

    vk::Format Swapchain::getImageFormat() const noexcept {
        return m_ImageFormat;
    }

    vk::Format Swapchain::getDepthFormat() const noexcept
    {
        return m_DepthFormat;
    }

    vk::Extent2D Swapchain::getExtent() const noexcept {
        return m_Extent;
    }

    uint32_t Swapchain::acquireNextImage(vk::Device device) {
        m_ActiveImage = device.acquireNextImageKHR(
            *m_Handle,
            std::numeric_limits<uint64_t>::max(), // timeout
            *m_ImageAvailableSemaphore,
            vk::Fence()
        ).value;

        return m_ActiveImage;
    }

    // this will switch image layouts+access for a typical image:
    // color aspect, all mip levels, all array layers,
    // ignore queue families
    vk::ImageMemoryBarrier Swapchain::imageBarrier(
        uint32_t           imageIndex,
        vk::AccessFlagBits srcAccess,
        vk::ImageLayout    srcLayout,
        vk::AccessFlagBits dstAccess,
        vk::ImageLayout    dstLayout
    ) const {
        vk::ImageMemoryBarrier result;

        vk::ImageSubresourceRange range;
        range
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLevelCount(VK_REMAINING_MIP_LEVELS)
            .setLayerCount(VK_REMAINING_ARRAY_LAYERS);

        result
            .setImage              (m_ColorImages[imageIndex])
            .setSubresourceRange   (range)
            .setSrcAccessMask      (srcAccess)
            .setOldLayout          (srcLayout)
            .setDstAccessMask      (dstAccess)
            .setNewLayout          (dstLayout)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

        return result;
    }

    void Swapchain::present(
        vk::Device        device,
        vk::Queue         graphicsQueue,
        vk::CommandBuffer graphicsCommands
    ) const {
        {
            vk::SubmitInfo info;

            const vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

            info
                .setWaitSemaphoreCount  (1)
                .setPWaitSemaphores     (&*m_ImageAvailableSemaphore)
                .setPWaitDstStageMask   (&stageMask)
                .setCommandBufferCount  (1)
                .setPCommandBuffers     (&graphicsCommands)
                .setSignalSemaphoreCount(1)
                .setPSignalSemaphores   (&*m_PresentCompletedSemaphore);

            graphicsQueue.submit({ info }, {});
        }

        {
            vk::PresentInfoKHR info;

            info
                .setSwapchainCount    (1)
                .setPSwapchains       (&*m_Handle)
                .setPImageIndices     (&m_ActiveImage)
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores   (&*m_PresentCompletedSemaphore);

            graphicsQueue.presentKHR(info);
        }

        device.waitIdle();
    }

    void Swapchain::createDepthBuffer(
        vk::Device         device,
        vk::PhysicalDevice gpu,
        uint32_t           graphicsFamilyIdx
    ) {
        vk::ImageCreateInfo imageInfo;

        imageInfo
            .setImageType            (vk::ImageType::e2D)
            .setFormat               (m_DepthFormat)
            .setExtent               ({ m_Extent.width, m_Extent.height, 1 })
            .setArrayLayers          (1)
            .setMipLevels            (1)
            .setSamples              (vk::SampleCountFlagBits::e1)
            .setTiling               (vk::ImageTiling::eOptimal)
            .setUsage                (vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc)
            .setSharingMode          (vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices  (&graphicsFamilyIdx)
            .setInitialLayout        (vk::ImageLayout::eUndefined);

        m_DepthImage = device.createImageUnique(imageInfo);

        auto req = device.getImageMemoryRequirements(*m_DepthImage);

        vk::MemoryAllocateInfo allocInfo;

        allocInfo
            .setAllocationSize(req.size)
            .setMemoryTypeIndex(
                selectMemoryTypeIndex(
                    gpu,
                    req.memoryTypeBits,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                )
            );

        m_DepthBuffer = device.allocateMemoryUnique(allocInfo);
        device.bindImageMemory(
            *m_DepthImage,
            *m_DepthBuffer,
            0 // offset
        );

        vk::ImageViewCreateInfo viewInfo;
        vk::ImageSubresourceRange range;

        range
            .setAspectMask    (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)
            .setBaseArrayLayer(0)
            .setLayerCount    (1)
            .setBaseMipLevel  (0)
            .setLevelCount    (1);

        viewInfo
            .setImage           (*m_DepthImage)
            .setViewType        (vk::ImageViewType::e2D)
            .setFormat          (m_DepthFormat)
            .setSubresourceRange(range);

        m_DepthView = device.createImageViewUnique(viewInfo);
    }
}