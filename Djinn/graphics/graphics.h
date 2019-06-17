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
            bool m_Windowed
                = true;  // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        // vulkan-related items
        void initVulkan();

        vk::UniqueInstance                 m_Instance;
        vk::UniqueDebugReportCallbackEXT   m_DebugCallback;
        vk::PhysicalDevice                 m_PhysicalDevice;
        vk::PhysicalDeviceMemoryProperties m_MemoryProps;
        vk::UniqueDevice                   m_Device;

        static constexpr uint32_t NOT_FOUND = ~0UL;

        uint32_t m_GraphicsFamilyIdx = NOT_FOUND;

        vk::UniqueCommandPool   m_CommandPool;
        vk::UniqueCommandBuffer m_CommandBuffer;
    };
}  // namespace djinn
