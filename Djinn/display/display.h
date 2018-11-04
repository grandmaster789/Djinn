#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"

#include "window.h"

#include <memory>

namespace djinn {
    class Display :
        public core::System
    {
    public:
        using Window = display::Window;

        Display();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        vk::Instance getVkInstance() const;

    private:
        void createWindow(int width, int height);
        void initVulkan();

        std::vector<Window> m_Windows;

        struct WindowSettings {
            int m_Width          = 800;
            int m_Height         = 600;
            bool m_Fullscreen    = false;
            // monitor? borderless?
        } m_MainWindowSettings;

        // vulkan-related
        vk::UniqueInstance               m_VkInstance;
        vk::UniqueDebugReportCallbackEXT m_VkDebugReportCallback;
    };
}
