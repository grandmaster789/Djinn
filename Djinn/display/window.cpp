#include "window.h"

namespace {
    LRESULT CALLBACK deferWinProc(
        HWND   window,
        UINT   msg,
        WPARAM wp,
        LPARAM lp
    ) {
        LONG_PTR ptr = GetWindowLongPtr(window, 0);

        if (ptr == NULL)
            return DefWindowProc(window, msg, wp, lp);
        else
            return ((djinn::display::Window*)ptr)->winProc(window, msg, wp, lp);
    }

    struct WndClass {
        static WndClass& instance() {
            static WndClass result;
            return result;
        }

        const WNDCLASSEX& get() {
            return m_WindowClass;
        }

    private:
        WndClass() {
            m_WindowClass = {};

            m_WindowClass.cbSize        = sizeof(m_WindowClass);
            m_WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
            m_WindowClass.lpfnWndProc   = deferWinProc;
            m_WindowClass.cbClsExtra    = 0;
            m_WindowClass.cbWndExtra    = sizeof(void*); // we're going to associate a single pointer to the window object
            m_WindowClass.hInstance     = NULL; // not sure what's the difference with GetModuleHandle(NULL); could also propagate from program entry point
            m_WindowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
            m_WindowClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
            m_WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
            m_WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            m_WindowClass.lpszMenuName  = nullptr;
            m_WindowClass.lpszClassName = "DjinnWindowClass";

            RegisterClassEx(&m_WindowClass);
        }

        ~WndClass() {
            UnregisterClass(m_WindowClass.lpszClassName, NULL);
        }

        WNDCLASSEX m_WindowClass;
    };
}

namespace djinn::display {
    Window::Window(int width, int height) 
    {
        const auto& wc = WndClass::instance().get();
        
        // [NOTE] this is for the primary monitor
        auto screen_width  = GetSystemMetrics(SM_CXSCREEN);
        auto screen_height = GetSystemMetrics(SM_CYSCREEN);

        m_Handle = CreateWindowEx(
            0,                                // exStyle
            wc.lpszClassName,
            "Djinn",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // style flags
            (screen_width - width) / 2,       // left
            (screen_height - height) / 2,     // top
            width,
            height,
            nullptr,                          // parent window
            nullptr,                          // menu
            0,                                // hInstance
            nullptr                           // additional parameters
        );

        if (m_Handle) {
            SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);
        }
        else
            throw std::runtime_error("Failed to create window");
    }

    Window::~Window() {
        if (m_Handle) {
            DestroyWindow(m_Handle);
        }
    }

    Window::Window(Window&& w):
        m_Handle(w.m_Handle)
    {
        SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);
        w.m_Handle = nullptr;
    }

    Window& Window::operator = (Window&& w) {
        if (&w == this)
            return *this;

        if (m_Handle)
            DestroyWindow(m_Handle);

        m_Handle = w.m_Handle;
        w.m_Handle = nullptr;

        if (m_Handle)
            SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);

        return *this;
    }

    HWND Window::getHandle() const {
        return m_Handle;
    }

    LRESULT Window::winProc(
        HWND handle,
        UINT message,
        WPARAM wp,
        LPARAM lp
    ) {
        switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(handle, message, wp, lp);
    }
}
