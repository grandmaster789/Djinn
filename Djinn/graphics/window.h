#pragma once

#include "third_party.h"

/*
    This is *very* platform specific, just about all of it is using the windows API.
    However, because it is very centralized right now it should be fairly easy to
    switch to a cross platform interface at some point.

    - TODO: window resizing events
    - TODO: minimizing + maximizing
    - TODO: actual text input

    [NOTE] maybe switch to the raw input model instead of window message translation?
*/

namespace djinn {
    class Graphics;

	namespace input {
		class Mouse;
		class Keyboard;
	}

    namespace graphics {
        class Window {
        public:
            friend class Context;

			using Mouse    = input::Mouse;
			using Keyboard = input::Keyboard;

            Window(
                int       width,
                int       height,
                bool      windowed,
                int       displayDevice,
				Graphics* owner
            );
            ~Window();

            // move-only (with custom move code)
            // we need custom move code in order synchronize the 
            // object pointer associated with the wrapped handle
			// TBH this is quickly becoming a hassle
            Window             (const Window&) = delete;
            Window& operator = (const Window&) = delete;
            Window             (Window&&); 
            Window& operator = (Window&&);

            HWND           getHandle()  const;
            
            LRESULT winProc(
                HWND   handle, 
                UINT   message, 
                WPARAM wp, 
                LPARAM lp
            );   

            bool isMainWindow() const;

            const Keyboard* getKeyboard() const;
            const Mouse*    getMouse()    const;

            uint32_t getWidth() const;
            uint32_t getHeight() const;

        private:
            static std::vector<DISPLAY_DEVICE> enumerateDisplayDevices();                // https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/ns-wingdi-_display_devicea
            static DEVMODE                     getCurrentDisplayMode(DISPLAY_DEVICE dd); // https://docs.microsoft.com/en-us/windows/desktop/api/Wingdi/ns-wingdi-_devicemodea

			void initKeyMapping();

            Graphics* m_Owner  = nullptr;
            HWND      m_Handle = nullptr;

            int  m_Width  = 0;
            int  m_Height = 0;

            bool m_CursorTracked = false;
            
            inline static HWND s_MainWindow = nullptr;

			std::unique_ptr<Keyboard> m_Keyboard;
            std::unique_ptr<Mouse>    m_Mouse;
        };
    }
}
