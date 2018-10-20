#pragma once

#include "dependencies.h"
#include <cstdint>

namespace djinn::renderer{
	class Texture {
	public:
		Texture(
			vk::Device                 device,
			vk::PhysicalDevice         gpu,
			uint32_t                   width,
			uint32_t                   height,
			void*                      rawPixels   = nullptr,
			vk::Format                 format      = vk::Format::eB8G8R8A8Unorm,
			vk::ImageLayout            layout      = vk::ImageLayout::eColorAttachmentOptimal,
			const vk::ImageUsageFlags& usage       = 
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eStorage,
			vk::Filter                 filter      = vk::Filter::eLinear,
			vk::SamplerAddressMode     addressMode = vk::SamplerAddressMode::eRepeat,
			vk::SampleCountFlagBits    samples     = vk::SampleCountFlagBits::e1,
			bool                       anisotropic = false,
			bool                       buildMipmap = false
		);

	private:
		void createImage(
			vk::ImageUsageFlags     usage, 
			vk::MemoryPropertyFlags mem_properties,
			uint32_t                layers = 1
		);

		void transitionImageLayout(
			vk::ImageLayout srcLayout,
			vk::ImageLayout dstLayout,
			uint32_t mipLevels,
			uint32_t baseArrayLayer = 0,
			uint32_t layerCount     = 1
		);

	private:
		vk::Device             m_Device;
		vk::PhysicalDevice     m_GPU;
		
		vk::UniqueImage        m_Image;
		vk::UniqueImageView    m_View;
		vk::UniqueDeviceMemory m_DeviceMemory;

		vk::ImageLayout         m_Layout;
		vk::Format              m_Format;
		vk::Filter              m_Filter;
		vk::Sampler             m_Sampler;
		vk::SampleCountFlagBits m_Samples;
		vk::SamplerAddressMode  m_AddressMode;

		bool     m_Anisotropic;
		uint32_t m_MipLevels  = 1;
		uint32_t m_Components = 4;
		uint32_t m_Width;
		uint32_t m_Height;

	};
}
