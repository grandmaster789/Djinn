#pragma once

#include "renderer.h"
#include "resources/resources.h"

namespace djinn::graphics {
	class ForwardRendererBase:
		public Renderer
	{
	public:
		ForwardRendererBase();
		ForwardRendererBase(
			Graphics* graphics,
			Resources* resources,
			int width,
			int height
		);

        const vk::RenderPass& getColorPass() const;
        const vk::RenderPass& getDepthPrepass() const;

        vk::CommandBuffer getPrimaryCommandBuffer();

        vk::DescriptorSet getDescriptorSet();

    private:
        struct RenderPasses {
            vk::RenderPass m_ColorPass;
            vk::RenderPass m_DepthPrepass;
        } m_RenderPasses;

        struct Semaphores {
            vk::UniqueSemaphore m_DepthPrepassFinished;
            vk::UniqueSemaphore m_ColorPassFinished;
        };
	};
}
