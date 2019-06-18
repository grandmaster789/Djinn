#pragma once

#include "core/mediator.h"
#include "core/system.h"
#include "third_party.h"

#include "window.h"

#include <memory>
#include <vector>

/*
    Very marginal platform dependant stuff in here:
    - the vulkan win32 surface extension name is in here
    - win32 surface creation
    - win32 presentation support
    [TODO] multi-GPU support... don't have the hardware for that tho
	[TODO] multi-window support... the render pipeline is just one window for now
*/

namespace djinn {
    class Graphics: public core::System {
    public:
        using Window = graphics::Window;

        using WindowPtr = std::unique_ptr<Window>;

        Graphics();

        void init() override;
        void update() override;
        void shutdown() override;

        Window*       getMainWindow();
        const Window* getMainWindow() const;

        void close(Window* w);

        vk::Instance       getInstance() const;
        vk::PhysicalDevice getPhysicalDevice() const;
        vk::Device         getDevice() const;
        vk::CommandPool    getCommandPool() const;
        vk::CommandBuffer  getCommandBuffer() const;

        uint32_t             getGraphicsFamilyIdx() const;
        uint32_t             getPresentFamilyIdx() const;
        vk::SurfaceFormatKHR getSurfaceFormat() const;
        vk::Format           getDepthFormat() const;

    private:
        // WSI integration
        Window* createWindow(
            int  width         = 1280,
            int  height        = 720,
            bool windowed      = true,
            int  displaydevice = 0);

        std::vector<WindowPtr> m_Windows;  // first window is the main one

        struct WindowSettings {
            int  m_Width         = 1280;
            int  m_Height        = 720;
            int  m_DisplayDevice = 0;
            bool m_Windowed      = true;  // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        // vulkan-related items, mostly based on the official API samples
        void initVulkan();
        void initLogicalDevice();  // [NOTE] depends on the existance of a window surface
        void initUniformBuffer();
        void initPipelineLayouts();
        void initRenderPass();
        void initShaders(const std::string& vtxSrc, const std::string& fragSrc);

        vk::UniqueInstance                 m_Instance;
        vk::UniqueDebugReportCallbackEXT   m_DebugCallback;
        vk::PhysicalDevice                 m_PhysicalDevice;
        vk::PhysicalDeviceMemoryProperties m_MemoryProps;
        vk::UniqueDevice                   m_Device;

        // [NOTE] much of the stuff here is really a per-window thingie,
        //        at some point I should restructure it to reflect that
        static constexpr uint32_t NOT_FOUND = ~0UL;

        uint32_t             m_GraphicsFamilyIdx = NOT_FOUND;
        uint32_t             m_PresentFamilyIdx  = NOT_FOUND;
        vk::SurfaceFormatKHR m_SurfaceFormat;
        vk::Format           m_DepthFormat = vk::Format::eD24UnormS8Uint;

        vk::UniqueCommandPool   m_CommandPool;
        vk::UniqueCommandBuffer m_CommandBuffer;

        vk::Queue m_GraphicsQueue;
        vk::Queue m_PresentQueue;

        glm::mat4 m_View;
        glm::mat4 m_Model;
        glm::mat4 m_Projection;
        glm::mat4 m_Clip;
        glm::mat4 m_MVP;

        vk::UniqueBuffer       m_UniformBuffer;
        vk::UniqueDeviceMemory m_UniformMemory;  // [NOTE] store capacity somewhere?

        vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
        vk::UniquePipelineLayout      m_PipelineLayout;
        vk::UniqueRenderPass          m_RenderPass;
    };

    namespace graphics {
        uint32_t selectMemoryTypeIndex(
            vk::PhysicalDevice      gpu,
            uint32_t                typeBits,
            vk::MemoryPropertyFlags properties);
    }
}  // namespace djinn
