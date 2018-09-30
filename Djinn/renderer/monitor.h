#pragma once

#include "dependencies.h"

#include <vector>
#include <string>
#include <iosfwd>

namespace djinn {
    class Renderer;
}

namespace djinn::renderer {
    class Monitor {
    public:
        friend class Renderer;

        using VideoMode = GLFWvidmode;

        Monitor(GLFWmonitor* handle);

        // move-only
        Monitor             (const Monitor&) = delete;
        Monitor& operator = (const Monitor&) = delete;
        Monitor             (Monitor&&) = default;
        Monitor& operator = (Monitor&&) = default;

        GLFWmonitor* getHandle() const;

        bool isPrimaryMonitor() const;
        const std::string& getName() const;

        std::pair<int, int> getPosition() const;     // (x, y) relative to the virtual desktop reference frame [NOTE] introduce a struct for this?
        std::pair<int, int> getPhysicalSize() const; // (horizontal, vertical) in millimeters, as reported by the OS
        double getDPI() const;
        double getAspectRatio() const;

        const VideoMode& getCurrentVideoMode() const;
        const std::vector<VideoMode>& getSupportedVideoModes() const;

        // ---------- Signals --------------

        struct OnConnected    { GLFWmonitor* m_Handle; };
        struct OnDisconnected { GLFWmonitor* m_Handle; };

    private:
        GLFWmonitor* m_Handle;

        std::string m_Name;

        int m_PositionX;
        int m_PositionY;
        int m_PhysicalWidth;
        int m_PhysicalHeight;

        VideoMode m_CurrentVideoMode;
        std::vector<VideoMode> m_SupportedVideoModes;
    };

    namespace detail {
        void registerMonitor(Monitor* m);
        void unregisterMonitor(Monitor* m);

        const Monitor* fetch(GLFWmonitor* handle);
    }

    std::ostream& operator << (std::ostream& os, const Monitor& m);
    std::ostream& operator << (std::ostream& os, const Monitor::VideoMode& v);
}
