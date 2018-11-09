#include "window.h"
#include "display.h"
#include "input/keyboard.h"
#include "input/input.h"
#include "core/engine.h"

namespace {
	static std::unordered_map<WPARAM, djinn::input::Keyboard::eKey> g_KeyMapping;

	djinn::input::Keyboard::eKey findKeyCode(WPARAM keyCode) {
		auto it = g_KeyMapping.find(keyCode);

		if (it != g_KeyMapping.end())
			return it->second;

		return djinn::input::Keyboard::eKey::undefined;
	}

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
    Window::Window(int width, int height, Display* owner):
        m_Owner(owner)
    {
		if (g_KeyMapping.empty())
			initKeyMapping();

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

		auto inputSystem = m_Owner->getEngine()->get<Input>();

		m_Keyboard = std::make_unique<Keyboard>(inputSystem);

        createSurface();
    }

    Window::~Window() {
        if (m_Handle) {
            DestroyWindow(m_Handle);
        }
    }

    Window::Window(Window&& w):
        m_Handle            (w.m_Handle)  ,
        m_Surface (std::move(w.m_Surface)),
		m_Keyboard(std::move(w.m_Keyboard))
    {
        SetWindowLongPtr(m_Handle, 0, (LONG_PTR)this);
        w.m_Handle = nullptr;
    }

    Window& Window::operator = (Window&& w) {
        if (&w == this)
            return *this;

        if (m_Handle)
            DestroyWindow(m_Handle);

        m_Handle   = w.m_Handle;
        m_Surface  = std::move(w.m_Surface);
		m_Keyboard = std::move(w.m_Keyboard);

        w.m_Handle = nullptr;

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
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: 
			{
				auto key = findKeyCode(wp);
				m_Keyboard->setKeyState(key, true);
				break;
			}

		case WM_KEYUP:
		case WM_SYSKEYUP: 
			{
				auto key = findKeyCode(wp);
				m_Keyboard->setKeyState(key, false);
				break;
			}
        }

        return DefWindowProc(handle, message, wp, lp);
    }

	void Window::initKeyMapping() {
		assert(g_KeyMapping.empty());

		using eKey = input::Keyboard::eKey;
		// VK_ stuff can be found in <winuser.h>
		
		// letters * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
		g_KeyMapping.insert({ 'A', eKey::a });
		g_KeyMapping.insert({ 'B', eKey::b });
		g_KeyMapping.insert({ 'C', eKey::c });
		g_KeyMapping.insert({ 'D', eKey::d });

		g_KeyMapping.insert({ 'E', eKey::e });
		g_KeyMapping.insert({ 'F', eKey::f });
		g_KeyMapping.insert({ 'G', eKey::g });
		g_KeyMapping.insert({ 'H', eKey::h });

		g_KeyMapping.insert({ 'I', eKey::i });
		g_KeyMapping.insert({ 'J', eKey::j });
		g_KeyMapping.insert({ 'K', eKey::k });
		g_KeyMapping.insert({ 'L', eKey::l });

		g_KeyMapping.insert({ 'M', eKey::m });
		g_KeyMapping.insert({ 'N', eKey::n });
		g_KeyMapping.insert({ 'O', eKey::o });
		g_KeyMapping.insert({ 'P', eKey::p });

		g_KeyMapping.insert({ 'Q', eKey::q });
		g_KeyMapping.insert({ 'R', eKey::r });
		g_KeyMapping.insert({ 'S', eKey::s });
		g_KeyMapping.insert({ 'T', eKey::t });

		g_KeyMapping.insert({ 'U', eKey::u });
		g_KeyMapping.insert({ 'V', eKey::v });
		g_KeyMapping.insert({ 'W', eKey::w });
		g_KeyMapping.insert({ 'X', eKey::x });

		g_KeyMapping.insert({ 'Y', eKey::y });
		g_KeyMapping.insert({ 'Z', eKey::z });

		// numbers * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
		g_KeyMapping.insert({ '1', eKey::_1 });
		g_KeyMapping.insert({ '2', eKey::_2 });
		g_KeyMapping.insert({ '3', eKey::_3 });
		g_KeyMapping.insert({ '4', eKey::_4 });

		g_KeyMapping.insert({ '5', eKey::_5 });
		g_KeyMapping.insert({ '6', eKey::_6 });
		g_KeyMapping.insert({ '7', eKey::_7 });
		g_KeyMapping.insert({ '8', eKey::_8 });

		g_KeyMapping.insert({ '9', eKey::_9 });
		g_KeyMapping.insert({ '0', eKey::_0 });

		// function keys
		g_KeyMapping.insert({ VK_F1, eKey::f1 });
		g_KeyMapping.insert({ VK_F2, eKey::f2 });
		g_KeyMapping.insert({ VK_F3, eKey::f3 });
		g_KeyMapping.insert({ VK_F4, eKey::f4 });

		g_KeyMapping.insert({ VK_F5, eKey::f5 });
		g_KeyMapping.insert({ VK_F6, eKey::f6 });
		g_KeyMapping.insert({ VK_F7, eKey::f7 });
		g_KeyMapping.insert({ VK_F8, eKey::f8 });

		g_KeyMapping.insert({ VK_F9,  eKey::f9  });
		g_KeyMapping.insert({ VK_F10, eKey::f10 });
		g_KeyMapping.insert({ VK_F11, eKey::f11 });
		g_KeyMapping.insert({ VK_F12, eKey::f12 });

		// symbol keys
		g_KeyMapping.insert({ '`',  eKey::backquote });
		g_KeyMapping.insert({ '\'', eKey::quote });
		g_KeyMapping.insert({ ',',  eKey::comma });
		g_KeyMapping.insert({ '.',  eKey::point });

		g_KeyMapping.insert({ '[', eKey::bracket_open  });
		g_KeyMapping.insert({ ']', eKey::bracket_close });
		g_KeyMapping.insert({ '=', eKey::equals        });
		g_KeyMapping.insert({ '-', eKey::minus         });

		g_KeyMapping.insert({ ';',  eKey::semicolon });
		g_KeyMapping.insert({ '/',  eKey::slash     });
		g_KeyMapping.insert({ '\\', eKey::backslash });

		// navigational keys
		g_KeyMapping.insert({ VK_UP,    eKey::up    });
		g_KeyMapping.insert({ VK_DOWN,  eKey::down  });
		g_KeyMapping.insert({ VK_LEFT,  eKey::left  });
		g_KeyMapping.insert({ VK_RIGHT, eKey::right });

		g_KeyMapping.insert({ VK_PRIOR, eKey::pg_up   });
		g_KeyMapping.insert({ VK_NEXT,  eKey::pg_down });
		g_KeyMapping.insert({ VK_HOME,  eKey::home    });
		g_KeyMapping.insert({ VK_END,   eKey::end     });

		g_KeyMapping.insert({ VK_INSERT, eKey::ins });
		g_KeyMapping.insert({ VK_DELETE, eKey::del });

		// everything else
		g_KeyMapping.insert({ VK_CONTROL, eKey::ctrl  });
		g_KeyMapping.insert({ VK_MENU,    eKey::alt   });
		g_KeyMapping.insert({ VK_SHIFT,   eKey::shift });
		g_KeyMapping.insert({ VK_SPACE,   eKey::space });

		g_KeyMapping.insert({ VK_TAB,     eKey::tab    });
		g_KeyMapping.insert({ VK_RETURN,  eKey::enter  });
		g_KeyMapping.insert({ VK_ESCAPE,  eKey::escape });
	}

    void Window::createSurface() {
        vk::Win32SurfaceCreateInfoKHR info = {};

        info
            .setHinstance(GetModuleHandle(NULL))
            .setHwnd     (m_Handle);

        m_Surface = m_Owner->getVkInstance().createWin32SurfaceKHRUnique(info);
    }
}
