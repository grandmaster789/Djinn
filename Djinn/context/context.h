#pragma once

#include "third_party.h"
#include "core/system.h"
#include "core/mediator.h"

#include "window.h"

#include <memory>

/*
    Very marginal platform dependant stuff in here:
    - the vulkan win32 surface extension name is in here    
    [TODO] multi-GPU support... don't have the hardware for that tho
*/

namespace djinn {
    class Context :
        public core::System
    {
    public:
        using Window    = context::Window;
        using WindowPtr = std::unique_ptr<Window>;

        Context();

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
            int width         = 1280, 
            int height        = 720,
            bool windowed     = true, 
            int displaydevice = 0
        );

        void initVulkan();
        void selectPhysicalDevice();
        void createDevice();
        void createSurface();
        void selectSwapchainFormat();
        void createSwapchain();
        void createCommandPool();

        WindowPtr m_Window;

        struct WindowSettings {
            int  m_Width         = 1280;
            int  m_Height        = 720;
            int  m_DisplayDevice = 0;
            bool m_Windowed      = true; // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        // vulkan-related
        vk::UniqueInstance                 m_Instance;
        vk::UniqueDebugReportCallbackEXT   m_DebugReportCallback;

        vk::PhysicalDevice                 m_PhysicalDevice;
        vk::PhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;

        static constexpr uint32_t NOT_FOUND = ~0ul;

        uint32_t m_GraphicsFamilyIdx = NOT_FOUND;

        bool m_PushDescriptorsSupported = false;
        bool m_CheckpointsSupported     = false;
        bool m_MeshShading              = false;

        vk::UniqueDevice                   m_Device;

        // some of these are more of a per-window thing
        vk::UniqueSurfaceKHR m_Surface;

        vk::UniqueSwapchainKHR m_Swapchain;
        std::vector<vk::Image> m_SwapchainImages;

        vk::Format m_SwapchainFormat;
        vk::Format m_DepthFormat = vk::Format::eD32Sfloat;

        vk::Extent2D m_Extent;

        vk::UniqueSemaphore m_AcquireSemaphore;
        vk::UniqueSemaphore m_ReleaseSemaphore;

        vk::Queue               m_GraphicsQueue;
        vk::UniqueCommandBuffer m_GraphicsCommands;

        vk::UniqueCommandPool m_CommandPool;
    };
}
