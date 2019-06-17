#include "window.h"
#include "core/engine.h"
#include "graphics.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "swapchain.h"

namespace {
    static std::unordered_map<WPARAM, djinn::input::Keyboard::eKey> g_KeyMapping;

    djinn::input::Keyboard::eKey findKeyCode(WPARAM keyCode) {
        auto it = g_KeyMapping.find(keyCode);

        if (it != g_KeyMapping.end())
            return it->second;

        return djinn::input::Keyboard::eKey::undefined;
    }

    LRESULT CALLBACK deferWinProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) {
        LONG_PTR ptr = GetWindowLongPtr(window, 0);

        if (ptr == NULL)
            return DefWindowProc(window, msg, wp, lp);
        else
            return ((djinn::graphics::Window*)ptr)->winProc(window, msg, wp, lp);
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

            m_WindowClass.cbSize      = sizeof(m_WindowClass);
            m_WindowClass.style       = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
            m_WindowClass.lpfnWndProc = deferWinProc;
            m_WindowClass.cbClsExtra  = 0;
            m_WindowClass.cbWndExtra
                = sizeof(void*);  // we're going to associate a single pointer to the window object
            m_WindowClass.hInstance
                = GetModuleHandle(NULL);  // could also propagate from program entry point
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
}  // namespace

namespace djinn::graphics {
    Window::Window(int width, int height, bool windowed, int displayDevice, Graphics* owner):
        m_Owner(owner) {
        if (g_KeyMapping.empty())
            initKeyMapping();

        auto devices = enumerateDisplayDevices();

        if (devices.empty())
            throw std::runtime_error("No display devices are available");

        if (devices.size() < displayDevice)
        {
            gLogWarning << "Display device number " << displayDevice
                        << " is unavailable, falling back to primary display";
            displayDevice = 0;
        }

        auto deviceMode = getCurrentDisplayMode(devices[displayDevice]);

        RECT rect;

        // https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-styles
        DWORD style = 0;

        // https://docs.microsoft.com/en-us/windows/desktop/winmsg/extended-window-styles
        DWORD exStyle = 0;

        if (windowed)
        {
            // center the rect
            int device_x      = deviceMode.dmPosition.x;
            int device_y      = deviceMode.dmPosition.y;
            int device_width  = deviceMode.dmPelsWidth;
            int device_height = deviceMode.dmPelsHeight;

            rect.left   = device_x + (device_width - width) / 2;
            rect.top    = device_y + (device_height - height) / 2;
            rect.right  = rect.left + width;
            rect.bottom = rect.top + height;

            style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

            exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        }
        else
        {
            // fill up the entire device space
            int device_x      = deviceMode.dmPosition.x;
            int device_y      = deviceMode.dmPosition.y;
            int device_width  = deviceMode.dmPelsWidth;
            int device_height = deviceMode.dmPelsHeight;

            rect.left   = device_x;
            rect.top    = device_y;
            rect.right  = rect.left + device_width;
            rect.bottom = rect.top + device_height;

            style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

            exStyle = WS_EX_APPWINDOW;
        }

        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        const auto& wc = WndClass::instance().get();

        m_Handle = CreateWindowEx(
            exStyle,
            wc.lpszClassName,
            "Djinn",  // title
            style,
            rect.left,
            rect.top,
            rect.right - rect.left,  // width
            rect.bottom - rect.top,  // height
            nullptr,                 // parent window
            nullptr,                 // menu
            GetModuleHandle(NULL),   // hInstance
            nullptr                  // additional parameters
        );

        if (m_Handle)
        {
            SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);

            if (!s_MainWindow)
                s_MainWindow = m_Handle;

            GetClientRect(m_Handle, &rect);
            m_Width  = rect.right - rect.left;
            m_Height = rect.bottom - rect.top;

            ShowWindow(m_Handle, SW_SHOW);
            SetForegroundWindow(m_Handle);

            auto inputSystem = m_Owner->getEngine()->get<Input>();

            m_Keyboard = std::make_unique<Keyboard>(inputSystem);
            m_Mouse    = std::make_unique<Mouse>(inputSystem);
        }
        else
            throw std::runtime_error("Failed to create window");

        // setup vulkan surface
        // NOTE currently this is platform specific, like the rest of this class
        {
            vk::Win32SurfaceCreateInfoKHR info;
            info.setHinstance(GetModuleHandle(NULL)).setHwnd(m_Handle);

            m_Surface = m_Owner->getInstance().createWin32SurfaceKHRUnique(info);

            if (!m_Surface)
                throw std::runtime_error("Failed to create vulkan surface");
        }
    }

    Window::~Window() {
        if (m_Handle)
        {
            DestroyWindow(m_Handle);
        }
    }

    HWND Window::getHandle() const {
        return m_Handle;
    }

    void Window::initKeyMapping() {
        assert(g_KeyMapping.empty());

        using eKey = input::Keyboard::eKey;
        // VK_ macros (virtual key codes) can be found in <winuser.h>
        // https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes

        // letters * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
        g_KeyMapping.insert({'A', eKey::a});
        g_KeyMapping.insert({'B', eKey::b});
        g_KeyMapping.insert({'C', eKey::c});
        g_KeyMapping.insert({'D', eKey::d});

        g_KeyMapping.insert({'E', eKey::e});
        g_KeyMapping.insert({'F', eKey::f});
        g_KeyMapping.insert({'G', eKey::g});
        g_KeyMapping.insert({'H', eKey::h});

        g_KeyMapping.insert({'I', eKey::i});
        g_KeyMapping.insert({'J', eKey::j});
        g_KeyMapping.insert({'K', eKey::k});
        g_KeyMapping.insert({'L', eKey::l});

        g_KeyMapping.insert({'M', eKey::m});
        g_KeyMapping.insert({'N', eKey::n});
        g_KeyMapping.insert({'O', eKey::o});
        g_KeyMapping.insert({'P', eKey::p});

        g_KeyMapping.insert({'Q', eKey::q});
        g_KeyMapping.insert({'R', eKey::r});
        g_KeyMapping.insert({'S', eKey::s});
        g_KeyMapping.insert({'T', eKey::t});

        g_KeyMapping.insert({'U', eKey::u});
        g_KeyMapping.insert({'V', eKey::v});
        g_KeyMapping.insert({'W', eKey::w});
        g_KeyMapping.insert({'X', eKey::x});

        g_KeyMapping.insert({'Y', eKey::y});
        g_KeyMapping.insert({'Z', eKey::z});

        // numbers * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
        g_KeyMapping.insert({'1', eKey::_1});
        g_KeyMapping.insert({'2', eKey::_2});
        g_KeyMapping.insert({'3', eKey::_3});
        g_KeyMapping.insert({'4', eKey::_4});

        g_KeyMapping.insert({'5', eKey::_5});
        g_KeyMapping.insert({'6', eKey::_6});
        g_KeyMapping.insert({'7', eKey::_7});
        g_KeyMapping.insert({'8', eKey::_8});

        g_KeyMapping.insert({'9', eKey::_9});
        g_KeyMapping.insert({'0', eKey::_0});

        // function keys
        g_KeyMapping.insert({VK_F1, eKey::f1});
        g_KeyMapping.insert({VK_F2, eKey::f2});
        g_KeyMapping.insert({VK_F3, eKey::f3});
        g_KeyMapping.insert({VK_F4, eKey::f4});

        g_KeyMapping.insert({VK_F5, eKey::f5});
        g_KeyMapping.insert({VK_F6, eKey::f6});
        g_KeyMapping.insert({VK_F7, eKey::f7});
        g_KeyMapping.insert({VK_F8, eKey::f8});

        g_KeyMapping.insert({VK_F9, eKey::f9});
        g_KeyMapping.insert({VK_F10, eKey::f10});
        g_KeyMapping.insert({VK_F11, eKey::f11});
        g_KeyMapping.insert({VK_F12, eKey::f12});

        // symbol keys (assuming US standard keyboard)
        g_KeyMapping.insert({VK_OEM_1, eKey::semicolon});
        g_KeyMapping.insert({VK_OEM_2, eKey::questionmark});
        g_KeyMapping.insert({VK_OEM_3, eKey::tilde});
        g_KeyMapping.insert({VK_OEM_4, eKey::brace_open});
        g_KeyMapping.insert({VK_OEM_5, eKey::vertical_pipe});
        g_KeyMapping.insert({VK_OEM_6, eKey::brace_close});
        g_KeyMapping.insert({VK_OEM_7, eKey::double_quote});
        g_KeyMapping.insert({VK_OEM_8, eKey::oem_8});

        g_KeyMapping.insert({VK_OEM_PLUS, eKey::plus});
        g_KeyMapping.insert({VK_OEM_MINUS, eKey::minus});
        g_KeyMapping.insert({VK_OEM_COMMA, eKey::comma});
        g_KeyMapping.insert({VK_OEM_PERIOD, eKey::period});

        // navigational keys
        g_KeyMapping.insert({VK_UP, eKey::up});
        g_KeyMapping.insert({VK_DOWN, eKey::down});
        g_KeyMapping.insert({VK_LEFT, eKey::left});
        g_KeyMapping.insert({VK_RIGHT, eKey::right});

        g_KeyMapping.insert({VK_PRIOR, eKey::pg_up});
        g_KeyMapping.insert({VK_NEXT, eKey::pg_down});
        g_KeyMapping.insert({VK_HOME, eKey::home});
        g_KeyMapping.insert({VK_END, eKey::end});

        g_KeyMapping.insert({VK_INSERT, eKey::ins});
        g_KeyMapping.insert({VK_DELETE, eKey::del});

        // everything else
        g_KeyMapping.insert({VK_CONTROL, eKey::ctrl});
        g_KeyMapping.insert({VK_MENU, eKey::alt});
        g_KeyMapping.insert({VK_SHIFT, eKey::shift});
        g_KeyMapping.insert({VK_SPACE, eKey::space});

        g_KeyMapping.insert({VK_TAB, eKey::tab});
        g_KeyMapping.insert({VK_RETURN, eKey::enter});
        g_KeyMapping.insert({VK_ESCAPE, eKey::escape});
        g_KeyMapping.insert({VK_BACK, eKey::backspace});
    }

    LRESULT Window::winProc(HWND handle, UINT message, WPARAM wp, LPARAM lp) {
        using eMouseButton = input::Mouse::eButton;

        auto getMouseCoords = [this, lp] {
            int screen_x = GET_X_LPARAM(lp);
            int screen_y = GET_Y_LPARAM(lp);

            RECT rect;
            GetClientRect(m_Handle, &rect);

            // remap to [-1..1], flip the Y so that [-1, -1] is the bottom left of the window
            float x = -1.0f + 2.0f * (screen_x - rect.left) / (rect.right - rect.left);
            float y = 1.0f - 2.0f * (screen_y - rect.top) / (rect.bottom - rect.top);

            return std::make_pair(x, y);
        };

        switch (message)
        {
        case WM_DESTROY:
        {
            if (isMainWindow())
                PostQuitMessage(0);
            break;
        }

        case WM_CLOSE:
        {
            m_Owner->close(this);
            break;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            auto key = findKeyCode(wp);
            m_Keyboard->setKeyState(key, true);
            break;
        }

        case WM_PAINT:
        {
            ValidateRect(m_Handle, nullptr);  // this makes the entire client area redraw
            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            auto key = findKeyCode(wp);
            m_Keyboard->setKeyState(key, false);
            break;
        }

        case WM_MOUSEMOVE:
        {
            auto [x, y] = getMouseCoords();
            m_Mouse->setPosition(x, y);

            if (!m_CursorTracked)
            {
                TRACKMOUSEEVENT tracker = {};

                tracker.cbSize    = sizeof(tracker);
                tracker.dwFlags   = TME_LEAVE;
                tracker.hwndTrack = m_Handle;

                TrackMouseEvent(&tracker);
                m_CursorTracked = true;

                m_Mouse->doEnter(this);
            }

            break;
        }

        case WM_LBUTTONDOWN:
        {
            SetCapture(m_Handle);
            m_Mouse->setButtonState(eMouseButton::left, true);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            SetCapture(m_Handle);
            m_Mouse->setButtonState(eMouseButton::right, true);
            break;
        }
        case WM_MBUTTONDOWN:
        {
            SetCapture(m_Handle);
            m_Mouse->setButtonState(eMouseButton::middle, true);
            break;
        }

        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            m_Mouse->setButtonState(eMouseButton::left, false);
            break;
        }
        case WM_RBUTTONUP:
        {
            ReleaseCapture();
            m_Mouse->setButtonState(eMouseButton::right, false);
            break;
        }
        case WM_MBUTTONUP:
        {
            ReleaseCapture();
            m_Mouse->setButtonState(eMouseButton::middle, false);
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            SetCapture(m_Handle);
            m_Mouse->doDoubleClick(eMouseButton::left);
            break;
        }
        case WM_RBUTTONDBLCLK:
        {
            SetCapture(m_Handle);
            m_Mouse->doDoubleClick(eMouseButton::right);
            break;
        }
        case WM_MBUTTONDBLCLK:
        {
            SetCapture(m_Handle);
            m_Mouse->doDoubleClick(eMouseButton::middle);
            break;
        }

        case WM_MOUSEWHEEL:
        {
            m_Mouse->doScroll(GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA);
            break;
        }

        case WM_MOUSELEAVE:
        {
            m_CursorTracked = false;
            m_Mouse->doLeave(this);
            break;
        }

        case WM_SIZE:
        {
            // NOTE this is a bit weaksauce
            m_Width  = GET_X_LPARAM(lp);
            m_Height = GET_Y_LPARAM(lp);
            break;
        }

        default: return DefWindowProc(handle, message, wp, lp);
        }

        return 0;
    }

    bool Window::isMainWindow() const {
        return m_Handle == s_MainWindow;
    }

    const Window::Keyboard* Window::getKeyboard() const {
        return m_Keyboard.get();
    }

    const Window::Mouse* Window::getMouse() const {
        return m_Mouse.get();
    }

    uint32_t Window::getWidth() const {
        return m_Width;
    }

    uint32_t Window::getHeight() const {
        return m_Height;
    }

    void Window::initSwapchain() {
        m_Swapchain = std::make_unique<Swapchain>(
            m_Owner->getDevice(),
            m_Owner->getPhysicalDevice(),
            *m_Surface,
            m_Owner->getSurfaceFormat().format,
            m_Owner->getDepthFormat(),
            m_Owner->getPresentFamilyIdx());
    }

    vk::SurfaceKHR Window::getSurface() const {
        return m_Surface.get();
    }

    Swapchain* Window::getSwapchain() const {
        return m_Swapchain.get();
    }

    std::vector<DISPLAY_DEVICE> Window::enumerateDisplayDevices() {
        std::vector<DISPLAY_DEVICE> result;

        for (int i = 0;; i++)
        {
            DISPLAY_DEVICE dd = {};
            dd.cb             = sizeof(dd);

            // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-enumdisplaydevicesa
            if (!EnumDisplayDevices(NULL, i, &dd, 0))
                break;  // stop if we couldn't find more devices

            if (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
                continue;  // skip over mirroring (pseudo) devices

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
}  // namespace djinn::graphics
