#pragma once

#include "third_party.h"

namespace djinn::graphics {
	struct SwapchainDetails {
		vk::SurfaceCapabilitiesKHR        m_Capabilities = {};
		std::vector<vk::SurfaceFormatKHR> m_Formats;
		std::vector<vk::PresentModeKHR>   m_PresentModes;
	};

	class Swapchain {
	public:
		Swapchain(
			const vk::PhysicalDevice& physical,
			const vk::Device&         logical,
			const vk::SurfaceKHR&     surface,
			uint32_t                  width,
			uint32_t                  height
		);
		// TODO figure out how to reuse old swapchains

		vk::SwapchainKHR getHandle() const;

		size_t getNumImages() const;
		const vk::Image&     getImage(size_t idx) const;
		const vk::ImageView& getView (size_t idx) const;

		vk::Format   getFormat() const;
		vk::Extent2D getExtent() const;

		static SwapchainDetails querySupport(
			const vk::PhysicalDevice& gpu,
			const vk::SurfaceKHR&     surface
		);

	private:
		static vk::SurfaceFormatKHR pickFormat     (const std::vector<vk::SurfaceFormatKHR>& formats);
		static vk::PresentModeKHR   pickPresentMode(const std::vector<vk::PresentModeKHR>&   modes);
		static vk::Extent2D         pickExtent     (const vk::SurfaceCapabilitiesKHR& caps, uint32_t width, uint32_t height);

		vk::Device m_Device;
		vk::UniqueSwapchainKHR m_Handle;

		std::vector<vk::Image>           m_Images;
		std::vector<vk::UniqueImageView> m_Views;

		vk::Format   m_Format;
		vk::Extent2D m_Extent;
	};
}
