#pragma once

#include "core/system.h"

namespace djinn {
    namespace graphics {
        class Window;
		class Renderer;

        class VkDebug;
        class VkDevice;
        class VkCommandManager;
        class VkCommandBuffer;
    }

    /*
        The purpose of this system is to provide wrappers for vulkan stuff. Some WSI integration
        is done here, but it is not a renderer. It provides functionality that a renderer can use.

        For now this is designed as a single window system, perhaps in the future
        this will be extended to a multi-window system.         
    */
    class Graphics:
        public core::System
    {
    public:
        using Window           = graphics::Window;
		using Renderer         = graphics::Renderer;

        using VkDebug          = graphics::VkDebug;
        using VkDevice         = graphics::VkDevice;
        using VkCommandManager = graphics::VkCommandManager;
        using VkCommandBuffer  = graphics::VkCommandBuffer;

        Graphics();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        void close(graphics::Window* window);

              Window* getMainWindow();
        const Window* getMainWindow() const;

        vk::Instance getVkInstance() const;
        
    private:
        struct WindowSettings {
            int  m_Width         = 1280;
            int  m_Height        = 720;
            int  m_DisplayDevice = 0;
            bool m_Windowed      = true; // only supporting borderless fullscreen windows right now
        } m_MainWindowSettings;

        static const size_t m_ThreadCount = 4; // could possibly make this runtime configurable as well

        void initVulkan();

        std::unique_ptr<Window>    m_MainWindow;
        vk::UniqueInstance         m_VkInstance;
        
        std::unique_ptr<VkDebug>   m_Debugger;
        vk::UniqueSurfaceKHR       m_Surface; 
        std::unique_ptr<VkDevice>  m_Device;

        std::unique_ptr<VkCommandManager> m_CommandManager;
    };
}
