#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"

#include "monitor.h"

#include <memory>

namespace djinn {
    namespace renderer {
        class Window;
        class WindowHints;
    }

    class Renderer :
        public core::System,
        public MessageHandler<renderer::Monitor::OnConnected>,
        public MessageHandler<renderer::Monitor::OnDisconnected>
    {
    private:
        struct MonitorDeleter;

        using  Monitor     = renderer::Monitor;
        using  MonitorPtr  = std::unique_ptr<Monitor, MonitorDeleter>;
        using  MonitorList = std::vector<MonitorPtr>;

        struct MonitorDeleter { void operator()(Monitor* m); };

        using WindowHints = renderer::WindowHints;
        using Window      = renderer::Window;
        using WindowPtr   = std::unique_ptr<Window>;
        using WindowList  = std::vector<WindowPtr>;

    public:
        Renderer();

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

        // ----- Windows -----
              Window* getPrimaryWindow();
        const Window* getPrimaryWindow() const;

        const WindowList& getWindowList() const;

    private:
        Window* createWindow(const std::string& title, int width, int height);	// window on primary monitor
        Window* createWindow(const std::string& title, const Monitor* m);		// borderless fullscreen on the specified monitor
        Window* createWindow(const std::string& title, const Monitor* m, int width, int height); // fullscreen at the specified monitor with the specified resolution

        void createVkInstance(); // might throw vk::SystemError

        struct {
            int m_Width       = 800;
            int m_Height      = 600;
            bool m_Fullscreen = false;
            bool m_Borderless = false;
        } m_WindowSettings;

        bool m_UseValidation = false;

        MonitorList m_Monitors;
        WindowList m_Windows;

        vk::UniqueInstance m_VKInstance;
    };
}
