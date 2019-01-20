#include "context.h"
#include "core/engine.h"
#include "util/flat_map.h"
#include "util/enum.h"
#include "util/algorithm.h"

#include <fstream>

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
    djinn::util::FlatMap<vk::QueueFlagBits, uint32_t> scanFamilies(
        const std::vector<vk::QueueFamilyProperties>& availableFamilies, 
        const std::vector<vk::QueueFlagBits>&         requestedTypes
    ) {
        using namespace djinn::util;

        FlatMap<vk::QueueFlagBits, uint32_t> result;

        for (const auto& type : requestedTypes) {
            auto scan = [type](const vk::QueueFamilyProperties& props) {
                return (props.queueFlags & type);
            };

            auto it = find_if(availableFamilies, scan);

            if (it == std::end(availableFamilies))
                throw std::runtime_error("Requested queue family type not found");

            uint32_t idx = static_cast<uint32_t>(std::distance(availableFamilies.begin(), it));
            
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
    Context::Context() :
        System("Display")
    {
        registerSetting("Width",         &m_MainWindowSettings.m_Width);
        registerSetting("Height",        &m_MainWindowSettings.m_Height);
        registerSetting("Windowed",      &m_MainWindowSettings.m_Windowed);
        registerSetting("DisplayDevice", &m_MainWindowSettings.m_DisplayDevice);
    }

    void Context::init() {
        System::init();

        initVulkan();
        selectPhysicalDevice();
        createDevice();

        createWindow(
            m_MainWindowSettings.m_Width,
            m_MainWindowSettings.m_Height,
            m_MainWindowSettings.m_Windowed,
            m_MainWindowSettings.m_DisplayDevice
        );

        createSurface();

        // make sure the surface supports presenting with the selected physical device
        if (!m_PhysicalDevice.getWin32PresentationSupportKHR(m_GraphicsFamilyIdx))
            throw std::runtime_error("Physical device does not support presenting to the surface");

        selectSwapchainFormat();
        m_Renderpass = createRenderpass(); // depends on the swapchain format
        createSwapchain();

        m_ImageAvailableSemaphore = m_Device->createSemaphoreUnique({});
        m_PresentCompletedSemaphore = m_Device->createSemaphoreUnique({});

        m_GraphicsQueue = m_Device->getQueue(m_GraphicsFamilyIdx, 0);

        createCommandPool();

        m_PipelineCache = m_Device->createPipelineCacheUnique({});

        // [NOTE] this doesn't really belong here, but it's the first time this has come up
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
        {
            char executable_name[MAX_PATH + 1] = {};
            GetModuleFileName(GetModuleHandle(NULL), executable_name, MAX_PATH);

            std::filesystem::path executable_path(executable_name);
            auto folder = executable_path.parent_path();

            char dir[MAX_PATH + 1] = {};
            strcpy_s(dir, sizeof(dir), folder.string().c_str());
            SetCurrentDirectory(dir);

            char current[MAX_PATH + 1];
            GetCurrentDirectory(MAX_PATH + 1, current);
        }
#endif

        // [NOTE] this currently relies on a post-build batch script to create correct locations
        m_TriangleVertexShader   = loadShader("shaders/triangle.vert.spv");
        m_TriangleFragmentShader = loadShader("shaders/triangle.frag.spv");
        m_TrianglePipelineLayout = createPipelineLayout();
        
        m_TrianglePipeline = createSimpleGraphicsPipeline(
            *m_TriangleVertexShader,
            *m_TriangleFragmentShader,
            *m_TrianglePipelineLayout
        );
    }

    void Context::update() {
        if (m_Window) {
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

            // present an image
            if (m_Swapchain) {
                uint32_t imageIndex = 0;

                imageIndex = m_Device->acquireNextImageKHR(
                    *m_Swapchain, 
                    std::numeric_limits<uint64_t>::max(), // timeout
                    *m_ImageAvailableSemaphore, 
                    vk::Fence()
                ).value;

                m_Device->resetCommandPool(*m_CommandPool, vk::CommandPoolResetFlags());

                {
                    // one time command for clearing the image
                    vk::CommandBufferBeginInfo cmdInfo;

                    cmdInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

                    m_GraphicsCommands->begin(cmdInfo);
                    
                    vk::RenderPassBeginInfo passBeginInfo;

                    vk::ClearColorValue ccv;
                    ccv.setFloat32({ 0.2f, 0.0f, 0.0f, 1.0f }); // this is in RGBA format

                    vk::ClearDepthStencilValue cdsv;
                    cdsv
                        .setDepth(0.0f)
                        .setStencil(0);

                    vk::ClearValue clearValue;
                    clearValue
                        .setDepthStencil(cdsv)
                        .setColor(ccv);

                    passBeginInfo
                        .setFramebuffer    (*m_Framebuffers[imageIndex])
                        .setRenderArea     (vk::Rect2D(
                            { 0, 0 }, 
                            { m_Window->getWidth(),
                              m_Window->getHeight() 
                            } 
                        ))
                        .setClearValueCount(1)
                        .setPClearValues   (&clearValue)
                        .setRenderPass     (*m_Renderpass);

                    m_GraphicsCommands->beginRenderPass(passBeginInfo, vk::SubpassContents());

                    // NOTE this assumes the pipeline has dynamic state flags for scissor and viewport
                    vk::Rect2D scissor = {
                        { 0, 0 },
                        {
                            m_Window->getWidth(),
                            m_Window->getHeight()
                        }
                    };

                    vk::Viewport viewport;
                    viewport
                        .setX     (0)
                        .setY     (0)
                        .setWidth (static_cast<float>(m_Window->getWidth()))
                        .setHeight(static_cast<float>(m_Window->getHeight()));

                    m_GraphicsCommands->setScissor(0, 1, &scissor);
                    m_GraphicsCommands->setViewport(0, 1, &viewport);

                    m_GraphicsCommands->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_TrianglePipeline);

                    // draw calls go here
                    m_GraphicsCommands->draw(
                        3, // vertex count
                        1, // instance count
                        0, // first vertex
                        0  // first instance
                    );

                    m_GraphicsCommands->end();
                }

                {
                    vk::SubmitInfo info;

                    vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

                    info
                        .setWaitSemaphoreCount  (1)
                        .setPWaitSemaphores     (&*m_ImageAvailableSemaphore)
                        .setPWaitDstStageMask   (&stageMask)
                        .setCommandBufferCount  (1)
                        .setPCommandBuffers     (&*m_GraphicsCommands)
                        .setSignalSemaphoreCount(1)
                        .setPSignalSemaphores   (&*m_PresentCompletedSemaphore);

                    m_GraphicsQueue.submit({ info }, {});
                }

                {
                    vk::PresentInfoKHR info;

                    info
                        .setSwapchainCount    (1)
                        .setPSwapchains       (&*m_Swapchain)
                        .setPImageIndices     (&imageIndex)
                        .setWaitSemaphoreCount(1)
                        .setPWaitSemaphores   (&*m_PresentCompletedSemaphore);

                    m_GraphicsQueue.presentKHR(info);
                }

                m_Device->waitIdle();
            }
        }
    }

    void Context::shutdown() {
        System::shutdown();

        if (m_Device)
            m_Device->waitIdle();

        m_Window.reset();
    }

    void Context::unittest() {
    }

    void Context::close(Window* w) {
        if (w == m_Window.get())
            m_Window.reset();
    }

    vk::Instance Context::getInstance() const {
        return *m_Instance;
    }

    vk::PhysicalDevice Context::getPhysicalDevice() const {
        return m_PhysicalDevice;
    }

    vk::Device Context::getDevice() const {
        return *m_Device;
    }

    Context::Window* Context::createWindow(
        int  width,
        int  height,
        bool windowed,
        int  displayDevice
    ) {
        m_Window = std::make_unique<Window>(
            width,
            height,
            windowed,
            displayDevice,
            this
        );

        return m_Window.get();
    }

    void Context::initVulkan() {
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
        // https://vulkan.lunarg.com/doc/sdk/1.1.92.1/windows/validation_layers.html
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

            m_Instance = vk::createInstanceUnique(info);
        }

#ifdef DJINN_DEBUG
        {
            // install vulkan debug callback
            pfn_vkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT) m_Instance->getProcAddr("vkCreateDebugReportCallbackEXT");
            pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)m_Instance->getProcAddr("vkDestroyDebugReportCallbackEXT");

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

            m_DebugReportCallback = m_Instance->createDebugReportCallbackEXTUnique(info);
        }
#endif
    }

    void Context::selectPhysicalDevice() {
        gLogDebug << "Available physical devices: ";
        auto available_physical_devices = m_Instance->enumeratePhysicalDevices();

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
                << "\n\tDriver version: " 
                    << VK_VERSION_MAJOR(props.driverVersion) << "."
                    << VK_VERSION_MINOR(props.driverVersion) << "."
                    << VK_VERSION_PATCH(props.driverVersion)
                << "\n\tAPI version: " 
                    << VK_VERSION_MAJOR(props.apiVersion) << "."
                    << VK_VERSION_MINOR(props.apiVersion) << "."
                    << VK_VERSION_PATCH(props.apiVersion);
        }

        // we *require*:
        // - extensions (swapchain etc)
        // - a graphics queue
        // - presentation support
        // - vulkan api 1.1
        std::vector<const char*> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        auto getFamilyIdx = [](const vk::PhysicalDevice& pd, vk::QueueFlagBits family) {
            auto props = pd.getQueueFamilyProperties();

            for (uint32_t i = 0; i < props.size(); ++i)
                if ((props[i].queueFlags & family) == family)
                    return i;

            return NOT_FOUND;
        };

        auto canPresent = [](const vk::PhysicalDevice& pd, uint32_t graphicsFamily) {
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
            return !!(pd.getWin32PresentationSupportKHR(graphicsFamily));
#else
            return true; // TODO figure out what this is supposed to be on different platforms
#endif
        };

        auto hasRequirements = [=](const vk::PhysicalDevice& device) {
            auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
            
            for (const auto& requirement : requiredDeviceExtensions)
                if (!isExtensionAvailable(requirement, availableDeviceExtensions))
                    return false;

            uint32_t graphicsFamily = getFamilyIdx(device, vk::QueueFlagBits::eGraphics);
            if (graphicsFamily == NOT_FOUND)
                return false;

            if (!canPresent(device, graphicsFamily))
                return false;

            return true;
        };
        
        // look through the list, prefer a discrete GPU but accept anything as
        // a fallback
        vk::PhysicalDevice preferred;
        vk::PhysicalDevice fallback;

        for (const auto& dev : available_physical_devices) {
            if (hasRequirements(dev)) {
                auto props = dev.getProperties();

                if (!preferred && props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                    preferred = dev;

                if (!fallback)
                    fallback = dev;
            }

            if (preferred && fallback)
                break;
        }

        if (fallback)
            m_PhysicalDevice = fallback;

        if (preferred)
            m_PhysicalDevice = preferred;

        if (!m_PhysicalDevice)
            throw std::runtime_error("No suitable physical device was found");
        else
            gLog << "Selected physical device: " << m_PhysicalDevice.getProperties().deviceName;

        m_GraphicsFamilyIdx = getFamilyIdx(m_PhysicalDevice, vk::QueueFlagBits::eGraphics);
    }

    void Context::createDevice() {
        float queuePriorities = 1.0f;

        vk::DeviceQueueCreateInfo queueInfo;

        queueInfo
            .setQueueFamilyIndex(m_GraphicsFamilyIdx)
            .setQueueCount      (1)
            .setPQueuePriorities(&queuePriorities);

        std::vector<const char*> requiredExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::DeviceCreateInfo devInfo;

        devInfo
            .setQueueCreateInfoCount   (1)
            .setPQueueCreateInfos      (&queueInfo)
            .setEnabledExtensionCount  (static_cast<uint32_t>(requiredExtensions.size()))
            .setPpEnabledExtensionNames(requiredExtensions.data());

        m_Device = m_PhysicalDevice.createDeviceUnique(devInfo);
    }

    void Context::createSurface() {
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
        vk::Win32SurfaceCreateInfoKHR info;

        info
            .setHinstance(GetModuleHandle(nullptr))
            .setHwnd     (m_Window->getHandle());

        m_Surface = m_Instance->createWin32SurfaceKHRUnique(info);
#else
    #error Unsupported platform
#endif
        // make sure the surface is supported by the current physical device
        if (!m_PhysicalDevice.getSurfaceSupportKHR(m_GraphicsFamilyIdx, *m_Surface))
            throw std::runtime_error("Physical device does not support the created surface");
    }

    void Context::selectSwapchainFormat() {
        auto formats = m_PhysicalDevice.getSurfaceFormatsKHR(*m_Surface);

        // prefer 32-bits BGRA unorm, or RGBA unorm
        // if neither is available, pick the first one in the supported set
        if (
            (formats.size() == 1) &&
            (formats[0].format == vk::Format::eUndefined)
        )
            m_SwapchainFormat = vk::Format::eB8G8R8A8Unorm;
        else {
            for (const auto& fmt : formats) {
                if (fmt.format == vk::Format::eB8G8R8A8Unorm) {
                    m_SwapchainFormat = vk::Format::eB8G8R8A8Unorm;
                    return;
                }

                if (fmt.format == vk::Format::eR8G8B8A8Unorm) {
                    m_SwapchainFormat = vk::Format::eR8G8B8A8Unorm;
                    return;
                }
            }
        }

        m_SwapchainFormat = formats[0].format;
    }

    void Context::createSwapchain() {
        vk::SwapchainCreateInfoKHR info;

        auto caps = m_PhysicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
        
        vk::Extent2D extent;

        extent
            .setWidth (m_Window->getWidth())
            .setHeight(m_Window->getHeight());

        info
            .setSurface              (*m_Surface)
            .setMinImageCount        (std::max(caps.minImageCount, 2u)) 
            .setImageFormat          (m_SwapchainFormat)
            .setImageColorSpace      (vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent          (extent)
            .setImageArrayLayers     (1)
            .setImageUsage           (vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode     (vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices  (&m_GraphicsFamilyIdx)
            .setPresentMode          (vk::PresentModeKHR::eFifo)
            .setOldSwapchain         (*m_Swapchain)
            .setCompositeAlpha       (vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPreTransform         (caps.currentTransform);

        m_Swapchain = m_Device->createSwapchainKHRUnique(info);

        m_SwapchainImages = m_Device->getSwapchainImagesKHR(*m_Swapchain);

        // views for all of the swapchain images
        for (const auto& img : m_SwapchainImages) {
            vk::ImageSubresourceRange range;

            range
                .setAspectMask    (vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount    (1)
                .setBaseMipLevel  (0)
                .setLevelCount    (1);

            vk::ImageViewCreateInfo view_info;

            view_info
                .setImage           (img)
                .setViewType        (vk::ImageViewType::e2D)
                .setFormat          (m_SwapchainFormat)
                .setSubresourceRange(range);

            m_SwapchainViews.push_back(m_Device->createImageViewUnique(view_info));
        }

        // framebuffers for all of the swapchain images
        for (const auto& view : m_SwapchainViews) {
            vk::FramebufferCreateInfo fb_info;

            fb_info
                .setWidth          (m_Window->getWidth())
                .setHeight         (m_Window->getHeight())
                .setLayers         (1)
                .setAttachmentCount(1)
                .setPAttachments   (&*view)
                .setRenderPass     (*m_Renderpass);

            m_Framebuffers.push_back(m_Device->createFramebufferUnique(fb_info));
        }
    }

    void Context::createCommandPool() {
        {
            vk::CommandPoolCreateInfo info;

            info
                .setQueueFamilyIndex(m_GraphicsFamilyIdx)
                .setFlags           (vk::CommandPoolCreateFlagBits::eTransient);

            m_CommandPool = m_Device->createCommandPoolUnique(info);
        }

        {
            vk::CommandBufferAllocateInfo info;

            info
                .setCommandPool       (*m_CommandPool)
                .setCommandBufferCount(1)
                .setLevel             (vk::CommandBufferLevel::ePrimary);

            // NOTE allocating a buffer yields a vector
            m_GraphicsCommands = std::move(m_Device->allocateCommandBuffersUnique(info).front());
        }
    }

    vk::UniqueRenderPass Context::createRenderpass() const {
        // for now, just use 1 subpass in a renderpass
        vk::AttachmentDescription attachment;
            
        attachment
            .setFormat        (m_SwapchainFormat)
            .setSamples       (vk::SampleCountFlagBits::e1)
            .setLoadOp        (vk::AttachmentLoadOp::eClear)
            .setStoreOp       (vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp (vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout (vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout   (vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference attRefs;
        attRefs
            .setAttachment(0)
            .setLayout    (vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpassDesc;

        subpassDesc
            .setPipelineBindPoint   (vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments   (&attRefs);

        vk::RenderPassCreateInfo rp_info;

        rp_info
            .setAttachmentCount(1)
            .setPAttachments   (&attachment)
            .setSubpassCount   (1)
            .setPSubpasses     (&subpassDesc);
        
        return m_Device->createRenderPassUnique(rp_info);
    }

    vk::UniqueFramebuffer Context::createFramebuffer(
        vk::RenderPass pass,
        vk::ImageView colorView
    ) const {
        vk::ImageView attachments[] = {
            colorView
        };

        vk::FramebufferCreateInfo fb_info;

        fb_info
            .setRenderPass     (pass)
            .setAttachmentCount(util::CountOf(attachments))
            .setPAttachments   (attachments)
            .setWidth          (m_Window->getWidth())
            .setHeight         (m_Window->getHeight())
            .setLayers         (1);

        return m_Device->createFramebufferUnique(fb_info);
    }

    vk::UniqueShaderModule Context::loadShader(const std::filesystem::path& p) const {
        std::ifstream in(p.string().c_str(), std::ios::binary);
        if (!in.good())
            throw std::runtime_error("Failed to open file");

        in.seekg(0, std::ios::end);
        auto numBytes = in.tellg();

        std::vector<char> data(numBytes);


        in.seekg(0, std::ios::beg);
        in.read(data.data(), numBytes);

        vk::ShaderModuleCreateInfo info;

        info
            .setCodeSize(numBytes) // NOTE createShaderModuleUnique will fail if this is not a multiple of 4
            .setPCode   (reinterpret_cast<const uint32_t*>(data.data()));

        return m_Device->createShaderModuleUnique(info);
    }

    vk::UniquePipelineLayout Context::createPipelineLayout() const {
        vk::PipelineLayoutCreateInfo info;

        

        return m_Device->createPipelineLayoutUnique(info);
    }

    vk::UniquePipeline Context::createSimpleGraphicsPipeline(
        vk::ShaderModule   vertexShader,
        vk::ShaderModule   fragmentShader,
        vk::PipelineLayout layout
    ) const {
        vk::PipelineShaderStageCreateInfo        stages[2];

        vk::GraphicsPipelineCreateInfo           info;

        // soo many settings to tweak...
        vk::PipelineVertexInputStateCreateInfo   vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        vk::PipelineViewportStateCreateInfo      viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo   multisampleState;
        vk::PipelineDepthStencilStateCreateInfo  depthStencilState;
        vk::PipelineColorBlendStateCreateInfo    colorBlendState;
        vk::PipelineDynamicStateCreateInfo       dynamicState;

        stages[0]
            .setModule(vertexShader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eVertex);

        stages[1]
            .setModule(fragmentShader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eFragment);

        inputAssembly
            .setTopology(vk::PrimitiveTopology::eTriangleList);

        viewportState
            .setScissorCount(1)
            .setViewportCount(1);

        rasterizationState
            .setLineWidth(1.0f);

        multisampleState
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
            );

        colorBlendState
            .setAttachmentCount(1)
            .setPAttachments   (&colorBlendAttachment); // err this should match what was done in the createRenderpass thingie

        vk::DynamicState dynamicStates[] = {
            vk::DynamicState::eScissor,
            vk::DynamicState::eViewport
        };

        dynamicState
            .setDynamicStateCount(util::CountOf(dynamicStates))
            .setPDynamicStates   (dynamicStates);

        info
            .setStageCount         (util::CountOf(stages))
            .setPStages            (stages)
            .setPVertexInputState  (&vertexInputState)
            .setPInputAssemblyState(&inputAssembly)
            .setPViewportState     (&viewportState)
            .setPRasterizationState(&rasterizationState)
            .setPMultisampleState  (&multisampleState)
            .setPDepthStencilState (&depthStencilState)
            .setPColorBlendState   (&colorBlendState)
            .setPDynamicState      (&dynamicState)
            .setLayout             (layout)
            .setRenderPass         (*m_Renderpass);

        return m_Device->createGraphicsPipelineUnique(*m_PipelineCache, info);
    }
}