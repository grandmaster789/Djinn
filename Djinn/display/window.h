#pragma once

#include "dependencies.h"
#include <string>
#include <memory>

namespace djinn {
    class Display;
}

namespace djinn::input {
    class Keyboard;
    class Mouse;
}

namespace djinn::display {
    class Monitor;

    class Window {
    public:
        friend class Display;

        using Keyboard = input::Keyboard;
        using Mouse    = input::Mouse;

        struct Frame {
            int m_Left;
            int m_Top;
            int m_Right;
            int m_Bottom;
        };

        // do not instantiate this directly, use the Context class (still public because it is stored in a unique_ptr)
        Window() = default;
        Window(GLFWwindow* handle, const std::string& title);
        ~Window();

        Window             (const Window&) = delete;
        Window& operator = (const Window&) = delete;
        Window             (Window&& w);
        Window& operator = (Window&& w);

        GLFWwindow* getHandle() const;

        bool shouldClose() const;

        void setShouldClose(bool value);
        void setTitle(const std::string& title);
        void setPosition(int x, int y); // relative to the virtual desktop
        void setSize(int width, int height);

        void iconify();
        void restore();
        void show();
        void hide();

        std::pair<int, int> getPosition() const; // (x, y) in screen coords
        std::pair<int, int> getSize()     const; // (width, height) client area size in screen coords
        Frame               getFrame()    const; // includes decorations, if any

        const Monitor*  getMonitor()  const;
              Keyboard* getKeyboard();
        const Keyboard* getKeyboard() const;
              Mouse*    getMouse();
        const Mouse*    getMouse()    const;

        // ----- Attribute queries -----
        bool isFocused()   const;
        bool isIconified() const;
        bool isVisible()   const;
        bool isResizable() const; // resizable by the user that is
        bool isDecorated() const;
        bool isFloating()  const; // also known as always-on-top

        // ----- Events ------
        struct OnMoved {
            Window* mWindow;
            int mOldPositionX;
            int mOldPositionY;
            int mPositionX;
            int mPositionY;
        };

        struct OnResized {
            Window* mWindow;
            int mOldWidth;
            int mOldHeight;
            int mWidth;
            int mHeight;
        };

        struct OnFramebufferResized {
            Window* mWindow;
            int mWidth;
            int mHeight;
        };

        struct OnCreated   { Window* mWindow; };
        struct OnRefreshed { Window* mWindow; };
        struct OnClosed    { Window* mWindow; };
        struct OnFocused   { Window* mWindow; };
        struct OnFocusLost { Window* mWindow; };
        struct OnIconify   { Window* mWindow; };
        struct OnRestore   { Window* mWindow; };

    private:
        void setGLFWcallbacks();

        void initVkSurface(vk::Instance instance, const vk::PhysicalDevice& gpu);

        GLFWwindow* m_Handle = nullptr;

        vk::UniqueSurfaceKHR              m_Surface;
        vk::SurfaceCapabilitiesKHR        m_SurfaceCaps;
        std::vector<vk::SurfaceFormatKHR> m_AvailableSurfaceFormats;
        vk::SurfaceFormatKHR              m_SurfaceFormat;

        std::string m_Title;

        std::unique_ptr<Keyboard> m_Keyboard;
        //std::unique_ptr<Mouse> m_Mouse;
    };
}