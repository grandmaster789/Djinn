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

		const vk::Instance& getVKInstance() const;

    private:
		struct GLFWwindowDeleter { void operator()(GLFWwindow* handle); };

		using WindowHandle = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

        struct {
            int m_Width = 800;
            int m_Height = 600;
        } m_WindowSettings;

        // [NOTE] currently designed for just one (1) window
        WindowHandle m_Window; 

		vk::UniqueInstance m_VKInstance;
    };
}
