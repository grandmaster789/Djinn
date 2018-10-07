#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"

#include "monitor.h"

#include <memory>

namespace djinn {
    namespace display {
        class Window;
        class WindowHints;
    }

    class Display :
        public core::System,
        public MessageHandler<display::Monitor::OnConnected>,
        public MessageHandler<display::Monitor::OnDisconnected>
    {
    private:
        struct MonitorDeleter;

        using  Monitor     = display::Monitor;
        using  MonitorPtr  = std::unique_ptr<Monitor, MonitorDeleter>;
        using  MonitorList = std::vector<MonitorPtr>;

        struct MonitorDeleter { void operator()(Monitor* m); };

        using WindowHints = display::WindowHints;
        using Window      = display::Window;
        using WindowPtr   = std::unique_ptr<Window>;
        using WindowList  = std::vector<WindowPtr>;

    public:
        Display();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        // ----- Monitors -----
        void               detectMonitors();
        Monitor*           getPrimaryMonitor() const;
        const MonitorList& getMonitorList() const;
        size_t             getNumMonitors() const;

        void operator()(const Monitor::OnConnected& o);
        void operator()(const Monitor::OnDisconnected& o);

        // ----- Window -----
              Window* getWindow();
        const Window* getWindow() const;

    private:
        void createWindow(const std::string& title, int width, int height);	// window on primary monitor
        void createWindow(const std::string& title, const Monitor* m);		// borderless fullscreen on the specified monitor
        void createWindow(const std::string& title, const Monitor* m, int width, int height); // fullscreen at the specified monitor with the specified resolution

        void createVkInstance(); // might throw vk::SystemError
        void setupVkDebugCallback();
        void selectVkPhysicalDevice();
        void createVkSurface();
        void createVkQueueIndices();
        void createVkDevice();
        
        struct {
            int m_Width = 800;
            int m_Height = 600;
            bool m_Fullscreen = false;
            bool m_Borderless = false;
        } m_WindowSettings;

        bool m_UseValidation = false;

        MonitorList m_Monitors;
        WindowPtr m_Window;

        vk::UniqueInstance               m_VkInstance;
        vk::PhysicalDevice               m_PhysicalDevice;
        vk::UniqueDevice                 m_Device;        
        vk::UniqueDebugReportCallbackEXT m_VkDebugCallback;

        vk::SampleCountFlagBits m_MaxSampleCount;

    };
}
