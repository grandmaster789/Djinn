#include "graphics.h"
#include "window.h"
#include "vk_debug.h"
#include "vk_device.h"
#include "vk_command_manager.h"
#include "core/engine.h"
#include "util/algorithm.h"
#include "compile_options.h"

namespace djinn {
    Graphics::Graphics():
        System("Graphics")
    {
        registerSetting("Width",         &m_MainWindowSettings.m_Width);
        registerSetting("Height",        &m_MainWindowSettings.m_Height);
        registerSetting("Windowed",      &m_MainWindowSettings.m_Windowed);
        registerSetting("DisplayDevice", &m_MainWindowSettings.m_DisplayDevice);
    }

    void Graphics::init() {
        System::init();

        m_MainWindow = std::make_unique<Window>(
            m_MainWindowSettings.m_Width,
            m_MainWindowSettings.m_Height,
            m_MainWindowSettings.m_Windowed,
            m_MainWindowSettings.m_DisplayDevice,
            this
        );

        initVulkan();
    }

    void Graphics::update() {
        if (m_MainWindow) {
            MSG msg = {};

            while (PeekMessage(
                &msg,     // message
                nullptr,  // hwnd
                0,        // msg filter min
                0,        // msg filter max
                PM_REMOVE // remove message 
            )) {
                if (msg.message == WM_QUIT)
                    m_Engine->stop();

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!m_MainWindow)
                return;
        }
    }

    void Graphics::shutdown() {
        System::shutdown();
    }

    void Graphics::unittest() {
    }

    void Graphics::close(graphics::Window* window) {
        if (window == m_MainWindow.get())
            m_MainWindow.reset();
    }

    Graphics::Window* Graphics::getMainWindow() {
        return m_MainWindow.get();
    }

    const Graphics::Window* Graphics::getMainWindow() const {
        return m_MainWindow.get();
    }

    vk::Instance Graphics::getVkInstance() const {
        return m_VkInstance.get();
    }

    void Graphics::initVulkan() {
        auto availableInstanceVersion        = vk::enumerateInstanceVersion();
        auto availableInstanceLayerProperies = vk::enumerateInstanceLayerProperties();
        auto availableInstanceExtensions     = vk::enumerateInstanceExtensionProperties();

        gLog << "Vulkan version "
            << VK_VERSION_MAJOR(availableInstanceVersion) << "."
            << VK_VERSION_MINOR(availableInstanceVersion) << "."
            << VK_VERSION_PATCH(availableInstanceVersion);

        std::vector<const char*> requiredLayers = {};
        std::vector<const char*> requiredInstanceExtensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        };

#if DJINN_VULKAN_VALIDATION
        // https://vulkan.lunarg.com/doc/sdk/1.1.92.1/windows/validation_layers.html
        requiredLayers            .push_back("VK_LAYER_LUNARG_standard_validation"); // not sure if there is a macro definition for this
        requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        // verify that the required layers and extensions are actually available
        std::vector<std::string> missingLayers;
        std::vector<std::string> missingExtensions;

        auto hasLayer = [&](const std::string& needle) {
            using util::contains_if;
            using vk::LayerProperties;

            return contains_if(
                availableInstanceLayerProperies,
                [&](const LayerProperties& layer) {
                    return (needle == layer.layerName);
                }
            );
        };

        auto hasExtension = [&](const std::string& needle) {
            using util::contains_if;
            using vk::ExtensionProperties;

            return contains_if(
                availableInstanceExtensions,
                [&](const ExtensionProperties& extension) {
                    return (needle == extension.extensionName);
                }
            );
        };

        for (const auto& layer : requiredLayers)
            if (!hasLayer(layer))
                missingLayers.push_back(layer);

        for (const auto& extension : requiredInstanceExtensions)
            if (!hasExtension(extension))
                missingExtensions.push_back(extension);

        if (!(missingLayers.empty() && missingExtensions.empty())) {
            for (const auto& layer: missingLayers)
                gLogError << "Missing vulkan instance layer: " << layer;

            for (const auto& ext : missingExtensions)
                gLogError << "Missing vulkan instance extension: " << ext;

            throw std::runtime_error("Missing prerequisites for Vulkan initialization");
        }

        vk::ApplicationInfo ai = {};
        
        // don't actually have a good way to figure application info out yet.. not too important right now
        ai
            .setPApplicationName  ("Bazaar")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName       ("Djinn")
            .setEngineVersion     (VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion        (VK_API_VERSION_1_1);

        vk::InstanceCreateInfo ici = {};

        ici
            .setPApplicationInfo       (&ai)
            .setEnabledLayerCount      (static_cast<uint32_t>(requiredLayers.size()))
            .setPpEnabledLayerNames    (requiredLayers.data())
            .setEnabledExtensionCount  (static_cast<uint32_t>(requiredInstanceExtensions.size()))
            .setPpEnabledExtensionNames(requiredInstanceExtensions.data());
            
        m_VkInstance = vk::createInstanceUnique(ici);

        if (!m_VkInstance)
            throw std::runtime_error("Failed to create vulkan instance");

#if DJINN_VULKAN_VALIDATION
        m_Debugger = std::make_unique<VkDebug>(m_VkInstance.get());
#endif

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
        {
            vk::Win32SurfaceCreateInfoKHR info;

            info
                .setHinstance(GetModuleHandle(NULL))
                .setHwnd     (m_MainWindow->getHandle());

            m_Surface = m_VkInstance->createWin32SurfaceKHRUnique(info);
        }
#else
    #error unsupported platform
#endif

        const std::vector<const char*> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_Device = std::make_unique<VkDevice>(
            m_VkInstance.get(), 
            m_Surface.get(), 
            requiredLayers,
            requiredDeviceExtensions
        );

        m_CommandManager = std::make_unique<VkCommandManager>(
            m_Device->get(),
            m_Device->getQueueFamilyIndices(),
            m_ThreadCount
        );
    }
}