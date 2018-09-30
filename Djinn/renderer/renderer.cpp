#include "renderer.h"
#include "core/engine.h"
#include "core/mediator.h"

namespace {
    void glfw_monitor_callback(GLFWmonitor* handle, int evt) {
        using djinn::broadcast;
        using djinn::renderer::Monitor;
        using djinn::renderer::detail::fetch;

        switch (evt) {
        case GLFW_CONNECTED:    
            broadcast(Monitor::OnConnected{ handle });
            break;

        case GLFW_DISCONNECTED:
            broadcast(Monitor::OnDisconnected{ handle });
            break;

        default:
            gLogWarning << "Unknown monitor event: " << evt;
        }
    }
}

namespace djinn {
    Renderer::Renderer() :
        System("Renderer")
    {
        registerSetting("width", &m_WindowSettings.m_Width);
        registerSetting("height", &m_WindowSettings.m_Height);
    }

    void Renderer::init() {
        System::init();

        if (glfwVulkanSupported() == GLFW_FALSE)
            throw std::runtime_error("Vulkan is not supported");

        detectMonitors();
        glfwSetMonitorCallback(&glfw_monitor_callback);

        gLogDebug
            << "Detected "
            << getNumMonitors()
            << " monitor(s)";

        gLogDebug
            << "Primary monitor: "
            << *m_Monitors.front();

        {
            uint32_t count = 0;
            auto ext = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char*> requiredExtensions;
            std::vector<const char*> requiredLayers;

            for (uint32_t i = 0; i < count; ++i)
                requiredExtensions.push_back(ext[i]);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    }

    void Renderer::update() {

    }

    void Renderer::shutdown() {
        System::shutdown();
    }

    void Renderer::unittest() {
    }

    void Renderer::operator()(const Monitor::OnConnected& o) {
        // cant' use make_unique because of the custom deleter... -_-
        MonitorPtr ptr(new renderer::Monitor(o.m_Handle), {});

        gLogDebug << "Monitor connected: " << ptr->getName();
        renderer::detail::registerMonitor(ptr.get());

        m_Monitors.push_back(std::move(ptr));
    }

    void Renderer::operator()(const Monitor::OnDisconnected& o) {
        auto monitor = renderer::detail::fetch(o.m_Handle);
        assert(monitor != nullptr);

        gLogDebug << "Monitor disconnected: " << monitor->getName();

        auto it = util::find_if(m_Monitors, [&](const MonitorPtr& ptr) {
            return ptr->getHandle() == o.m_Handle;
        });

        m_Monitors.erase(it);
    }

    void Renderer::detectMonitors() {
        m_Monitors.clear();
        int count = 0;

        // [NOTE] documentation says that the first monitor returned
        //        by glfwGetMonitors is *always* the primary monitor
        auto monitors = glfwGetMonitors(&count);
        for (int i = 0; i < count; ++i) {
            MonitorPtr ptr(new Monitor(monitors[i]), {});

            renderer::detail::registerMonitor(ptr.get());
            m_Monitors.push_back(std::move(ptr));
        }
    }

    Renderer::Monitor* Renderer::getPrimaryMonitor() const {
        return m_Monitors.front().get();
    }

    const Renderer::MonitorList& Renderer::getMonitorList() const {
        return m_Monitors;
    }

    size_t Renderer::getNumMonitors() const {
        return m_Monitors.size();
    }

    void Renderer::MonitorDeleter::operator()(Monitor* m) {
        renderer::detail::unregisterMonitor(m);
        delete m;
    }
}