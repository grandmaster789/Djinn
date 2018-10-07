#pragma once

#include <iosfwd>

// partial wrapping of the GLFW windowing API
// discarded everything that is specifically geared towards openGL
// http://www.glfw.org/docs/latest/window_guide.html#window_hints_values

namespace djinn {
    class Display;
}

namespace djinn::display {
    class Monitor;

    class WindowHints {
    private:
        friend class Display;

        WindowHints() = default;

    public:        
        void apply();

        // these are the defaults as provided by GLFW
        bool m_Resizable   = true;
        bool m_Visible     = true; // ignored for fullscreen windows
        bool m_Decorated   = true; // ignored for fullscreen windows
        bool m_Focused     = true; // ignored for fullscreen and initially hidden windows
        bool m_AutoIconify = true;
        bool m_Floating    = false; // ignored for fullscreen windows; aka always-on-top

        int m_RefreshRate = 0; // ignored for windowed mode
    }; 

    std::ostream& operator << (std::ostream& os, const WindowHints& hints);
}
