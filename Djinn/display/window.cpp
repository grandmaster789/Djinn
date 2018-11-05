#include "window.h"
#include "display.h"

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
            m_WindowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
            m_WindowClass.lpfnWndProc   = deferWinProc;
            m_WindowClass.cbClsExtra    = 0;
            m_WindowClass.cbWndExtra    = sizeof(void*); // we're going to associate a single pointer to the window object
            m_WindowClass.hInstance     = GetModuleHandle(NULL); // could also propagate from program entry point
            m_WindowClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
            m_WindowClass.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);
            m_WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
            m_WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            m_WindowClass.lpszMenuName  = nullptr;
            m_WindowClass.lpszClassName = "DjinnWindowClass";

            if (RegisterClassEx(&m_WindowClass) == 0)
                throw std::runtime_error("Failed to register Djinn WindowClass");
        }

        ~WndClass() {
            UnregisterClass(m_WindowClass.lpszClassName, GetModuleHandle(NULL));
        }

        WNDCLASSEX m_WindowClass;
    };
}

namespace djinn::display {
    Window::Window(
        int      width, 
        int      height, 
        bool     windowed,
        int      displayDevice,
        Display* owner
    ):
        m_Owner(owner)
    {
        auto devices = enumerateDisplayDevices();

        if (devices.empty())
            throw std::runtime_error("No display devices are available");

        if (devices.size() < displayDevice) {
            gLogWarning << "Display device number " << displayDevice << " is unavailable, falling back to primary display";
            displayDevice = 0;
        }

        auto deviceMode = getCurrentDisplayMode(devices[displayDevice]);

        RECT rect;
        DWORD style = 0; // https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-styles
        DWORD exStyle = 0; // https://docs.microsoft.com/en-us/windows/desktop/winmsg/extended-window-styles

        if (windowed) {
            // center the rect
            int device_x      = deviceMode.dmPosition.x;
            int device_y      = deviceMode.dmPosition.y;
            int device_width  = deviceMode.dmPelsWidth;
            int device_height = deviceMode.dmPelsHeight;

            rect.left   = device_x  + (device_width  - width)  / 2;
            rect.top    = device_y  + (device_height - height) / 2;
            rect.right  = rect.left + width;
            rect.bottom = rect.top  + height;

            style = 
                WS_OVERLAPPEDWINDOW | 
                WS_CLIPCHILDREN     | 
                WS_CLIPSIBLINGS;

            exStyle =
                WS_EX_APPWINDOW |
                WS_EX_WINDOWEDGE;
        }
        else {
            // fill up the entire device space
            int device_x      = deviceMode.dmPosition.x;
            int device_y      = deviceMode.dmPosition.y;
            int device_width  = deviceMode.dmPelsWidth;
            int device_height = deviceMode.dmPelsHeight;

            rect.left   = device_x;
            rect.top    = device_y;
            rect.right  = rect.left + device_width;
            rect.bottom = rect.top  + device_height;

            style =
                WS_POPUP        | 
                WS_CLIPSIBLINGS | 
                WS_CLIPCHILDREN;

            exStyle = WS_EX_APPWINDOW;
        }

        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        const auto& wc = WndClass::instance().get();
        
        // [NOTE] this is for the primary monitor
        int left = CW_USEDEFAULT;
        int top = CW_USEDEFAULT;

        if (isMainWindow()) {
            int screen_width = GetSystemMetrics(SM_CXSCREEN);
            int screen_height = GetSystemMetrics(SM_CYSCREEN);

            left = (screen_width - width) / 2;
            top  = (screen_height - height) / 2;
        }

        m_Handle = CreateWindowEx(
            exStyle, 
            wc.lpszClassName,
            "Djinn",                 // title
            style,
            rect.left,
            rect.top,
            rect.right  - rect.left, // width
            rect.bottom - rect.top,  // height
            nullptr,                 // parent window
            nullptr,                 // menu
            GetModuleHandle(NULL),   // hInstance
            nullptr                  // additional parameters
        );

        if (m_Handle) {
            SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);

            if (!s_MainWindow)
                s_MainWindow = m_Handle;
     
            GetClientRect(m_Handle, &rect);
            m_Width  = rect.right  - rect.left;
            m_Height = rect.bottom - rect.top;

            ShowWindow(m_Handle, SW_SHOW);
            SetForegroundWindow(m_Handle);

            createSurface();
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
        m_Handle (w.m_Handle),
        m_Surface(std::move(w.m_Surface)),
        m_Owner  (w.m_Owner)
    {
        SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);
        w.m_Handle = nullptr;
        w.m_Owner  = nullptr;
    }

    Window& Window::operator = (Window&& w) {
        if (&w == this)
            return *this;

        if (m_Handle)
            DestroyWindow(m_Handle);

        m_Handle   = w.m_Handle;
        m_Surface  = std::move(w.m_Surface);
        m_Owner    = w.m_Owner;
        w.m_Handle = nullptr;
        w.m_Owner  = nullptr;

        if (m_Handle)
            SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);

        return *this;
    }

    HWND Window::getHandle() const {
        return m_Handle;
    }

    vk::SurfaceKHR Window::getSurface() const {
        return *m_Surface;
    }

    LRESULT Window::winProc(
        HWND handle,
        UINT message,
        WPARAM wp,
        LPARAM lp
    ) {
        switch (message) {
        case WM_DESTROY: {
            if (isMainWindow())
                PostQuitMessage(0);
            break;
        }

        case WM_CLOSE: {
            m_Owner->close(this);
            break;
        }

        default:
            return DefWindowProc(handle, message, wp, lp);
        }

        return 0;
    }

    bool Window::isMainWindow() const {
        return m_Handle == s_MainWindow;
    }

    std::vector<DISPLAY_DEVICE> Window::enumerateDisplayDevices() {
        std::vector<DISPLAY_DEVICE> result;

        for (int i = 0;; i++) {
            DISPLAY_DEVICE dd = {};
            dd.cb = sizeof(dd);

            // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-enumdisplaydevicesa
            if (!EnumDisplayDevices(NULL, i, &dd, 0))
                break; // stop if we couldn't find more devices

            if (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
                continue; // skip over mirroring (pseudo) devices

            result.push_back(dd);
        }

        return result;
    }

    DEVMODE Window::getCurrentDisplayMode(DISPLAY_DEVICE dd) {
        DEVMODE mode = {};

        mode.dmSize        = sizeof(mode);
        mode.dmDriverExtra = 0;

        if (!EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &mode))
            throw std::runtime_error("Failed to query current display mode");

        return mode;
    }

    void Window::createSurface() {
        vk::Win32SurfaceCreateInfoKHR info = {};

        info
            .setHinstance(GetModuleHandle(NULL))
            .setHwnd     (m_Handle);

        m_Surface = m_Owner->getVkInstance().createWin32SurfaceKHRUnique(info);
    }
}
