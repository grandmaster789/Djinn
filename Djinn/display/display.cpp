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
		selectVkPhysicalDevice();
		setupVkQueues();
		setupVkDevice();
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

		if (m_VkDevice)
			m_VkDevice->waitIdle();

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

        uint32_t count = 0;
        auto raw_list = glfwGetRequiredInstanceExtensions(&count);
        for (uint32_t i = 0; i < count; ++i)
            m_RequiredInstanceExtensions.push_back(raw_list[i]);


        if (m_UseValidation) {
            m_RequiredInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");

            m_RequiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			m_RequiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // check that the required extensions/layers are actually available
        auto layers = enumerateInstanceLayerProperties();
        auto extensions = enumerateInstanceExtensionProperties();

        for (const auto& ext : m_RequiredInstanceExtensions)
            if (!isExtensionAvailable(ext, extensions))
                throw runtime_error(string("Instance extension unavailable: ") + string(ext));

        for (const auto& layer : m_RequiredInstanceLayers)
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
            .setPApplicationInfo       (&appInfo)
            .setEnabledLayerCount      ((uint32_t)m_RequiredInstanceLayers.size())
            .setPpEnabledLayerNames    (m_RequiredInstanceLayers.data())
            .setEnabledExtensionCount  ((uint32_t)m_RequiredInstanceExtensions.size())
            .setPpEnabledExtensionNames(m_RequiredInstanceExtensions.data());

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

        m_VkPhysicalDevice = all_physical_devices.front();
        gLog << "Selecting phyiscal device: " << m_VkPhysicalDevice.getProperties().deviceName;

        m_MaxSampleCount = findMaxSampleCount(m_VkPhysicalDevice.getProperties());

		// now that we have a physical device, create a surface 
		// for the window
		m_Window->initVkSurface(this);
    }

	void Display::setupVkQueues() {
		auto deviceQueueFamilies = m_VkPhysicalDevice.getQueueFamilyProperties();

		// [TODO] figure out some way to determine sets of candidates
		//        and then determining the best one
		for (uint32_t i = 0; i < deviceQueueFamilies.size(); ++i) {
			const auto& prop = deviceQueueFamilies[i];

			if (
				(m_GraphicsFamilyIdx == IDX_NOT_FOUND) &&
			    (prop.queueFlags & vk::QueueFlagBits::eGraphics) 
			) {
				m_GraphicsFamilyIdx = i;
				m_SupportedQueues |= vk::QueueFlagBits::eGraphics;
			}

			if (m_PresentFamilyIdx == IDX_NOT_FOUND) {
				auto has_present_support = m_VkPhysicalDevice.getSurfaceSupportKHR(i, m_Window->getSurface());

				if (
					(prop.queueCount > 0) &&
					(has_present_support)
				)
					m_PresentFamilyIdx = i;
			}

			if (
				(m_ComputeFamilyIdx == IDX_NOT_FOUND) &&
				(prop.queueFlags & vk::QueueFlagBits::eCompute)
			) {
				m_ComputeFamilyIdx = i;
				m_SupportedQueues |= vk::QueueFlagBits::eCompute;
			}

			if (
				(m_TransferFamilyIdx == IDX_NOT_FOUND) &&
				(prop.queueFlags & vk::QueueFlagBits::eTransfer)
			) {
				m_TransferFamilyIdx = i;
				m_SupportedQueues |= vk::QueueFlagBits::eTransfer;
			}

			if (
				(m_GraphicsFamilyIdx != IDX_NOT_FOUND) &&
				(m_PresentFamilyIdx  != IDX_NOT_FOUND) &&
				(m_ComputeFamilyIdx  != IDX_NOT_FOUND) &&
				(m_TransferFamilyIdx != IDX_NOT_FOUND)
			)
				break;
		}

		if (m_GraphicsFamilyIdx == -1)
			throw std::runtime_error("No graphics enabled vulkan queue was found");
	}

	void Display::setupVkDevice() {
		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		float defaultQueuePriority = 0.0f;

		if (m_SupportedQueues & vk::QueueFlagBits::eGraphics) {
			vk::DeviceQueueCreateInfo info;

			info.queueFamilyIndex = m_GraphicsFamilyIdx;
			info.queueCount       = 1;
			info.pQueuePriorities = &defaultQueuePriority;

			queueInfos.push_back(std::move(info));
		}
		
		if (
			(m_SupportedQueues & vk::QueueFlagBits::eCompute) &&
			(m_ComputeFamilyIdx != m_GraphicsFamilyIdx)
		) { 
			vk::DeviceQueueCreateInfo info;

			info.queueFamilyIndex = m_ComputeFamilyIdx;
			info.queueCount       = 1;
			info.pQueuePriorities = &defaultQueuePriority;

			queueInfos.push_back(std::move(info));
		}

		if (
			(m_SupportedQueues & vk::QueueFlagBits::eTransfer) &&
			(m_TransferFamilyIdx != m_GraphicsFamilyIdx) &&
			(m_TransferFamilyIdx != m_ComputeFamilyIdx)
		) {
			vk::DeviceQueueCreateInfo info;

			info.queueFamilyIndex = m_TransferFamilyIdx;
			info.queueCount       = 1;
			info.pQueuePriorities = &defaultQueuePriority;

			queueInfos.push_back(std::move(info));
		}

		vk::PhysicalDeviceFeatures requiredFeatures;
		vk::PhysicalDeviceFeatures availableFeatures = m_VkPhysicalDevice.getFeatures();

		requiredFeatures.fillModeNonSolid                     = VK_TRUE;
		requiredFeatures.fragmentStoresAndAtomics             = VK_TRUE;
		requiredFeatures.samplerAnisotropy                    = VK_TRUE;
		requiredFeatures.sampleRateShading                    = VK_TRUE;
		requiredFeatures.shaderClipDistance                   = VK_TRUE;
		requiredFeatures.shaderCullDistance                   = VK_TRUE;
		requiredFeatures.shaderStorageImageExtendedFormats    = VK_TRUE;
		requiredFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE;

		     if (availableFeatures.textureCompressionBC)       requiredFeatures.textureCompressionBC       = VK_TRUE;
		else if (availableFeatures.textureCompressionASTC_LDR) requiredFeatures.textureCompressionASTC_LDR = VK_TRUE;
		else if (availableFeatures.textureCompressionETC2)     requiredFeatures.textureCompressionETC2     = VK_TRUE;
		
		if (availableFeatures.tessellationShader)
			requiredFeatures.tessellationShader = VK_TRUE;
		else
			gLogError << "Physical device does not support tesselation shaders";

		if (availableFeatures.geometryShader)
			requiredFeatures.geometryShader = VK_TRUE;
		else
			gLogError << "Physical device does not support geometry shader";

        // make sure the device has swapchain support
		m_RequiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vk::DeviceCreateInfo info;

		info
			.setPEnabledFeatures       (&requiredFeatures)
			.setPpEnabledLayerNames    (m_RequiredInstanceLayers.data())
			.setEnabledLayerCount      ((uint32_t)m_RequiredInstanceLayers.size())
			.setPpEnabledExtensionNames(m_RequiredDeviceExtensions.data())
			.setEnabledExtensionCount  ((uint32_t)m_RequiredDeviceExtensions.size())
			.setPQueueCreateInfos      (queueInfos.data())
			.setQueueCreateInfoCount   ((uint32_t)queueInfos.size());

		m_VkDevice = m_VkPhysicalDevice.createDeviceUnique(info);

		m_GraphicsQueue = m_VkDevice->getQueue(m_GraphicsFamilyIdx, 0);
		m_PresentQueue  = m_VkDevice->getQueue(m_PresentFamilyIdx,  0);
		m_ComputeQueue  = m_VkDevice->getQueue(m_ComputeFamilyIdx,  0);
		m_TransferQueue = m_VkDevice->getQueue(m_TransferFamilyIdx, 0);

        m_Window->initVkSwapchain(this);
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

    vk::Instance Display::getVkInstance() const {
        return m_VkInstance.get();
    }

    const vk::PhysicalDevice& Display::getVkPhysicalDevice() const {
        return m_VkPhysicalDevice;
    }

    vk::Device Display::getVkDevice() const {
        return m_VkDevice.get();
    }

    uint32_t Display::getGraphicsFamilyIdx() const {
        return m_GraphicsFamilyIdx;
    }

    uint32_t Display::getPresentFamilyIdx()  const {
        return m_PresentFamilyIdx;
    }

    uint32_t Display::getComputeFamilyIdx()  const {
        return m_ComputeFamilyIdx;
    }
    
    uint32_t Display::getTransferFamilyIdx() const {
        return m_TransferFamilyIdx;
    }
}