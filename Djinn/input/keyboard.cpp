#include "keyboard.h"
#include "display/window.h"
#include "core/mediator.h"
#include "core/logger.h"

namespace {
    /*
    void unicodeCallback(GLFWwindow* handle, unsigned int codepoint) {
    }

    void unicodeModsCallback(GLFWwindow* handle, unsigned int codepoint, int modifiers) {
    }
    */

    void keyCallback(
        GLFWwindow* handle,
        int key,
        int, // scancode
        int action,
        int modifiers
    ) {
        using djinn::display::Window;
        using djinn::input::Keyboard;
        using djinn::broadcast;

        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        Keyboard* kbd = w->getKeyboard();

        switch (action) {
        case GLFW_PRESS:
            kbd->m_KeyState[key] = true;
            broadcast(Keyboard::OnKeyPress{ kbd, key, modifiers });
            break;

        case GLFW_RELEASE:
            kbd->m_KeyState[key] = false;
            broadcast(Keyboard::OnKeyRelease{ kbd, key, modifiers });
            break;

        /*
        default: 
            // [NOTE] what do we want with key repeats?
            gLogDebug << "Unhandled keyboard event: " << action;
            break;
            */
        }
    }
}

namespace djinn::input {
    Keyboard::Keyboard(Window* w) :
        m_SourceWindow(w)
    {
        if (w) {
            using namespace std;

            fill(
                begin(m_KeyState), 
                end(m_KeyState), 
                false
            );

            /*
                glfwSetCharCallback(sourceWindow->getHandle(), unicodeCallback);
                glfwSetCharModsCallback(sourceWindow->getHandle(), unicodeModsCallback);
            */
            glfwSetKeyCallback(w->getHandle(), keyCallback);
        }
    }

    void Keyboard::setStickyKeys(bool enabled) {
        glfwSetInputMode(
            m_SourceWindow->getHandle(), 
            GLFW_STICKY_KEYS, 
            enabled ? GLFW_TRUE : GLFW_FALSE
        );
    }

    bool Keyboard::getStickyKeys() const {
        return (glfwGetInputMode(
            m_SourceWindow->getHandle(),
            GLFW_STICKY_KEYS
        ) != GLFW_FALSE);
    }

    Keyboard::Window* Keyboard::getSourceWindow() const {
        return m_SourceWindow;
    }

    bool Keyboard::operator[](int button) const {
        return m_KeyState[button];
    }
}