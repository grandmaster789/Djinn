#include "display.h"
#include "core/engine.h"
#include "util/flat_map.h"
#include "util/enum.h"

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
        const char*                      layerPrefix,
        const char*                      message,
              void*                      userdata
    ) {
        (void)objectType;
        (void)object;
        (void)location;
        (void)userdata;

             if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)               gLogDebug   << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)             gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)               gLogError   << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)         gLog        << "[" << layerPrefix << "] Code " << code << " : " << message;

        return VK_FALSE;
    }

    // will throw if the requested type is not found
    // [NOTE] some simple preference system for dedicated queues would be nice
    djinn::util::FlatMap<vk::QueueFlagBits, int> scanFamilies(
        const std::vector<vk::QueueFamilyProperties>& availableFamilies, 
        const std::vector<vk::QueueFlagBits>&         requestedTypes
    ) {
        using namespace djinn::util;

        FlatMap<vk::QueueFlagBits, int> result;

        auto scan = [](
            const vk::QueueFamilyProperties& props, 
            const vk::QueueFlagBits&         type
        ) {
            return (props.queueFlags & type);
        };

        for (const auto& type : requestedTypes) {
            auto it = find_if(availableFamilies, scan);

            if (it == std::end(availableFamilies))
                throw std::runtime_error("Requested queue family type not found");

            int idx = static_cast<int>(std::distance(availableFamilies.begin(), it));
            
            result.assign(type, idx);
        }

        return result;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
          VkInstance                          instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
          VkDebugReportCallbackEXT*           pCallback
) {
    if (pfn_vkCreateDebugReportCallbackEXT)
        return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    else
        // [NOTE] this is expected to be nothrow + noexcept
        //        so use an error code instead
        return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
          VkInstance               instance,
          VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks*   pAllocator
) {
    if (pfn_vkDestroyDebugReportCallbackEXT)
        pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    // [NOTE] this is expected to be nothrow + noexcept    
    //        so silent fail because of the void return type
}

namespace djinn {
    Display::Display() :
        System("Display")
    {
        registerSetting("Width",         &m_MainWindowSettings.m_Width);
        registerSetting("Height",        &m_MainWindowSettings.m_Height);
        registerSetting("Windowed",      &m_MainWindowSettings.m_Windowed);
        registerSetting("DisplayDevice", &m_MainWindowSettings.m_DisplayDevice);
    }

    void Display::init() {
        System::init();

        initVulkan();

        createWindow(
            m_MainWindowSettings.m_Width,
            m_MainWindowSettings.m_Height,
            m_MainWindowSettings.m_Windowed,
            m_MainWindowSettings.m_DisplayDevice
        );

        createVulkanDevice();
        
        for (auto& window : m_Windows)
            window->initSwapChain();

    }

    void Display::update() {
        if (!m_Windows.empty()) {
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
        }
    }

    void Display::shutdown() {
        System::shutdown();

        m_Windows.clear();
    }

    void Display::unittest() {
    }

    void Display::close(Window* w) {
        auto it = util::find_if(m_Windows, [w](const WindowPtr& win) { return (win.get() == w); });
        if (it != m_Windows.end())
            m_Windows.erase(it);
    }

    vk::Instance Display::getVkInstance() const {
        return *m_VkInstance;
    }

    vk::PhysicalDevice Display::getVkPhysicalDevice() const {
        return m_VkPhysicalDevice;
    }

    vk::Device Display::getVkDevice() const {
        return *m_VkDevice;
    }

    Display::Window* Display::createWindow(
        int  width,
        int  height,
        bool windowed,
        int  displayDevice
    ) {
        m_Windows.push_back(std::make_unique<Window>(
            width,
            height,
            windowed,
            displayDevice,
            this
        ));

        return m_Windows.back().get();
    }

    void Display::initVulkan() {
        auto availableInstanceVersion        = vk::enumerateInstanceVersion();
        auto availableInstanceLayerProperies = vk::enumerateInstanceLayerProperties();
        auto availableInstanceExtensions     = vk::enumerateInstanceExtensionProperties();

        gLog << "Vulkan version "
            << VK_VERSION_MAJOR(availableInstanceVersion) << "."
            << VK_VERSION_MINOR(availableInstanceVersion) << "."
            << VK_VERSION_PATCH(availableInstanceVersion);

        std::vector<const char*> requiredLayers = {};
        std::vector<const char*> requiredExtensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        };

        // in debug mode, use validation
#ifdef DJINN_DEBUG
        // https://vulkan.lunarg.com/doc/sdk/1.1.85.0/windows/validation_layers.html
        requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // not sure if there is a macro definition for this
        requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
        {
            // verify the required layers/extensions are available
            for (const auto& layerName : requiredLayers)
                if (!isLayerAvailable(layerName, availableInstanceLayerProperies))
                    throw std::runtime_error("Required layer not available");

            for (const auto& extensionName : requiredExtensions)
                if (!isExtensionAvailable(extensionName, availableInstanceExtensions))
                    throw std::runtime_error("Required extension not available");
        }


        {
            // create application-wide vulkan Instance
            vk::ApplicationInfo appInfo = {};

            appInfo
                .setApiVersion(VK_API_VERSION_1_1)
                .setPApplicationName("Bazaar")
                .setPEngineName("Djinn")
                .setEngineVersion(1);

            vk::InstanceCreateInfo info = {};

            info
                .setPApplicationInfo       (&appInfo)
                .setEnabledLayerCount      ((uint32_t)requiredLayers.size())
                .setPpEnabledLayerNames    (          requiredLayers.data())
                .setEnabledExtensionCount  ((uint32_t)requiredExtensions.size())
                .setPpEnabledExtensionNames(          requiredExtensions.data());

            m_VkInstance = vk::createInstanceUnique(info);
        }

#ifdef DJINN_DEBUG
        {
            // install vulkan debug callback
            pfn_vkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT) m_VkInstance->getProcAddr("vkCreateDebugReportCallbackEXT");
            pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)m_VkInstance->getProcAddr("vkDestroyDebugReportCallbackEXT");

            vk::DebugReportCallbackCreateInfoEXT info = {};

            info
                .setFlags(
                    vk::DebugReportFlagBitsEXT::eDebug |
                    vk::DebugReportFlagBitsEXT::eError |
                    //vk::DebugReportFlagBitsEXT::eInformation | // this one is kinda spammy
                    vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                    vk::DebugReportFlagBitsEXT::eWarning
                )
                .setPfnCallback(report_to_log);

            m_VkDebugReportCallback = m_VkInstance->createDebugReportCallbackEXTUnique(info);
        }
#endif
    }

    void Display::createVulkanDevice() {
        gLogDebug << "Available physical devices: ";
        auto available_physical_devices = m_VkInstance->enumeratePhysicalDevices();

        for (const auto& dev : available_physical_devices) {
            auto props = dev.getProperties();

            std::string gpu_type;

            switch (props.deviceType) {
            case vk::PhysicalDeviceType::eCpu:           gpu_type = "CPU";            break;
            case vk::PhysicalDeviceType::eDiscreteGpu:   gpu_type = "Discrete GPU";   break;
            case vk::PhysicalDeviceType::eIntegratedGpu: gpu_type = "Integrated GPU"; break;
            case vk::PhysicalDeviceType::eOther:         gpu_type = "Other";          break;
            case vk::PhysicalDeviceType::eVirtualGpu:    gpu_type = "Virtual GPU";    break;
            default:
                gpu_type = "<< UNKNOWN >>";
            }

            gLogDebug                     << props.deviceName 
                << "\n\tType:"            << gpu_type
                << "\n\tDriver version: " << props.driverVersion
                << "\n\tAPI version: " 
                    << VK_VERSION_MAJOR(props.apiVersion) << "."
                    << VK_VERSION_MINOR(props.apiVersion) << "."
                    << VK_VERSION_PATCH(props.apiVersion);
        }

        // we *require*:
        // - swapchain support
        std::vector<const char*> requiredDeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        std::vector<const char*> requiredDeviceLayers;

#ifdef DJINN_DEBUG
        requiredDeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

        auto hasRequirements = [=](const vk::PhysicalDevice& device) {
            auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
            auto availableDeviceLayers     = device.enumerateDeviceLayerProperties();

            for (const auto& requirement : requiredDeviceExtensions)
                if (!isExtensionAvailable(requirement, availableDeviceExtensions))
                    return false;

            for (const auto& requirement : requiredDeviceLayers)
                if (!isLayerAvailable(requirement, availableDeviceLayers))
                    return false;

            return true;
        };
        
        // we *prefer*:
        // - discrete GPU
        // - the largest texture size available
        
        // filter available devices for required features
        std::vector<vk::PhysicalDevice> candidates;
        util::copy_if(available_physical_devices, candidates, hasRequirements);

        if (candidates.empty())
            throw std::runtime_error("None of the physical devices conforms to the requirements");

        // if we only have one candidate, we're done
        if (candidates.size() == 1) {
            m_VkPhysicalDevice = candidates[0];
            gLog << "Selected " << m_VkPhysicalDevice.getProperties().deviceName;            
        }
        else {
            // [TODO] implement scoring to order candidates and pick the best one
            m_VkPhysicalDevice = candidates[0];
            gLog << "Selected " << m_VkPhysicalDevice.getProperties().deviceName;
        }

        m_VkPhysicalDeviceMemoryProperties = m_VkPhysicalDevice.getMemoryProperties();

        // for now, I'm going for a two-queue approach; one for graphics and one for
        // transfers. Probably this will be extended to more queues at some point.
        {
            vk::DeviceQueueCreateInfo queue_info[2];

            float priorities[] = { 
                1.0f, 
                0.0f 
            };

            auto availableQueueFamilies = m_VkPhysicalDevice.getQueueFamilyProperties();
            if (availableQueueFamilies.empty())
                throw std::runtime_error("No vulkan queue families available...");

            // scan the available families for graphics and transfer queues;
            // [TODO] prefer separate families if possible
            auto selection = scanFamilies(
                availableQueueFamilies,
                { 
                    vk::QueueFlagBits::eGraphics,
                    vk::QueueFlagBits::eTransfer
                }
            );

            // right now we only have either 1 family for both queue types
            // or 2 families... when we get more, this should be revisited

            int numQueues;

            if (selection[vk::QueueFlagBits::eGraphics] == selection[vk::QueueFlagBits::eTransfer]) {
                numQueues = 1;
            }
            else {
                numQueues = 2;
            }

            queue_info[0]
                .setQueueCount      (1)
                .setQueueFamilyIndex(0)
                .setPQueuePriorities(priorities);

            vk::DeviceCreateInfo info;

            info
                .setQueueCreateInfoCount   (1)
                .setPQueueCreateInfos      (queue_info)
                .setEnabledLayerCount      ((uint32_t)requiredDeviceLayers.size())
                .setPpEnabledLayerNames    (          requiredDeviceLayers.data())
                .setEnabledExtensionCount  ((uint32_t)requiredDeviceExtensions.size())
                .setPpEnabledExtensionNames(          requiredDeviceExtensions.data());

            m_VkDevice = m_VkPhysicalDevice.createDeviceUnique(info);
        }
    }

}