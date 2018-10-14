#include "swapchain.h"
#include "renderer.h"
#include "display/window.h"
#include "display/display.h"
#include "util/algorithm.h"

namespace djinn::renderer {
    Swapchain::Swapchain(
        const Window&   window,
              Display*  display,
              Renderer* render
    ) {
        const auto& gpu = display->getVkPhysicalDevice();
        auto device     = display->getVkDevice();

        auto availablePresentModes = gpu.getSurfacePresentModesKHR(window.getSurface());

        {
            using util::contains;
            using Mode = vk::PresentModeKHR;

            // prefer mailbox, fallback to immediate and if that fails, go for fifo (which is guaranteed)
                 if (contains(availablePresentModes, Mode::eMailbox))   m_PresentMode = Mode::eMailbox;
            else if (contains(availablePresentModes, Mode::eImmediate)) m_PresentMode = Mode::eImmediate;
            else                                                        m_PresentMode = Mode::eFifo;
        }

        {
            vk::SwapchainCreateInfoKHR info;

            auto capsMinImageCount = window.getSurfaceCaps().minImageCount;
            auto capsMaxImageCount = window.getSurfaceCaps().maxImageCount;

            auto imageCount = capsMinImageCount + 1; // default to one more than the minimum
            if (
                (capsMaxImageCount > 0) &&           // but if a maximum was set
                (imageCount > capsMaxImageCount)     // and one more than the minimum was more than that
            )
                imageCount = capsMaxImageCount;      // set to the maximum

            // default to the window size, then clip to the surface capabilities
            m_Extent.setWidth (window.getSize().first);
            m_Extent.setHeight(window.getSize().second);

            auto capsMinExtent = window.getSurfaceCaps().minImageExtent;
            auto capsMaxExtent = window.getSurfaceCaps().maxImageExtent;

            m_Extent.width = 
                std::max(capsMinExtent.width, 
                    std::min(
                        capsMaxExtent.width, 
                        m_Extent.width
                    )
                );

            m_Extent.height =
                std::max(capsMinExtent.height,
                    std::min(
                        capsMinExtent.height,
                        m_Extent.height
                    )
                );

            info
                .setSurface         (window.getSurface())
                .setMinImageCount   (imageCount)
                .setImageFormat     (window.getSurfaceFormat().format)
                .setImageColorSpace (window.getSurfaceFormat().colorSpace)
                .setImageExtent     (m_Extent)
                .setImageArrayLayers(1)
                .setImageUsage      (vk::ImageUsageFlagBits::eColorAttachment)
                .setPreTransform    (window.getSurfaceCaps().currentTransform)
                .setCompositeAlpha  (vk::CompositeAlphaFlagBitsKHR::eOpaque)
                .setPresentMode     (m_PresentMode)
                .setClipped         (VK_TRUE);

            // when available, set image usage transfer bits
            if (window.getSurfaceCaps().supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
                info.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
            if (window.getSurfaceCaps().supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
                info.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;

            // ~~ if the window had some old swapchain, transfer it
            if (auto oldChain = render->getSwapchain())
                info.setOldSwapchain(oldChain->getHandle());

            // ~~ figure out if we should use concurrent sharing
            std::vector<uint32_t> queueFamilies;
            auto graphicsFamilyIdx = display->getGraphicsFamilyIdx();
            auto presentFamilyIdx  = display->getPresentFamilyIdx();

            if (graphicsFamilyIdx != presentFamilyIdx) {
                queueFamilies = {
                   graphicsFamilyIdx,
                   presentFamilyIdx
                };

                info
                    .setImageSharingMode     (vk::SharingMode::eConcurrent)
                    .setQueueFamilyIndexCount(2)
                    .setPQueueFamilyIndices  (queueFamilies.data());
            }
            else
                info.setImageSharingMode(vk::SharingMode::eExclusive);

            m_Handle = device.createSwapchainKHRUnique(info);
            m_Images = device.getSwapchainImagesKHR(m_Handle.get());
            
            // There is probably going to be some overlap with a Texture(View) class
            vk::ComponentMapping identityComponents = {
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity
            };

            m_Views.reserve(m_Images.size());
            for (uint32_t i = 0; i < m_Images.size(); ++i) {
                vk::ImageSubresourceRange range;
                range
                    .setAspectMask    (vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel  (0)
                    .setLevelCount    (1)
                    .setBaseArrayLayer(0)
                    .setLayerCount    (1);
                
                vk::ImageViewCreateInfo view_info;
                view_info
                    .setImage           (m_Images[i])
                    .setViewType        (vk::ImageViewType::e2D)
                    .setFormat          (window.getSurfaceFormat().format)
                    .setComponents      (identityComponents)
                    .setSubresourceRange(range);

                m_Views.push_back(device.createImageViewUnique(view_info));
            }
        }
    }


    vk::SwapchainKHR Swapchain::getHandle() const {
        return m_Handle.get();
    }

    const vk::Extent2D& Swapchain::getExtent() const {
        return m_Extent;
    }

    bool Swapchain::hasSameExtent(const vk::Extent2D& extent) const {
        return (extent == m_Extent);
    }

    uint32_t Swapchain::getNumImages() const {
        return static_cast<uint32_t>(m_Images.size());
    }

    const std::vector<vk::Image>& Swapchain::getImages() const {
        return m_Images;
    }

    std::vector<vk::ImageView> Swapchain::getViews() const {
        using namespace std;

        std::vector<vk::ImageView> views;
        views.reserve(m_Views.size());

        for (const auto& v : m_Views)
            views.push_back(v.get());

        return views;
    }
}