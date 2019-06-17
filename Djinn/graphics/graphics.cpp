#include "graphics.h"
#include "core/engine.h"
#include "extensions.h"
#include "math/trigonometry.h"
#include "util/algorithm.h"
#include "util/flat_map.h"

#include <fstream>

// ------ Vulkan debug reports ------
namespace {
    VKAPI_ATTR VkBool32 VKAPI_CALL report_to_log(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT /* objectType */,
        uint64_t /* object     */,
        size_t /* location   */,
        int32_t     code,
        const char* layerPrefix,
        const char* message,
        void* /* userdata   */
    ) {
        // clang-format off
		     if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)               gLogDebug   << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)             gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)               gLogError   << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)         gLog        << "[" << layerPrefix << "] Code " << code << " : " << message;
        // clang-format on

        return VK_FALSE;
    }

    void checkInstanceLayers(
        const std::vector<const char*>&         requiredLayerNames,
        const std::vector<vk::LayerProperties>& availableLayers) {
        using djinn::util::contains_if;

        bool valid = true;

        // log all missing layers
        for (const auto& name : requiredLayerNames) {
            std::string s(name);

            if (!contains_if(availableLayers, [s](const vk::LayerProperties& prop) {
                    return (s == prop.layerName);
                })) {
                valid = false;
                gLogError << "Missing required layer: " << name;
            }
        }

        if (!valid) throw std::runtime_error("Missing required layer(s)");
    }

    void checkInstanceExtensions(
        const std::vector<const char*>&             requiredExtensions,
        const std::vector<vk::ExtensionProperties>& availableExtensions) {
        using djinn::util::contains_if;

        bool valid = true;

        // log all missing layers
        for (const auto& name : requiredExtensions) {
            std::string s(name);

            if (!contains_if(
                    availableExtensions, [s](const vk::ExtensionProperties& prop) {
                        return (s == prop.extensionName);
                    })) {
                valid = false;
                gLogError << "Missing required layer: " << name;
            }
        }

        if (!valid) throw std::runtime_error("Missing required layer(s)");
    }
}  // namespace

namespace djinn {
    Graphics::Graphics(): System("Graphics") {
        registerSetting("Width", &m_MainWindowSettings.m_Width);
        registerSetting("Height", &m_MainWindowSettings.m_Height);
        registerSetting("Windowed", &m_MainWindowSettings.m_Windowed);
        registerSetting("DisplayDevice", &m_MainWindowSettings.m_DisplayDevice);
    }

    void Graphics::init() {
        System::init();

        initVulkan();

        createWindow(
            m_MainWindowSettings.m_Width,
            m_MainWindowSettings.m_Height,
            m_MainWindowSettings.m_Windowed,
            m_MainWindowSettings.m_DisplayDevice);

        // [NOTE] this doesn't really belong here, but it's the first time this has come up
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
        {
            char executable_name[MAX_PATH + 1] = {};
            GetModuleFileName(GetModuleHandle(NULL), executable_name, MAX_PATH);

            std::filesystem::path executable_path(executable_name);
            auto                  folder = executable_path.parent_path();

            char dir[MAX_PATH + 1] = {};
            strcpy_s(dir, sizeof(dir), folder.string().c_str());
            SetCurrentDirectory(dir);

            char current[MAX_PATH + 1];
            GetCurrentDirectory(MAX_PATH + 1, current);
        }
#else
#error Unsupported platform
#endif
    }

    void Graphics::update() {
        if (!m_Windows.empty()) {
            MSG msg = {};

            while (PeekMessage(
                &msg,      // message
                nullptr,   // hwnd
                0,         // msg filter min
                0,         // msg filter max
                PM_REMOVE  // remove message
                )) {
                if (msg.message == WM_QUIT) m_Engine->stop();

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (m_Windows.empty()) return;
        }
    }

    void Graphics::shutdown() {
        System::shutdown();

        m_Windows.clear();
    }

    Graphics::Window* Graphics::getMainWindow() {
        assert(!m_Windows.empty());
        return m_Windows.front().get();
    }

    const Graphics::Window* Graphics::getMainWindow() const {
        assert(!m_Windows.empty());
        return m_Windows.front().get();
    }

    void Graphics::close(Window* w) {
        util::erase_if(m_Windows, [=](const WindowPtr& wp) { return wp.get() == w; });
    }

    vk::Instance Graphics::getInstance() const {
        return *m_Instance;
    }

    vk::PhysicalDevice Graphics::getPhysicalDevice() const {
        return m_PhysicalDevice;
    }

    vk::Device Graphics::getDevice() const {
        return *m_Device;
    }

    vk::CommandPool Graphics::getCommandPool() const {
        return *m_CommandPool;
    }

    vk::CommandBuffer Graphics::getCommandBuffer() const {
        return *m_CommandBuffer;
    }

    Graphics::Window*
        Graphics::createWindow(int width, int height, bool windowed, int displayDevice) {
        m_Windows.push_back(
            std::make_unique<Window>(width, height, windowed, displayDevice, this));

        return m_Windows.back().get();
    }

    void Graphics::initVulkan() {
        {
            // setup Instance
            auto availableLayers     = vk::enumerateInstanceLayerProperties();
            auto availableExtensions = vk::enumerateInstanceExtensionProperties();

            // [NOTE] possibly the debug report should be optional

            std::vector<const char*> requiredLayers = {};
            std::vector<const char*> requiredExtensions
                = {VK_KHR_SURFACE_EXTENSION_NAME,
                   VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                   VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

            checkInstanceLayers(requiredLayers, availableLayers);
            checkInstanceExtensions(requiredExtensions, availableExtensions);

            vk::ApplicationInfo           appInfo;
            vk::InstanceCreateInfo        instInfo;
            vk::Win32SurfaceCreateInfoKHR surfaceInfo;

            appInfo.setPApplicationName("Bazaar")
                .setApplicationVersion(1)
                .setPEngineName("Djinn")
                .setEngineVersion(1)
                .setApiVersion(VK_API_VERSION_1_1);

            instInfo.setFlags(vk::InstanceCreateFlags())
                .setPApplicationInfo(&appInfo)
                .setEnabledExtensionCount(
                    static_cast<uint32_t>(requiredExtensions.size()))
                .setPpEnabledExtensionNames(requiredExtensions.data())
                .setEnabledLayerCount(static_cast<uint32_t>(requiredLayers.size()))
                .setPpEnabledLayerNames(requiredLayers.data());

            m_Instance = vk::createInstanceUnique(instInfo);
        }

        // provide extension signatures etc
        graphics::loadInstanceExtensions(*m_Instance);

        {
            // setup debug report
            vk::DebugReportCallbackCreateInfoEXT info;

            info.setFlags(
                    vk::DebugReportFlagBitsEXT::ePerformanceWarning
                    | vk::DebugReportFlagBitsEXT::eError
                    //| vk::DebugReportFlagBitsEXT::eInformation // very spammy
                    | vk::DebugReportFlagBitsEXT::eWarning
                    | vk::DebugReportFlagBitsEXT::eDebug)
                .setPfnCallback(report_to_log);

            m_DebugCallback = m_Instance->createDebugReportCallbackEXTUnique(info);
        }

        // pick a physical device to work with
        {
            // [NOTE] all of my development machines have just one actual physical device,
            //        theoretically this could work with multiple but I don't have the
            //        opportunity to develop with that in mind

            auto allDevices = m_Instance->enumeratePhysicalDevices();

            if (allDevices.empty())
                throw std::runtime_error("No suitable physical device available");

            // just pick the first one
            m_PhysicalDevice = allDevices.front();

            auto props = m_PhysicalDevice.getProperties();

            // log version info
            gLog << "Selected device: " << props.deviceName;
            gLog << "Device type: " << props.deviceType;

            auto driver_major = VK_VERSION_MAJOR(props.driverVersion);
            auto driver_minor = VK_VERSION_MINOR(props.driverVersion);
            auto driver_patch = VK_VERSION_PATCH(props.driverVersion);

            gLog << "Driver v" << driver_major << "." << driver_minor << "."
                 << driver_patch;

            auto api_major = VK_VERSION_MAJOR(props.apiVersion);
            auto api_minor = VK_VERSION_MINOR(props.apiVersion);
            auto api_patch = VK_VERSION_PATCH(props.apiVersion);

            gLog << "Vulkan v" << api_major << "." << api_minor << "." << api_patch;

            // fetch memory properties
            m_MemoryProps = m_PhysicalDevice.getMemoryProperties();
        }

        // setup a draw queue, init logical device
        // [TODO] add compute, transfer queues etc
        {
            auto queueFamilyProps = m_PhysicalDevice.getQueueFamilyProperties();
            if (queueFamilyProps.empty())
                throw std::runtime_error("No queue family properties available");

            auto hasFlags
                = [](const vk::QueueFamilyProperties& prop, vk::QueueFlags flags) {
                      return (prop.queueFlags & flags) == flags;
                  };

            for (uint32_t i = 0; i < queueFamilyProps.size(); ++i) {
                if (hasFlags(queueFamilyProps[i], vk::QueueFlagBits::eGraphics)) {
                    m_GraphicsFamilyIdx = i;
                    break;
                }
            }

            if (m_GraphicsFamilyIdx == NOT_FOUND)
                throw std::runtime_error("No graphics queue family was found");

            float priorities[] = {0.0f};

            std::vector<vk::DeviceQueueCreateInfo> queueInfos;
            std::vector<const char*>               deviceExtensions = {};
            std::vector<const char*>               deviceLayers     = {};

            queueInfos.emplace_back();
            queueInfos.back()
                .setQueueCount(1)
                .setQueueFamilyIndex(m_GraphicsFamilyIdx)
                .setPQueuePriorities(priorities);

            vk::DeviceCreateInfo deviceInfo;

            deviceInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueInfos.size()))
                .setPQueueCreateInfos(queueInfos.data())
                .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
                .setPpEnabledExtensionNames(deviceExtensions.data())
                .setEnabledLayerCount(static_cast<uint32_t>(deviceLayers.size()))
                .setPpEnabledLayerNames(deviceLayers.data());

            m_Device = m_PhysicalDevice.createDeviceUnique(deviceInfo);
        }

        // setup primary command pool + buffer
        {
            vk::CommandPoolCreateInfo info;
            info.setQueueFamilyIndex(m_GraphicsFamilyIdx);

            m_CommandPool = m_Device->createCommandPoolUnique(info);

            vk::CommandBufferAllocateInfo cbai;
            cbai.setCommandBufferCount(1)
                .setCommandPool(*m_CommandPool)
                .setLevel(vk::CommandBufferLevel::ePrimary);

            // [NOTE] allocateCommandBuffersUnique yields a vector...
            m_CommandBuffer = std::move(m_Device->allocateCommandBuffersUnique(cbai)[0]);
        }
    }
}  // namespace djinn
