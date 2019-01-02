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
    class Display :
        public core::System
    {
    public:
        using Window    = display::Window;
        using WindowPtr = std::unique_ptr<Window>;

        Display();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        void close(Window* w);

        vk::Instance       getVkInstance() const;
        vk::PhysicalDevice getVkPhysicalDevice() const;
        vk::Device         getVkDevice() const;

    private:
        Window* createWindow(
            int width         = 1280, 
            int height        = 720,
            bool windowed     = true, 
            int displaydevice = 0
        );

        void initVulkan();
        void createVulkanDevice();

        std::vector<WindowPtr> m_Windows;

        struct WindowSettings {
            int  m_Width         = 1280;
            int  m_Height        = 720;
            int  m_DisplayDevice = 0;
            bool m_Windowed      = true; // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        // vulkan-related
        vk::UniqueInstance                 m_VkInstance;
        vk::UniqueDebugReportCallbackEXT   m_VkDebugReportCallback;

        vk::PhysicalDevice                 m_VkPhysicalDevice;
        vk::PhysicalDeviceMemoryProperties m_VkPhysicalDeviceMemoryProperties;

        vk::UniqueDevice                   m_VkDevice;

        vk::Queue m_GraphicsQueue;
        vk::Queue m_TransferQueue;

        vk::UniqueShaderModule m_VertexShader;
        vk::UniqueShaderModule m_FragmentShader;
    };
}
