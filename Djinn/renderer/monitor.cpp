#include "monitor.h"
#include "util/flat_map.h"
#include <ostream>
#include <cassert>

namespace djinn::renderer {
    Monitor::Monitor(GLFWmonitor* handle):
        m_Handle(handle)
    {
        assert(handle);

        m_Name = glfwGetMonitorName(handle);

        glfwGetMonitorPhysicalSize(handle, &m_PhysicalWidth, &m_PhysicalHeight);
        glfwGetMonitorPos(handle, &m_PositionX, &m_PositionY);

        int numModes = 0;
        auto vidModes = glfwGetVideoModes(handle, &numModes);
        m_SupportedVideoModes.reserve(numModes);

        for (int i = 0; i < numModes; ++i)
            m_SupportedVideoModes.push_back(vidModes[i]);

        m_CurrentVideoMode = *glfwGetVideoMode(handle);
    }

    GLFWmonitor* Monitor::getHandle() const {
        return m_Handle;
    }

    bool Monitor::isPrimaryMonitor() const {
        return m_Handle == glfwGetPrimaryMonitor();
    }

    const std::string& Monitor::getName() const {
        return m_Name;
    }

    std::pair<int, int> Monitor::getPosition() const {
        return std::make_pair(m_PositionX, m_PositionY);
    }

    std::pair<int, int> Monitor::getPhysicalSize() const {
        return std::make_pair(m_PhysicalWidth, m_PhysicalHeight);
    }

    double Monitor::getDPI() const {
        return m_CurrentVideoMode.width * 25.4 / m_PhysicalWidth;
    }

    double Monitor::getAspectRatio() const {
        assert(m_PhysicalHeight != 0);

        return
            static_cast<double>(m_PhysicalWidth) /
            static_cast<double>(m_PhysicalHeight);
    }

    const Monitor::VideoMode& Monitor::getCurrentVideoMode() const {
        return m_CurrentVideoMode;
    }

    const std::vector<Monitor::VideoMode>& Monitor::getSupportedVideoModes() const {
        return m_SupportedVideoModes;
    }

    namespace detail {
        static util::FlatMap<GLFWmonitor*, Monitor*> g_MonitorMapping;

        void registerMonitor(Monitor* m) {
            assert(m != nullptr);
            assert(!g_MonitorMapping.contains(m->getHandle()));
            g_MonitorMapping.assign_or_insert(m->getHandle(), m);
        }

        void unregisterMonitor(Monitor* m) {
            assert(m != nullptr);
            assert(g_MonitorMapping.contains(m->getHandle()));
            g_MonitorMapping.erase(m->getHandle());
        }

        const Monitor* fetch(GLFWmonitor* handle) {
            if (auto result = g_MonitorMapping[handle])
                return *result;
            else
                return nullptr;
        }
    }

    std::ostream& operator << (std::ostream& os, const Monitor& m) {
        auto physical = m.getPhysicalSize();

        os
            << m.getName()
            << ", ("
            << physical.first
            << "x"
            << physical.second
            << "mm) "
            << m.getDPI()
            << "DPI "
            << m.getCurrentVideoMode();

        return os;
    }

    std::ostream& operator << (std::ostream& os, const Monitor::VideoMode& v) {
        os
            << "("
            << v.width
            << "x"
            << v.height
            << " @ "
            << v.refreshRate
            << "hz)";

        return os;
    }
}