#include "window_hints.h"
#include "dependencies.h"

#include <ostream>
#include <string>

namespace {
    std::string maybeDontCare(int value) {
        if (value == GLFW_DONT_CARE)
            return "Don't care";
        else
            return std::to_string(value);
    }

    bool checkHintNumber(int value) {
        static_assert(GLFW_DONT_CARE == -1);
        return (
            (value >= -1) && // <- this equals GLFW_DONT_CARE
            (value <= INT_MAX)
        );
    }
}
namespace djinn::renderer {
    void WindowHints::apply() {
        // [NOTE] I don't believe it's possible to verify the version numbers beforehand. 
            //        Not entirely sure though
        glfwWindowHint(GLFW_RESIZABLE,    m_Resizable   ? 1 : 0);
        glfwWindowHint(GLFW_VISIBLE,      m_Visible     ? 1 : 0);
        glfwWindowHint(GLFW_DECORATED,    m_Decorated   ? 1 : 0);
        glfwWindowHint(GLFW_FOCUSED,      m_Focused     ? 1 : 0);
        glfwWindowHint(GLFW_AUTO_ICONIFY, m_AutoIconify ? 1 : 0);
        glfwWindowHint(GLFW_FLOATING,     m_Floating    ? 1 : 0);

        glfwWindowHint(GLFW_REFRESH_RATE, m_RefreshRate);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // vulkan mode always has this
    }

    std::ostream& operator << (std::ostream& os, const WindowHints& hints) {
        os << std::boolalpha
            << "\tWindow hints:"
            << "\n\tResizable: "   << hints.m_Resizable
            << "\n\tVisible: "     << hints.m_Visible
            << "\n\tDecorated: "   << hints.m_Decorated
            << "\n\tFocused: "     << hints.m_Focused
            << "\n\tAutoIconify: " << hints.m_AutoIconify
            << "\n\tFloating: "    << hints.m_Floating
            << "\n\tRefreshrate: " << hints.m_RefreshRate;

        return os;
    }
}