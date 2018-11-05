#pragma once

#include "third_party.h"

namespace djinn {
    class Display;

    namespace display {
        class Window {
        public:
            Window(
                int      width, 
                int      height, 
                bool     windowed,
                int      displayDevice,
                Display* owner
            );
            ~Window();

            // move-only (with custom move code)
            // we need custom move code in order synchronize the 
            // object pointer associated with the wrapped handle
            // [NOTE] the move code turned out to be quite a hassle, it may be better
            //        to just get rid of it altogether
            Window             (const Window&) = delete;
            Window& operator = (const Window&) = delete;
            Window             (Window&&); 
            Window& operator = (Window&&);

            HWND getHandle() const;
            vk::SurfaceKHR getSurface() const;

            LRESULT winProc(
                HWND handle, 
                UINT message, 
                WPARAM wp, 
                LPARAM lp
            );

            bool isMainWindow() const;

        private:
            static std::vector<DISPLAY_DEVICE> enumerateDisplayDevices(); // https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/ns-wingdi-_display_devicea
            static DEVMODE                     getCurrentDisplayMode(DISPLAY_DEVICE dd); // https://docs.microsoft.com/en-us/windows/desktop/api/Wingdi/ns-wingdi-_devicemodea

            void createSurface();

            Display* m_Owner  = nullptr;
            HWND     m_Handle = nullptr;

            int m_Width  = 0;
            int m_Height = 0;

            inline static HWND s_MainWindow = nullptr;

            vk::UniqueSurfaceKHR m_Surface;
        };
    }
}
