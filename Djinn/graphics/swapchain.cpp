#include "swapchain.h"
#include "queueFamilyIndices.h"
#include "core/logger.h"
#include "util/algorithm.h"

namespace djinn::graphics {
	Swapchain::Swapchain(
		const vk::PhysicalDevice& physical,
		const vk::Device&         logical,
		const vk::SurfaceKHR&     surface,
		uint32_t                  width,
		uint32_t                  height
	):
		m_Device(logical)
	{
		SwapchainDetails details = querySupport(physical, surface);

		auto format = pickFormat     (details.m_Formats);
		auto mode   = pickPresentMode(details.m_PresentModes);
		auto extent = pickExtent     (details.m_Capabilities, width, height);

		uint32_t numImages = details.m_Capabilities.minImageCount + 1;
		if (details.m_Capabilities.maxImageCount > 0)
			numImages = std::min(numImages, details.m_Capabilities.maxImageCount);

		vk::SwapchainCreateInfoKHR info = {};
		info
			.setSurface         (surface)
			.setMinImageCount   (numImages)
			.setImageFormat     (format.format)
			.setImageColorSpace (format.colorSpace)
			.setImageExtent     (extent)
			.setImageArrayLayers(1)
			.setImageUsage      (vk::ImageUsageFlagBits::eColorAttachment);

		auto qfi = QueueFamilyIndices::findQueueFamilyIndices(physical, surface);

		std::array<uint32_t, 2> indices = {
			static_cast<uint32_t>(qfi.m_GraphicsFamilyIdx),
			static_cast<uint32_t>(qfi.m_PresentFamilyIdx)
		};

		// see if the present queue and the graphics queue are the same
		// and select a sharing mode accordingly
		if (qfi.m_GraphicsFamilyIdx != qfi.m_PresentFamilyIdx) {
			info
				.setImageSharingMode     (vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(2)
				.setPQueueFamilyIndices  (indices.data());
		}
		else {
			info
				.setImageSharingMode     (vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(0)
				.setPQueueFamilyIndices  (nullptr);
		}

		info
			.setPreTransform  (details.m_Capabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode   (mode)
			.setClipped       (VK_TRUE);

		// actually create the swapchain
		m_Handle = m_Device.createSwapchainKHRUnique(info);

		m_Images = m_Device.getSwapchainImagesKHR(*m_Handle);
		m_Format = format.format;
		m_Extent = extent;

		m_Views.resize(m_Images.size());

		for (size_t i = 0; i < m_Images.size(); ++i) {
			vk::ImageViewCreateInfo   ivci       = {};
			vk::ComponentMapping      components = {};
			vk::ImageSubresourceRange range      = {};

			components.r = vk::ComponentSwizzle::eIdentity;
			components.g = vk::ComponentSwizzle::eIdentity;
			components.b = vk::ComponentSwizzle::eIdentity;
			components.a = vk::ComponentSwizzle::eIdentity;

			range
				.setAspectMask    (vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel  (0)
				.setLevelCount    (1)
				.setBaseArrayLayer(0)
				.setLayerCount    (1);

			ivci
				.setImage           (m_Images[i])
				.setViewType        (vk::ImageViewType::e2D)
				.setFormat          (m_Format)
				.setComponents      (components)
				.setSubresourceRange(range);

			m_Views[i] = m_Device.createImageViewUnique(ivci);
		}
	}

	vk::SwapchainKHR Swapchain::getHandle() const {
		return *m_Handle;
	}

	size_t Swapchain::getNumImages() const {
		return m_Images.size();
	}

	const vk::Image& Swapchain::getImage(size_t idx) const {
		return m_Images[idx];
	}

	const vk::ImageView& Swapchain::getView(size_t idx) const {
		return *m_Views[idx];
	}

	vk::Format Swapchain::getFormat() const {
		return m_Format;
	}

	vk::Extent2D Swapchain::getExtent() const {
		return m_Extent;
	}

	SwapchainDetails Swapchain::querySupport(
		const vk::PhysicalDevice& gpu,
		const vk::SurfaceKHR&     surface
	) {
		SwapchainDetails result;

		result.m_Capabilities = gpu.getSurfaceCapabilitiesKHR(surface);
		result.m_Formats      = gpu.getSurfaceFormatsKHR     (surface);
		result.m_PresentModes = gpu.getSurfacePresentModesKHR(surface);

		return result;
	}

	vk::SurfaceFormatKHR Swapchain::pickFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
		vk::SurfaceFormatKHR rgba32_sRGBnonlinear;
		rgba32_sRGBnonlinear.format     = vk::Format::eB8G8R8A8Unorm;
		rgba32_sRGBnonlinear.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

		if (
			(formats.size() == 1) &&
			(formats[0].format == vk::Format::eUndefined)
		) {
			// this is a special case where the driver actually supports *all* formats
			// so we can pick anything we like
			return rgba32_sRGBnonlinear;
		}

		// prefer 32-bits BGRA with non-linear sRGB
		if (util::contains(formats, rgba32_sRGBnonlinear))
			return rgba32_sRGBnonlinear;

		// fallback to whatever is provided
		return formats[0];
	}

	vk::PresentModeKHR Swapchain::pickPresentMode(const std::vector<vk::PresentModeKHR>& modes) {
		return *util::prefer(
			modes,
			vk::PresentModeKHR::eMailbox,
			vk::PresentModeKHR::eImmediate,
			vk::PresentModeKHR::eFifoRelaxed,
			vk::PresentModeKHR::eFifo
		);
	}

	vk::Extent2D Swapchain::pickExtent(const vk::SurfaceCapabilitiesKHR& caps, uint32_t width, uint32_t height) {
		// if possible, use the current extent
		if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return caps.currentExtent;
		else {
			vk::Extent2D result;

			result.width  = std::max(caps.minImageExtent.width,  width);
			result.height = std::max(caps.minImageExtent.height, height);
			result.width  = std::min(caps.maxImageExtent.width,  result.width);
			result.height = std::min(caps.maxImageExtent.height, result.height);

			return result;
		}
	}
}