#include "display.h"
#include "core/engine.h"
#include "core/mediator.h"
#include "window.h"
#include "window_hints.h"
#include <iterator>

// ----- GLFW monitor callback -----
namespace {
    void glfw_monitor_callback(GLFWmonitor* handle, int evt) {
        using djinn::broadcast;
        using djinn::display::Monitor;
        using djinn::display::detail::fetch;

        switch (evt) {
        case GLFW_CONNECTED:
            broadcast(Monitor::OnConnected{ handle });
            break;

        case GLFW_DISCONNECTED:
            broadcast(Monitor::OnDisconnected{ handle });
            break;

        default:
            gLogWarning << "Unknown monitor event: " << evt;
        }
    }
}

// ------ Vulkan debug reports ------
namespace {
    PFN_vkCreateDebugReportCallbackEXT  pfn_vkCreateDebugReportCallbackEXT = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT = nullptr;

    using string = std::string;
    using ExtensionList = std::vector<vk::ExtensionProperties>;
    using LayerList = std::vector<vk::LayerProperties>;

    bool isExtensionAvailable(
        const string&        name,
        const ExtensionList& availableExtensions
    ) {
        using djinn::util::contains_if;
        using vk::ExtensionProperties;

        return contains_if(
            availableExtensions,
            [&](const ExtensionProperties& extension) {
            return (name == extension.extensionName);
        }
        );
    }

    bool isLayerAvailable(
        const string&    name,
        const LayerList& availableLayers
    ) {
        using djinn::util::contains_if;
        using vk::LayerProperties;

        return contains_if(
            availableLayers,
            [&](const LayerProperties& layer) {
            return (name == layer.layerName);
        }
        );
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL report_to_log(
        VkDebugReportFlagsEXT      flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t                   object,
        size_t                     location,
        int32_t                    code,
        const char*                layerPrefix,
        const char*                message,
        void*                      userdata
    ) {
        (void)objectType;
        (void)object;
        (void)location;
        (void)userdata;

             if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)               gLogDebug << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)             gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)               gLogError << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)         gLog << "[" << layerPrefix << "] Code " << code << " : " << message;

        return VK_FALSE;
    }

    vk::SampleCountFlagBits findMaxSampleCount(vk::PhysicalDeviceProperties physicalProps) {
        // [NOTE] there are more options here (stencil, NoAttachments)
        auto check = [&](vk::SampleCountFlagBits bits) {
            return
                (physicalProps.limits.framebufferColorSampleCounts & bits) &&
                (physicalProps.limits.framebufferDepthSampleCounts & bits);
        };

        if (check(vk::SampleCountFlagBits::e64)) return vk::SampleCountFlagBits::e64;
        if (check(vk::SampleCountFlagBits::e32)) return vk::SampleCountFlagBits::e32;
        if (check(vk::SampleCountFlagBits::e16)) return vk::SampleCountFlagBits::e16;
        if (check(vk::SampleCountFlagBits::e8))  return vk::SampleCountFlagBits::e8;
        if (check(vk::SampleCountFlagBits::e4))  return vk::SampleCountFlagBits::e4;
        if (check(vk::SampleCountFlagBits::e2))  return vk::SampleCountFlagBits::e2;
        
        return vk::SampleCountFlagBits::e1;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugReportCallbackEXT*                 pCallback
) {
    if (pfn_vkCreateDebugReportCallbackEXT)
        return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    else
        // [NOTE] this is expected to be nothrow, noexcept
        //        so use an error code instead
        return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                   instance,
    VkDebugReportCallbackEXT     callback,
    const VkAllocationCallbacks* pAllocator
) {
    if (pfn_vkDestroyDebugReportCallbackEXT)
        pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    // [NOTE] this is expected to be nothrow, noexcept    
    //        so silent fail because of the void return type
}

namespace djinn {
    Display::Display() :
        System("Display")
    {
        registerSetting("width",      &m_WindowSettings.m_Width);
        registerSetting("height",     &m_WindowSettings.m_Height);
        registerSetting("fullscreen", &m_WindowSettings.m_Fullscreen);
        registerSetting("borderless", &m_WindowSettings.m_Borderless);

        registerSetting("vk_validation", &m_UseValidation);
    }

    void Display::init() {
        System::init();

        if (glfwVulkanSupported() == GLFW_FALSE)
            throw std::runtime_error("Vulkan is not supported");

        detectMonitors();
        glfwSetMonitorCallback(&glfw_monitor_callback);

        gLogDebug
            << "Detected "
            << getNumMonitors()
            << " monitor(s)";

        gLogDebug
            << "Primary monitor: "
            << *m_Monitors.front();

        {
            uint32_t count = 0;
            auto ext = glfwGetRequiredInstanceExtensions(&count);

            std::vector<const char*> requiredExtensions;
            std::vector<const char*> requiredLayers;

            for (uint32_t i = 0; i < count; ++i)
                requiredExtensions.push_back(ext[i]);
        }

        gLogDebug
            << "Creating main window: "
            << m_WindowSettings.m_Width
            << "x"
            << m_WindowSettings.m_Height
            << " fullscreen: "
            << m_WindowSettings.m_Fullscreen
            << " borderless: "
            << m_WindowSettings.m_Borderless;

        display::WindowHints hints;

        hints.m_Resizable = false; // TODO - resizing windows has some implications about recreating swapbuffer chains etc
        hints.m_Visible = true;
        hints.m_Decorated = !m_WindowSettings.m_Borderless;
        hints.m_Focused = true;
        hints.m_AutoIconify = true;
        hints.m_Floating = true; // always-on-top enabled

        hints.apply();

        // different ways to create a window:
        // 1) fullscreen            (implies switching video mode, requires target monitor)
        // 2) borderless fullscreen (ignores width/height, requires target monitor)
        // 3) borderless windowed 
        // 4) bordered windowed
        if (m_WindowSettings.m_Fullscreen) {
            if (m_WindowSettings.m_Borderless)
                createWindow("Djinn", getPrimaryMonitor());
            else
                createWindow(
                    "Djinn",
                    getPrimaryMonitor(),
                    m_WindowSettings.m_Width,
                    m_WindowSettings.m_Height
                );
        }
        else {
            if (m_WindowSettings.m_Borderless) {
                hints.m_Decorated = false;
                hints.apply();
            }

            createWindow(
                "Djinn",
                m_WindowSettings.m_Width,
                m_WindowSettings.m_Height
            );
        }

        // center the primary window
        {
            auto currentMode = getPrimaryMonitor()->getCurrentVideoMode();
            auto winSize = getWindow()->getSize();

            getWindow()->setPosition(
                (currentMode.width - winSize.first) / 2,
                (currentMode.height - winSize.second) / 2
            );
        }

        createVkInstance();
        setupVkDebugCallback();
        
        
    }

    void Display::update() {
        if (
            (!m_Window) || 
            (m_Window->shouldClose())
        )
            m_Engine->stop();

        // ~~ present surface here ?
        
        glfwPollEvents();

        {
            using namespace std;
            using namespace chrono_literals;
            this_thread::sleep_for(1ms); // just to reduce CPU load
        }
    }

    void Display::shutdown() {
        System::shutdown();
        
        m_Window.reset();
    }

    void Display::unittest() {
    }

    void Display::operator()(const Monitor::OnConnected& o) {
        // cant' use make_unique because of the custom deleter... -_-
        MonitorPtr ptr(new display::Monitor(o.m_Handle), {});

        gLogDebug << "Monitor connected: " << ptr->getName();
        display::detail::registerMonitor(ptr.get());

        m_Monitors.push_back(std::move(ptr));
    }

    void Display::operator()(const Monitor::OnDisconnected& o) {
        auto monitor = display::detail::fetch(o.m_Handle);
        assert(monitor != nullptr);

        gLogDebug << "Monitor disconnected: " << monitor->getName();

        auto it = util::find_if(m_Monitors, [&](const MonitorPtr& ptr) {
            return ptr->getHandle() == o.m_Handle;
        });

        m_Monitors.erase(it);
    }

    Display::Window* Display::getWindow() {
        return m_Window.get();
    }

    const Display::Window* Display::getWindow() const {
        return m_Window.get();
    }

    
    void Display::createWindow(
        const std::string& title,
        int width,
        int height
    ) {
        // windowed mode (both bordered and borderless)
        m_Window = std::make_unique<Window>(
            glfwCreateWindow(
                width,
                height,
                title.c_str(),
                nullptr,
                nullptr
            ),
            title
        );
    }

    void Display::createWindow(
        const std::string& title,
        const Monitor* m
    ) {
        auto mode = m->getCurrentVideoMode();

        // sigh... [TODO] put these in the WindowHints as well
        glfwWindowHint(GLFW_RED_BITS,     mode.redBits);
        glfwWindowHint(GLFW_GREEN_BITS,   mode.greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,    mode.blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode.refreshRate);

        // fullscreen borderless
        m_Window = std::make_unique<Window>(
            glfwCreateWindow(
                mode.width,
                mode.height,
                title.c_str(),
                m->getHandle(),
                nullptr
            ),
            title
        );
    }

    void Display::createWindow(
        const std::string& title,
        const Monitor* m,
        int width,
        int height
    ) {
        // fullscreen
        m_Window = std::make_unique<Window>(
            glfwCreateWindow(
                width,
                height,
                title.c_str(),
                m->getHandle(),
                nullptr
            ),
            title
        );
    }

    void Display::createVkInstance() {
        using std::vector;
        using std::string;
        using std::runtime_error;

        using vk::enumerateInstanceExtensionProperties;
        using vk::enumerateInstanceLayerProperties;
        using vk::ApplicationInfo;
        using vk::InstanceCreateInfo;
        using vk::createInstanceUnique;

        vector<const char*> requiredLayers;
        vector<const char*> requiredExtensions;

        uint32_t count = 0;
        auto raw_list = glfwGetRequiredInstanceExtensions(&count);
        for (uint32_t i = 0; i < count; ++i)
            requiredExtensions.push_back(raw_list[i]);


        if (m_UseValidation) {
            requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");

            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // check that the required extensions/layers are actually available
        auto layers = enumerateInstanceLayerProperties();
        auto extensions = enumerateInstanceExtensionProperties();

        for (const auto& ext : requiredExtensions)
            if (!isExtensionAvailable(ext, extensions))
                throw runtime_error(string("Instance extension unavailable: ") + string(ext));

        for (const auto& layer : requiredLayers)
            if (!isLayerAvailable(layer, layers))
                throw runtime_error(string("Instance layer unavailable: ") + string(layer));

        ApplicationInfo appInfo;
        appInfo
            .setPEngineName("Djinn")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPApplicationName("Bazaar")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(VK_API_VERSION_1_1);

        InstanceCreateInfo info;
        info
            .setPApplicationInfo(&appInfo)
            .setEnabledLayerCount((uint32_t)requiredLayers.size())
            .setPpEnabledLayerNames(requiredLayers.data())
            .setEnabledExtensionCount((uint32_t)requiredExtensions.size())
            .setPpEnabledExtensionNames(requiredExtensions.data());

        m_VkInstance = createInstanceUnique(info);
    }

    void Display::setupVkDebugCallback() {
        if (m_UseValidation) {
            vk::DebugReportCallbackCreateInfoEXT info;

            info.flags =
                vk::DebugReportFlagBitsEXT::eError |
                vk::DebugReportFlagBitsEXT::eWarning |
                vk::DebugReportFlagBitsEXT::eInformation |
                vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                vk::DebugReportFlagBitsEXT::eDebug;

            info.pfnCallback = &report_to_log;

            m_VkDebugCallback = m_VkInstance->createDebugReportCallbackEXTUnique(info);
        }
    }

    void Display::selectVkPhysicalDevice() {
        auto all_physical_devices = m_VkInstance->enumeratePhysicalDevices();

        // just pick the first available device
        // TODO - implement some kind of scoring system to select the best available
        //        AND/OR provide methods for multi-GPU usage

        /*
        gLogDebug << "Physical Device [0]: " << all_physical_devices.front().getProperties().deviceName;

        for (const auto& prop : props) {
            gLogDebug << "\t" << prop.extensionName;
        }
        */

        if (all_physical_devices.empty())
            throw std::runtime_error("No Vulkan devices available");

        m_PhysicalDevice = all_physical_devices.front();
        gLog << "Selecting phyiscal device: " << m_PhysicalDevice.getProperties().deviceName;

        m_MaxSampleCount = findMaxSampleCount(m_PhysicalDevice.getProperties());
    }

    void Display::createVkSurface() {
        m_Window->initVkSurface(m_VkInstance.get(), m_PhysicalDevice);
        
    }

    void Display::createVkQueueIndices() {
    }

    void Display::createVkDevice() {
    }
    
    void Display::detectMonitors() {
        m_Monitors.clear();
        int count = 0;

        // [NOTE] documentation says that the first monitor returned
        //        by glfwGetMonitors is *always* the primary monitor
        auto monitors = glfwGetMonitors(&count);
        for (int i = 0; i < count; ++i) {
            MonitorPtr ptr(new Monitor(monitors[i]), {});

            display::detail::registerMonitor(ptr.get());
            m_Monitors.push_back(std::move(ptr));
        }
    }

    Display::Monitor* Display::getPrimaryMonitor() const {
        return m_Monitors.front().get();
    }

    const Display::MonitorList& Display::getMonitorList() const {
        return m_Monitors;
    }

    size_t Display::getNumMonitors() const {
        return m_Monitors.size();
    }

    void Display::MonitorDeleter::operator()(Monitor* m) {
        display::detail::unregisterMonitor(m);
        delete m;
    }
}