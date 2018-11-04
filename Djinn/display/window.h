#pragma once

#include "dependencies.h"

namespace djinn::display {
    class Window {
    public:
        Window(int width, int height);
        ~Window();

        // move-only (with custom move code)
        // we need custom move code in order synchronize the 
        // object pointer associated with the wrapped handle
        Window             (const Window&) = delete;
        Window& operator = (const Window&) = delete;
        Window             (Window&&); 
        Window& operator = (Window&&);

        HWND getHandle() const;

        LRESULT winProc(
            HWND handle, 
            UINT message, 
            WPARAM wp, 
            LPARAM lp
        );

    private:
        HWND m_Handle = nullptr;
    };
}
