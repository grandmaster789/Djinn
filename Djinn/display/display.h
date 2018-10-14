#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"

#include "monitor.h"

#include <memory>

namespace djinn {
    namespace display {
        class Window;
        class WindowHints;
    }

    /*
        For simplicity, this only describes a single window.
        Multi-window could be a thing, but until it's needed it'll only
        be a lot of complication.
    */
    class Display :
        public core::System,
        public MessageHandler<display::Monitor::OnConnected>,
        public MessageHandler<display::Monitor::OnDisconnected>
    {
    private:
        struct MonitorDeleter;

        using  Monitor     = display::Monitor;
        using  MonitorPtr  = std::unique_ptr<Monitor, MonitorDeleter>;
        using  MonitorList = std::vector<MonitorPtr>;

        struct MonitorDeleter { void operator()(Monitor* m); };

        using WindowHints = display::WindowHints;
        using Window      = display::Window;
        using WindowPtr   = std::unique_ptr<Window>;
        using WindowList  = std::vector<WindowPtr>;

    public:
        Display();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

        // ----- Monitors -----
        void               detectMonitors();
        Monitor*           getPrimaryMonitor() const;
        const MonitorList& getMonitorList() const;
        size_t             getNumMonitors() const;

        void operator()(const Monitor::OnConnected& o);
        void operator()(const Monitor::OnDisconnected& o);

        // ----- Window -----
              Window* getWindow();
        const Window* getWindow() const;

        // ----- Vulkan support -----
              vk::Instance        getVkInstance()       const;
        const vk::PhysicalDevice& getVkPhysicalDevice() const;
              vk::Device          getVkDevice()         const;

        uint32_t getGraphicsFamilyIdx() const;
        uint32_t getPresentFamilyIdx()  const;
        uint32_t getComputeFamilyIdx()  const;
        uint32_t getTransferFamilyIdx() const;
        
        vk::Queue getGraphicsQueue() const;
        vk::Queue getPresentQueue()  const;
        vk::Queue getComputeQueue()  const;
        vk::Queue getTransferQueue() const;

    private:
        void createWindow(const std::string& title, int width, int height);	// window on primary monitor
        void createWindow(const std::string& title, const Monitor* m);		// borderless fullscreen on the specified monitor
        void createWindow(const std::string& title, const Monitor* m, int width, int height); // fullscreen at the specified monitor with the specified resolution

        void createVkInstance(); // might throw vk::SystemError
        void setupVkDebugCallback();
        void selectVkPhysicalDevice();
		void setupVkQueues();
		void setupVkDevice();
        
        struct {
            int m_Width = 800;
            int m_Height = 600;
            bool m_Fullscreen = false;
            bool m_Borderless = false;
        } m_WindowSettings;

        bool m_UseValidation = false;

        MonitorList m_Monitors;
        WindowPtr m_Window;

		std::vector<const char*> m_RequiredDeviceExtensions;
		std::vector<const char*> m_RequiredInstanceExtensions;
		std::vector<const char*> m_RequiredInstanceLayers;

        vk::UniqueInstance               m_VkInstance;
        vk::PhysicalDevice               m_VkPhysicalDevice;
        vk::UniqueDevice                 m_VkDevice;        
        vk::UniqueDebugReportCallbackEXT m_VkDebugCallback;

        vk::SampleCountFlagBits m_MaxSampleCount;
		vk::QueueFlags          m_SupportedQueues;

		static constexpr const uint32_t IDX_NOT_FOUND = std::numeric_limits<uint32_t>::max();

		uint32_t m_GraphicsFamilyIdx = IDX_NOT_FOUND;
		uint32_t m_PresentFamilyIdx  = IDX_NOT_FOUND;
		uint32_t m_ComputeFamilyIdx  = IDX_NOT_FOUND;
		uint32_t m_TransferFamilyIdx = IDX_NOT_FOUND;

		vk::Queue m_GraphicsQueue;
		vk::Queue m_PresentQueue;
		vk::Queue m_ComputeQueue;
		vk::Queue m_TransferQueue;
    };
}
