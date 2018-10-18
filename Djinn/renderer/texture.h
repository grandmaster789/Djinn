#pragma once

#include "dependencies.h"
#include <cstdint>

namespace djinn::renderer{
	class Texture {
	public:
		Texture(
			vk::Device                 device,
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
			vk::SampleCountFlags       samples     = vk::SampleCountFlagBits::e1,
			bool                       anisotropic = false,
			bool                       mipmap      = false
		);

	private:
		vk::Device          m_Device;
		vk::UniqueImage     m_Image;
		vk::UniqueImageView m_View;
		vk::DeviceMemory    m_DeviceMemory;
		vk::ImageLayout     m_Layout;
		vk::Format          m_Format;

		vk::Filter             m_Filter;
		vk::Sampler            m_Sampler;
		vk::SampleCountFlags   m_Samples;
		vk::SamplerAddressMode m_AddressMode;

		bool     m_Anisotropic;
		uint32_t m_MipLevels  = 1;
		uint32_t m_Components = 4;
		uint32_t m_Width;
		uint32_t m_Height;

	};
}
