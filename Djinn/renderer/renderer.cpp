#include "renderer.h"
#include "core/engine.h"

namespace djinn {
    void Renderer::GLFWwindowDeleter::operator()(GLFWwindow* handle) {
        glfwDestroyWindow(handle);
    }

    Renderer::Renderer():
        System("Renderer")
    {
        registerSetting("width",  &m_WindowSettings.m_Width);
        registerSetting("height", &m_WindowSettings.m_Height);
    }

    void Renderer::init() {
        System::init();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        m_Window.reset(
            glfwCreateWindow(
                m_WindowSettings.m_Width,
                m_WindowSettings.m_Height,
                "Djinn",
                nullptr, // monitor
                nullptr // shared context
            )
        );

        
    }

    void Renderer::update() {
        if (glfwWindowShouldClose(m_Window.get()))
            m_Engine->stop();

        glfwPollEvents();
    }

    void Renderer::shutdown() {
        System::shutdown();
    }

    void Renderer::unittest() {
    }
}