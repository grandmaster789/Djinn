#pragma once

#include "third_party.h"
#include "core/system.h"
#include "core/mediator.h"

#include "window.h"
#include "swapchain.h"

#include <memory>

/*
    Very marginal platform dependant stuff in here:
    - the vulkan win32 surface extension name is in here
    - win32 surface creation
    - win32 presentation support
    [TODO] multi-GPU support... don't have the hardware for that tho
*/

namespace djinn {
    class Graphics :
        public core::System
    {
    public:        
        using Window    = graphics::Window;
        using Swapchain = graphics::Swapchain;

        // [TODO] currently using pointer wrapping here to control lifetime -- should be replaced with UniqueResource or something
        using WindowPtr    = std::unique_ptr<Window>;        
        using SwapchainPtr = std::unique_ptr<Swapchain>; 

		Graphics();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        void close(Window* w);

        vk::Instance       getInstance()       const;
        vk::PhysicalDevice getPhysicalDevice() const;
        vk::Device         getDevice()         const;

    private:
        Window* createWindow(
            int  width         = 1280, 
            int  height        = 720,
            bool windowed      = true, 
            int  displaydevice = 0
        );

        void initVulkan();
        void selectPhysicalDevice();
        void createDevice();
        void createSurface();
        void createCommandPool();

        vk::UniqueRenderPass createSimpleRenderpass(
            vk::Format imageFormat, 
            vk::Format depthFormat
        ) const;

        // load a shader binary; which would typically have been compiled with GLSLvalidator to SPIRV
        vk::UniqueShaderModule loadShader(const std::filesystem::path& p) const;

        // simple vertex+fragment shader, both with an entry point called 'main'
        // accepting triangle lists, pretty much all 'default' settings (there are a lot though)
        vk::UniquePipeline createSimpleGraphicsPipeline(
            vk::ShaderModule   vertexShader,
            vk::ShaderModule   fragmentShader,
            vk::PipelineLayout layout
        ) const;

        WindowPtr m_Window;

        struct WindowSettings {
            int  m_Width         = 1280;
            int  m_Height        = 720;
            int  m_DisplayDevice = 0;
            bool m_Windowed      = true; // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        // [NOTE] the order is pretty specific, for proper destruction ordering
        static constexpr uint32_t NOT_FOUND = ~0ul;

        uint32_t m_GraphicsFamilyIdx = NOT_FOUND;

        vk::Format m_SwapchainFormat = vk::Format::eB8G8R8A8Unorm;
        vk::Format m_DepthFormat     = vk::Format::eD32Sfloat;

        vk::UniqueInstance                 m_Instance;
        vk::UniqueDebugReportCallbackEXT   m_DebugReportCallback;
        vk::UniqueDevice                   m_Device;
        vk::UniqueSurfaceKHR               m_Surface;

        vk::PhysicalDevice                 m_PhysicalDevice;
        vk::PhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;

        SwapchainPtr            m_Swapchain;
        
        vk::UniqueRenderPass    m_Renderpass;
        vk::UniqueCommandPool   m_CommandPool;

        vk::Queue               m_GraphicsQueue;
        vk::UniqueCommandBuffer m_GraphicsCommands;
        vk::UniquePipelineCache m_PipelineCache;

        vk::UniqueShaderModule   m_TriangleVertexShader;
        vk::UniqueShaderModule   m_TriangleFragmentShader;
        vk::UniquePipelineLayout m_TrianglePipelineLayout;
        vk::UniquePipeline       m_TrianglePipeline;

        // temporary buffering storage
        vk::UniqueBuffer       m_Cube;
        vk::UniqueDeviceMemory m_CubeBuffer;
    };
}
