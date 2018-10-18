#include "texture.h"

namespace djinn::renderer {
	Texture::Texture(
		vk::Device                 device,
		uint32_t                   width,
		uint32_t                   height,
		void*                      rawPixels,
		vk::Format                 format,
		vk::ImageLayout            layout,
		const vk::ImageUsageFlags& usage,
		vk::Filter                 filter,
		vk::SamplerAddressMode     addressMode,
		vk::SampleCountFlags       samples,
		bool                       anisotropic,
		bool                       mipmap
	):
		m_Device     (device),
		m_Filter     (filter),
		m_AddressMode(addressMode),
		m_Anisotropic(anisotropic),
		m_Samples    (samples),
		m_Layout     (layout),
		m_Width      (width),
		m_Height     (height),
		m_Format     (format)
	{
	}
}