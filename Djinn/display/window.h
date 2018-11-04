#pragma once

#include "dependencies.h"

namespace djinn {
    class Display;

    namespace display {
        class Window {
        public:
            Window(int width, int height, Display* owner);
            ~Window();

            // move-only (with custom move code)
            // we need custom move code in order synchronize the 
            // object pointer associated with the wrapped handle
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

            

        private:
            void createSurface();


            Display* m_Owner  = nullptr;
            HWND     m_Handle = nullptr;

            vk::UniqueSurfaceKHR m_Surface;
        };
    }
}
