#pragma once

#include "core/system.h"
#include "dependencies.h"
#include <memory>

namespace djinn {
    class Renderer :
        public core::System
    {
    public:
        Renderer();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

    private:
        struct {
            int m_Width = 800;
            int m_Height = 600;
        } m_WindowSettings;

        struct GLFWwindowDeleter { void operator()(GLFWwindow* handle); };

        // [NOTE] currently designed for just one (1) window
        std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_Window; 
    };
}
