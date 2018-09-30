#include "window.h"
#include "renderer.h"
#include "monitor.h"

// ----- GLFW window callbacks -----
namespace {
    using djinn::broadcast;
    using djinn::renderer::Window;

    void windowCloseCallback(GLFWwindow* handle) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        broadcast(Window::OnClosed{ w });
    }

    void windowFocusCallback(GLFWwindow* handle, int state) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        switch (state) {
        case 0: broadcast(Window::OnFocusLost{ w }); break;
        case 1: broadcast(Window::OnFocused{ w }); break;
        default:
            gLogWarning << "Unsupported window focus state from callback: " << state;
        }
    }

    void windowIconifyCallback(GLFWwindow* handle, int state) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        switch (state) {
        case 0: broadcast(Window::OnRestore{ w }); break;
        case 1: broadcast(Window::OnIconify{ w }); break;
        default:
            gLogWarning << "Unsupported window iconify state from callback: " << state;
        }
    }

    void windowPositionCallback(GLFWwindow* handle, int x, int y) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        auto oldPosition = w->getPosition();
        broadcast(Window::OnMoved{ w, oldPosition.first, oldPosition.second, x, y });
    }

    void windowRefreshCallback(GLFWwindow* handle) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        broadcast(Window::OnRefreshed{ w });
    }

    void windowResizeCallback(GLFWwindow* handle, int width, int height) {
        Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        assert(w);

        // if the window has a swapchain, update it
        /*
        if (w->getSwapchain().isValid())
            w->getSwapchain().resize(width, height);
        */

        auto oldSize = w->getSize();
        broadcast(Window::OnResized{ w, oldSize.first, oldSize.second, width, height });
    }
}

namespace djinn::renderer {
    Window::Window(GLFWwindow* handle, const std::string& title):
        m_Handle(handle),
        m_Title(title)
    {
        setGLFWcallbacks();
    }

    Window::~Window() {
        if (m_Handle)
            glfwDestroyWindow(m_Handle);
    }

    Window::Window(Window&& w):
        m_Handle (w.m_Handle),
        m_Title  (std::move(w.m_Title)),
        m_Context(w.m_Context)
    {
        w.m_Handle = nullptr;
    }

    Window& Window::operator = (Window&& w) {
        if (m_Handle)
            glfwDestroyWindow(m_Handle);

        m_Handle  = w.m_Handle;
        m_Title   = w.m_Title;
        m_Context = w.m_Context;

        w.m_Handle = nullptr;

        return *this;
    }

    GLFWwindow* Window::getHandle() const {
        return m_Handle;
    }

    bool Window::shouldClose() const {
        return (glfwWindowShouldClose(m_Handle) != 0);
    }

    void Window::setShouldClose(bool value) {
        glfwSetWindowShouldClose(m_Handle, value ? 1 : 0);
    }

    void Window::setTitle(const std::string& title) {
        m_Title = title;
        glfwSetWindowTitle(m_Handle, title.c_str());
    }

    void Window::setPosition(int x, int y) {
        glfwSetWindowPos(m_Handle, x, y);
    }

    void Window::setSize(int width, int height) {
        glfwSetWindowSize(m_Handle, width, height);
    }

    void Window::iconify() {
        glfwIconifyWindow(m_Handle);
    }

    void Window::restore() {
        glfwRestoreWindow(m_Handle);
    }

    void Window::show() {
        glfwShowWindow(m_Handle);
    }

    void Window::hide() {
        glfwHideWindow(m_Handle);
    }

    std::pair<int, int> Window::getPosition() const {
        int x = 0;
        int y = 0;
        
        glfwGetWindowPos(m_Handle, &x, &y);
        
        return std::make_pair(x, y);
    }

    std::pair<int, int> Window::getSize() const {
        int width = 0;
        int height = 0;

        glfwGetWindowSize(m_Handle, &width, &height);

        return std::make_pair(width, height);
    }

    Window::Frame Window::getFrame() const {
        Frame result;

        glfwGetWindowFrameSize(
            m_Handle,
            &result.m_Left,
            &result.m_Top,
            &result.m_Right,
            &result.m_Bottom
        );

        return result;
    }

    const Monitor* Window::getMonitor() const {
        return detail::fetch(glfwGetWindowMonitor(m_Handle));
    }
    
    bool Window::isFocused() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_FOCUSED) != 0);
    }

    bool Window::isIconified() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_ICONIFIED) != 0);
    }

    bool Window::isVisible() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_VISIBLE) != 0);
    }

    bool Window::isResizable() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_RESIZABLE) != 0);
    }

    bool Window::isDecorated() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_DECORATED) != 0);
    }

    bool Window::isFloating() const {
        return (glfwGetWindowAttrib(m_Handle, GLFW_FLOATING) != 0);
    }

    void Window::setGLFWcallbacks() {
        // associate the handle with a pointer to this object
        glfwSetWindowUserPointer(m_Handle, this);

        // [NOTE] the framebuffer callback is missing here, because this is specific to an openGL context
        glfwSetWindowCloseCallback  (m_Handle, &windowCloseCallback);
        glfwSetWindowFocusCallback  (m_Handle, &windowFocusCallback);
        glfwSetWindowIconifyCallback(m_Handle, &windowIconifyCallback);
        glfwSetWindowPosCallback    (m_Handle, &windowPositionCallback);
        glfwSetWindowRefreshCallback(m_Handle, &windowRefreshCallback);
        glfwSetWindowSizeCallback   (m_Handle, &windowResizeCallback);

        broadcast(OnCreated{ this });
    }
}