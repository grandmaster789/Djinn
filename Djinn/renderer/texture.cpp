#include "texture.h"
#include "buffer.h"
#include <numeric>

namespace djinn::renderer {
	namespace {
		uint32_t getMipLevels(
			uint32_t width, 
			uint32_t height
		) {
			auto largest = std::max(width, height);
			auto log2    = std::log2(largest);

			return static_cast<uint32_t>(
				std::floor(log2) + 1
			);
		}

		bool hasStencilComponent(vk::Format format) {
			switch (format) {
			case vk::Format::eD32SfloatS8Uint:
			case vk::Format::eD24UnormS8Uint:
				return true;

			default:
				return false;
			}
		}
	}

	Texture::Texture(
		vk::Device                 device,
		vk::PhysicalDevice         gpu,
		uint32_t                   width,
		uint32_t                   height,
		void*                      rawPixels,
		vk::Format                 format,
		vk::ImageLayout            layout,
		const vk::ImageUsageFlags& usage,
		vk::Filter                 filter,
		vk::SamplerAddressMode     addressMode,
		vk::SampleCountFlagBits    samples,
		bool                       anisotropic,
		bool                       buildMipmap
	):
		m_Device     (device),
		m_GPU        (gpu),
		m_Filter     (filter),
		m_AddressMode(addressMode),
		m_Anisotropic(anisotropic),
		m_Samples    (samples),
		m_Layout     (layout),
		m_Width      (width),
		m_Height     (height),
		m_Format     (format)
	{
		if (buildMipmap)
			m_MipLevels = getMipLevels(m_Width, m_Height);

		createImage(usage, vk::MemoryPropertyFlagBits::eDeviceLocal);

		if (
			(rawPixels != nullptr) ||
			(buildMipmap)
		)
			transitionImageLayout(
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal,
				m_MipLevels
			);
	}
	
	void Texture::createImage(
		vk::ImageUsageFlags     usage,
		vk::MemoryPropertyFlags mem_properties,
		uint32_t                layers
	) {
		vk::ImageCreateInfo imageInfo;

		imageInfo
			.setImageType    (vk::ImageType::e2D)
			.setExtent       ({ m_Width, m_Height, 1 })
			.setMipLevels    (m_MipLevels)
			.setArrayLayers  (layers)
			.setFormat       (m_Format)
			.setTiling       (vk::ImageTiling::eOptimal)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(
				usage |
				vk::ImageUsageFlagBits::eTransferSrc |
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled
			)
			.setSamples      (m_Samples)
			.setSharingMode  (vk::SharingMode::eExclusive);

		m_Image = m_Device.createImageUnique(imageInfo);

		auto mem_requirements = m_Device.getImageMemoryRequirements(*m_Image);

		vk::MemoryAllocateInfo memoryInfo;
		memoryInfo
			.setAllocationSize(mem_requirements.size)
			.setMemoryTypeIndex(
				Buffer::findMemoryType(
					m_GPU,
					mem_requirements.memoryTypeBits,
					mem_properties
				)
			);

		m_DeviceMemory = m_Device.allocateMemoryUnique(memoryInfo);
		m_Device.bindImageMemory(*m_Image, *m_DeviceMemory, 0);
	}

	void Texture::transitionImageLayout(
		vk::ImageLayout srcLayout,
		vk::ImageLayout dstLayout,
		uint32_t        mipLevels,
		uint32_t        baseArrayLayer,
		uint32_t        layerCount
	) {
		vk::ImageMemoryBarrier barrier;
		vk::ImageSubresourceRange subrange;
		
		barrier.oldLayout = srcLayout;
		barrier.newLayout = dstLayout;
		barrier.image     = *m_Image;

		if (dstLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			vk::ImageAspectFlags flags = vk::ImageAspectFlagBits::eDepth;

			if (hasStencilComponent(m_Format))
				flags |= vk::ImageAspectFlagBits::eStencil;

			subrange.setAspectMask(flags);
		}
		else
			subrange.setAspectMask(vk::ImageAspectFlagBits::eColor);
		
		subrange
			.setBaseArrayLayer(baseArrayLayer)
			.setLayerCount    (layerCount)
			.setBaseMipLevel  (0)
			.setLevelCount    (mipLevels);

		barrier.setSubresourceRange(subrange);

		vk::PipelineStageFlags srcStageMask;
		vk::PipelineStageFlags dstStageMask;

		if (
			srcLayout == vk::ImageLayout::eUndefined &&
			dstLayout == vk::ImageLayout::eTransferDstOptimal
		) {
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

			srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (
			srcLayout == vk::ImageLayout::eTransferDstOptimal &&
			dstLayout == vk::ImageLayout::eShaderReadOnlyOptimal
		) {
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (
			srcLayout == vk::ImageLayout::eTransferDstOptimal &&
			dstLayout == vk::ImageLayout::eColorAttachmentOptimal
		) {
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (
			srcLayout == vk::ImageLayout::eUndefined &&
			dstLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal
		) {
			barrier.setDstAccessMask(
				vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite
			);

			srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else if (
			srcLayout == vk::ImageLayout::eUndefined &&
			dstLayout == vk::ImageLayout::eColorAttachmentOptimal
		) {
			barrier.setDstAccessMask(
				vk::AccessFlagBits::eColorAttachmentRead |
				vk::AccessFlagBits::eColorAttachmentWrite
			);

			srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else
			assert(false); // unsupported layout transition


	}
}