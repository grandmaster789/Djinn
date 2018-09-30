#include "renderer.h"
#include "core/engine.h"
#include "core/mediator.h"
#include "window.h"
#include "window_hints.h"

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
        registerSetting("width",      &m_WindowSettings.m_Width);
        registerSetting("height",     &m_WindowSettings.m_Height);
        registerSetting("fullscreen", &m_WindowSettings.m_Fullscreen);
        registerSetting("borderless", &m_WindowSettings.m_Borderless);

        registerSetting("vk_validation", &m_UseValidation);
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

        gLogDebug
            << "Creating main window: "
            << m_WindowSettings.m_Width
            << "x"
            << m_WindowSettings.m_Height
            << " fullscreen: "
            << m_WindowSettings.m_Fullscreen
            << " borderless: "
            << m_WindowSettings.m_Borderless;

        renderer::WindowHints hints;

        hints.m_Resizable   = false; // TODO - resizing windows has some implications about recreating swapbuffer chains etc
        hints.m_Visible     = true;
        hints.m_Decorated   = true;
        hints.m_Focused     = true;
        hints.m_AutoIconify = true;
        hints.m_Floating    = true; // always-on-top enabled

        hints.apply();

        // different ways to create a window:
        // 1) fullscreen            (implies switching video mode, requires target monitor)
        // 2) borderless fullscreen (ignores width/height, requires target monitor)
        // 3) borderless windowed 
        // 4) bordered windowed
        if (m_WindowSettings.m_Fullscreen) {
            if (m_WindowSettings.m_Borderless)
                createWindow("Djinn", getPrimaryMonitor());
            else
                createWindow(
                    "Djinn",
                    getPrimaryMonitor(),
                    m_WindowSettings.m_Width,
                    m_WindowSettings.m_Height
                );
        }
        else {
            if (m_WindowSettings.m_Borderless) {
                hints.m_Decorated = false;
                hints.apply();
            }

            createWindow(
                "Djinn", 
                m_WindowSettings.m_Width,
                m_WindowSettings.m_Height
            );
        }
    }

    void Renderer::update() {
        if (m_Windows.empty())
            m_Engine->stop();

        auto it = m_Windows.begin();

        for (; it != m_Windows.end();) {
            if ((*it)->shouldClose())
                it = m_Windows.erase(it);
            else {
                // ~~ present surface here
                ++it;
            }
        }

        glfwPollEvents();
    }

    void Renderer::shutdown() {
        System::shutdown();

        m_Windows.clear();
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

    Renderer::Window* Renderer::getPrimaryWindow() {
        return m_Windows.front().get();
    }

    const Renderer::Window* Renderer::getPrimaryWindow() const {
        return m_Windows.front().get();
    }

    const Renderer::WindowList& Renderer::getWindowList() const {
        return m_Windows;
    }

    Renderer::Window* Renderer::createWindow(
        const std::string& title,
        int width,
        int height
    ) {
        // windowed mode (both bordered and borderless)
        return m_Windows.emplace_back(
            std::make_unique<Window>(
                glfwCreateWindow(
                    width, 
                    height, 
                    title.c_str(), 
                    nullptr, 
                    nullptr
                ),
                title
            )
        ).get();
    }

    Renderer::Window* Renderer::createWindow(
        const std::string& title,
        const Monitor* m
    ) {
        auto mode = m->getCurrentVideoMode();

        // fullscreen borderless
        return m_Windows.emplace_back(
            std::make_unique<Window>(
                glfwCreateWindow(
                    mode.width,
                    mode.height,
                    title.c_str(),
                    m->getHandle(),
                    nullptr
                ),
                title
            )
        ).get();
    }

    Renderer::Window* Renderer::createWindow(
        const std::string& title,
        const Monitor* m,
        int width,
        int height
    ) {
        // fullscreen
        return m_Windows.emplace_back(
            std::make_unique<Window>(
                glfwCreateWindow(
                    width,
                    height,
                    title.c_str(),
                    m->getHandle(),
                    nullptr
                ),
                title
            )
        ).get();
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