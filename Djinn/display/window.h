#pragma once

#include "dependencies.h"

/*
    This is *very* platform specific, just about all of it is using the windows API.
    However, because it is very centralized right now it should be fairly easy to
    switch to a cross platform interface at some point.
*/

namespace djinn {
    class Display;

	namespace input {
		class Mouse;
		class Keyboard;
	}

    namespace display {
        class Window {
        public:
			using Mouse    = input::Mouse;
			using Keyboard = input::Keyboard;

            Window(int width, int height, Display* owner);
            ~Window();

            // move-only (with custom move code)
            // we need custom move code in order synchronize the 
            // object pointer associated with the wrapped handle
			// TBH this is quickly becoming a hassle
            Window             (const Window&) = delete;
            Window& operator = (const Window&) = delete;
            Window             (Window&&); 
            Window& operator = (Window&&);

            HWND getHandle() const;
            vk::SurfaceKHR getSurface() const;

            LRESULT winProc(
                HWND   handle, 
                UINT   message, 
                WPARAM wp, 
                LPARAM lp
            );   

        private:
			// while browsing the virtual key docs, it seems that
			// 
			void initKeyMapping();
            void createSurface();

            Display* m_Owner  = nullptr;
            HWND     m_Handle = nullptr;

			std::unique_ptr<Keyboard> m_Keyboard;

            vk::UniqueSurfaceKHR m_Surface;
        };
    }
}
