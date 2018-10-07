#pragma once

#include "dependencies.h"
#include <array>

namespace djinn::display {
    class Window;
}

namespace djinn::input {
    /*
        [NOTE] this is unwieldy for text input, but should be okay for hotkeys and the like
    */
    class Keyboard {
    public:
        using Window = display::Window;

        Keyboard(Window* w);

        Keyboard             (const Keyboard&) = delete;
        Keyboard& operator = (const Keyboard&) = delete;
        Keyboard             (Keyboard&&) = default;
        Keyboard& operator = (Keyboard&&) = default;

        void setStickyKeys(bool enabled);
        bool getStickyKeys() const;

        Window* getSourceWindow() const;
        
        // For an overview of keycodes, see http://www.glfw.org/docs/latest/group__keys.html
        // An overview of modifiers can be found at http://www.glfw.org/docs/latest/group__mods.html
        bool operator[](int button) const; // ex : kbd[GLFW_BUTTON_ESCAPE], kbd['W']

        std::array<bool, GLFW_KEY_LAST> m_KeyState; // ~~ somewhat unfortunate that this is public

        struct OnKeyPress {
            Keyboard* m_Keyboard;
            int m_Key;
            int m_Modifiers;
        };

        struct OnKeyRelease {
            Keyboard* m_Keyboard;
            int m_Key;
            int m_Modifiers;
        };

    private:
        Window* m_SourceWindow;
    };
}
