#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"
#include "monitor.h"

#include <memory>

namespace djinn {
    class Renderer :
        public core::System,
        public MessageHandler<renderer::Monitor::OnConnected>,
        public MessageHandler<renderer::Monitor::OnDisconnected>
    {
    private:
        struct MonitorDeleter;

    public:
        using Monitor     = renderer::Monitor;
        using MonitorPtr  = std::unique_ptr<Monitor, MonitorDeleter>;
        using MonitorList = std::vector<MonitorPtr>;

        //using WindowHints = renderer::WindowHints;
        //using Window      = renderer::Window;
        //using WindowPtr   = std::unique_ptr<Window>;
        //using WindowList  = std::vector<WindowPtr>;

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

    private:
        struct {
            int m_Width       = 800;
            int m_Height      = 600;
            bool m_Fullscreen = false;
            bool m_Borderless = false;
        } m_WindowSettings;

        struct MonitorDeleter { void operator()(Monitor* m); };

        MonitorList m_Monitors;
    };
}
