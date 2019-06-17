#pragma once

#include "third_party.h"
#include "window.h"

namespace djinn::graphics {
    class SwapChain {
    public:
        SwapChain(Window* w);

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain(SwapChain&&)                 = delete;
        SwapChain& operator=(SwapChain&&) = delete;

    private:
        Window* m_Window;
    };
}  // namespace djinn::graphics
