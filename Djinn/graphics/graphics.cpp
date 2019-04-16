#include "graphics.h"
#include "extensions.h"
#include "core/engine.h"
#include "util/flat_map.h"
#include "util/algorithm.h"

#include <fstream>

// ------ Vulkan debug reports ------
namespace {
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
              VkDebugReportObjectTypeEXT /* objectType */,
              uint64_t                   /* object     */,
              size_t                     /* location   */,
              int32_t                    code,
        const char*                      layerPrefix,
        const char*                      message,
              void*                      /* userdata   */
    ) {
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

            const uint32_t idx = static_cast<uint32_t>(std::distance(availableFamilies.begin(), it));
            
            result.assign(type, idx);
        }

        return result;
    }

    vk::Format selectSwapchainFormat(vk::PhysicalDevice physical, vk::SurfaceKHR surface) {
        auto formats = physical.getSurfaceFormatsKHR(surface);

        if (formats.empty())
            throw std::runtime_error("No formats are supported for presentation");

        // prefer 32-bits BGRA unorm, or RGBA unorm
        // if neither is available, pick the first one in the supported set
        if (
            (formats.size() == 1) &&
            (formats[0].format == vk::Format::eUndefined)
        )
            // this is a corner case, if only 1 undefined format is reported actually all formats are supported
            return vk::Format::eB8G8R8A8Unorm;
        else {
            // prefer either BGRA32 or RGBA32 formats
            for (const auto& fmt : formats) {
                if (fmt.format == vk::Format::eB8G8R8A8Unorm)
                    return vk::Format::eB8G8R8A8Unorm;

                if (fmt.format == vk::Format::eR8G8B8A8Unorm)
                    return vk::Format::eR8G8B8A8Unorm;
            }
        }

        // if none of the preferred formats is available, pick the first one
        return formats[0].format;
    }

    vk::Format selectDepthFormat(vk::PhysicalDevice gpu) {
        // see if one of the depth formats is optimal, and if so pick that one
        std::array<vk::Format, 5> formats = {
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD32Sfloat,
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD16UnormS8Uint,
            vk::Format::eD16Unorm
        };

        for (auto fmt : formats) {
            auto props = gpu.getFormatProperties(fmt);

            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                return fmt;
        }

        // fallback to some common format
        return vk::Format::eD24UnormS8Uint;
    }

    uint32_t selectMemoryTypeIndex(
        vk::PhysicalDevice      gpu,
        uint32_t                typeBits,
        vk::MemoryPropertyFlags properties
    ) {
        auto props = gpu.getMemoryProperties();

        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if ((typeBits & 1) == 1) {
                if ((props.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }

            typeBits >>= 1; // NOTE not entirely sure about this
        }

        return 0;
    }
}

namespace djinn {
    Graphics::Graphics() :
        System("Graphics")
    {
        registerSetting("Width",         &m_MainWindowSettings.m_Width);
        registerSetting("Height",        &m_MainWindowSettings.m_Height);
        registerSetting("Windowed",      &m_MainWindowSettings.m_Windowed);
        registerSetting("DisplayDevice", &m_MainWindowSettings.m_DisplayDevice);
    }

    void Graphics::init() {
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

        auto swapchainFormat = selectSwapchainFormat(m_PhysicalDevice, *m_Surface);
        auto depthFormat     = selectDepthFormat(m_PhysicalDevice);

        m_Renderpass = createSimpleRenderpass(swapchainFormat, depthFormat); // depends on the swapchain format                
        
        createDepthImage(depthFormat);

        m_Swapchain = std::make_unique<Swapchain>(
            *m_Device,
             m_PhysicalDevice,
            *m_Surface,
             swapchainFormat,
            *m_DepthView,
             m_GraphicsFamilyIdx,
            *m_Renderpass
        );

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
#else
    #error Unsupported platform
#endif

        // [NOTE] this currently relies on a post-build batch script to create correct locations
        m_TriangleVertexShader   = loadShader("assets/shaders/triangle.vert.spv");
        m_TriangleFragmentShader = loadShader("assets/shaders/triangle.frag.spv");
		m_TrianglePipelineLayout = m_Device->createPipelineLayoutUnique({});
        
        m_TrianglePipeline = createSimpleGraphicsPipeline(
            *m_TriangleVertexShader,
            *m_TriangleFragmentShader,
            *m_TrianglePipelineLayout
        );
    }

    void Graphics::update() {
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

            if (!m_Window)
                return;

            // present an image
            if (m_Swapchain) {
                if (m_Swapchain->getExtent() != vk::Extent2D(m_Window->getWidth(), m_Window->getHeight()))
                    m_Swapchain = std::make_unique<Swapchain>(
                         *m_Device,
                          m_PhysicalDevice,
                         *m_Surface,
                          m_Swapchain->getImageFormat(),
                         *m_DepthView,
                          m_GraphicsFamilyIdx,
                         *m_Renderpass,
                        &*m_Swapchain
                    );

                uint32_t imageIndex = m_Swapchain->acquireNextImage(*m_Device);

                m_Device->resetCommandPool(*m_CommandPool, vk::CommandPoolResetFlags());

                {
                    // one time command for clearing the image
                    vk::CommandBufferBeginInfo cmdInfo;

                    cmdInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

                    m_GraphicsCommands->begin(cmdInfo);
                    
                    vk::RenderPassBeginInfo passBeginInfo;

                    // set access to 'write', set layout to color attachment
					auto renderBeginBarrier = m_Swapchain->imageBarrier(
						imageIndex,
						vk::AccessFlagBits::eMemoryRead,
						vk::ImageLayout   ::eUndefined,
						vk::AccessFlagBits::eColorAttachmentWrite,
						vk::ImageLayout   ::eColorAttachmentOptimal
					);

					m_GraphicsCommands->pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::DependencyFlagBits(),
						0,                    // memory barrier count
						nullptr,              // pMemoryBarriers
						0,                    // buffer memory barriers
						nullptr,              // pBufferMemoryBarriers
						1,			          // image memory barriers
						&renderBeginBarrier   // pImageMemoryBarriers
					);
                    
                    // assemble a clear value

                    vk::ClearValue      clearValues[] = {
                        { vk::ClearColorValue(std::array<float, 4>{ 0.2f, 0.0f, 0.0f, 1.0f }) }, // this is in RGBA format
                        { vk::ClearDepthStencilValue(0.0f, 0)                                 }  // assuming f32s8 here
                    };
                    
                    // indicate the correct framebuffer, clear value and render area
                    passBeginInfo
                        .setFramebuffer    (m_Swapchain->getFramebuffer(imageIndex))
                        .setRenderArea     (vk::Rect2D(
                            { 0, 0 }, 
                            { m_Window->getWidth(),
                              m_Window->getHeight() 
                            } 
                        ))
                        .setClearValueCount(2)
                        .setPClearValues   (clearValues)
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

                    // NOTE vulkan uses an unusual screenspace coordinate system, here it is flipped to match openGL/DirectX style
                    vk::Viewport viewport;
                    viewport
                        .setX     (0)
                        .setY     ( static_cast<float>(m_Window->getHeight()))
                        .setWidth ( static_cast<float>(m_Window->getWidth ()))
                        .setHeight(-static_cast<float>(m_Window->getHeight()));

                    m_GraphicsCommands->setScissor (0, 1, &scissor);
                    m_GraphicsCommands->setViewport(0, 1, &viewport);

                    m_GraphicsCommands->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_TrianglePipeline);

                    // draw calls go here
                    m_GraphicsCommands->draw(
                        3, // vertex count
                        1, // instance count
                        0, // first vertex
                        0  // first instance
                    );

					m_GraphicsCommands->endRenderPass();

					auto renderEndBarrier = m_Swapchain->imageBarrier(
						imageIndex,
						vk::AccessFlagBits::eColorAttachmentWrite,
						vk::ImageLayout   ::eColorAttachmentOptimal,
						vk::AccessFlagBits::eMemoryRead,
						vk::ImageLayout   ::ePresentSrcKHR
					);

					m_GraphicsCommands->pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::DependencyFlagBits(),
						0,                    // memory barrier count
						nullptr,              // pMemoryBarriers
						0,                    // buffer memory barriers
						nullptr,              // pBufferMemoryBarriers
						1,			          // image memory barriers
						&renderEndBarrier     // pImageMemoryBarriers
					);

                    m_GraphicsCommands->end();
                }

                m_Swapchain->present(
                    *m_Device,
                     m_GraphicsQueue,
                    *m_GraphicsCommands
                );
            }
        }
    }

    void Graphics::shutdown() {
        System::shutdown();

        if (m_Device)
            m_Device->waitIdle();

        m_Window.reset();
    }

    void Graphics::unittest() {
    }

    void Graphics::close(Window* w) {
        if (w == m_Window.get())
            m_Window.reset();
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

	Graphics::Window* Graphics::createWindow(
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

    void Graphics::initVulkan() {
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

			graphics::loadInstanceExtensions(*m_Instance);
        }

#ifdef DJINN_DEBUG
        {
            // install vulkan debug callback
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

    void Graphics::selectPhysicalDevice() {
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
    #error Unsupported platform
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

    void Graphics::createDevice() {
        float queuePriorities = 1.0f; // range is [0..1], with 1.0 being the highest priority

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

    void Graphics::createSurface() {
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
    
    

    void Graphics::createCommandPool() {
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

    vk::UniqueRenderPass Graphics::createSimpleRenderpass(
        vk::Format imageFormat, 
        vk::Format depthFormat
    ) const {
        // for now, just use 1 subpass in a renderpass
        std::vector<vk::AttachmentDescription> attachments;

        vk::AttachmentDescription colorAttachment;
            
        colorAttachment
            .setFormat        (imageFormat)
            .setSamples       (vk::SampleCountFlagBits::e1)
            .setLoadOp        (vk::AttachmentLoadOp::eClear)
            .setStoreOp       (vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp (vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout (vk::ImageLayout::eUndefined)
            .setFinalLayout   (vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentDescription depthAttachment;

        depthAttachment
            .setFormat        (depthFormat)
            .setSamples       (vk::SampleCountFlagBits::e1)
            .setLoadOp        (vk::AttachmentLoadOp::eClear)
            .setStoreOp       (vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp (vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout (vk::ImageLayout::eUndefined)
            .setFinalLayout   (vk::ImageLayout::eDepthStencilAttachmentOptimal);

        attachments.push_back(std::move(colorAttachment));
        attachments.push_back(std::move(depthAttachment));

        vk::AttachmentReference colorRefs;
        colorRefs
            .setAttachment(0)
            .setLayout    (vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference depthRefs;
        depthRefs
            .setAttachment(1)
            .setLayout    (vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpassDesc;

        subpassDesc
            .setPipelineBindPoint      (vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount   (1)
            .setPColorAttachments      (&colorRefs)
            .setPDepthStencilAttachment(&depthRefs);

        // list which subpasses can access which other subpasses
        // NOTE - not sure about this, should check what this does exactly
        std::vector<vk::SubpassDependency> dependencies; 

        vk::SubpassDependency bottom, top;
        
        bottom
            .setSrcSubpass     (~0U)
            .setDstSubpass     (0)
            .setSrcStageMask   (vk::PipelineStageFlagBits::eBottomOfPipe)
            .setDstStageMask   (vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask  (vk::AccessFlagBits::eMemoryRead)
            .setDstAccessMask  (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

        top
            .setSrcSubpass     (0)
            .setDstSubpass     (~0U)
            .setSrcStageMask   (vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask   (vk::PipelineStageFlagBits::eBottomOfPipe)
            .setSrcAccessMask  (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask  (vk::AccessFlagBits::eMemoryRead)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

        dependencies.push_back(std::move(bottom));
        dependencies.push_back(std::move(top));

        vk::RenderPassCreateInfo rp_info;

        rp_info
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments   (attachments.data())
            .setSubpassCount   (1)
            .setPSubpasses     (&subpassDesc)
            .setDependencyCount(static_cast<uint32_t>(dependencies.size()))
            .setPDependencies  (dependencies.data());
        
        return m_Device->createRenderPassUnique(rp_info);
    }

    void Graphics::createDepthImage(vk::Format depthFormat) {
        vk::ImageCreateInfo imageInfo;

        imageInfo
            .setImageType            (vk::ImageType::e2D)
            .setFormat               (depthFormat)
            .setExtent               ({m_Window->getWidth(), m_Window->getHeight(), 1 }) // TODO this suggests it is sensitive to resizing...
            .setArrayLayers          (1)
            .setMipLevels            (1)
            .setSamples              (vk::SampleCountFlagBits::e1)
            .setTiling               (vk::ImageTiling::eOptimal)
            .setUsage                (vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc)
            .setSharingMode          (vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices  (&m_GraphicsFamilyIdx)
            .setInitialLayout        (vk::ImageLayout::eUndefined);

        m_DepthImage = m_Device->createImageUnique(imageInfo);

        auto req = m_Device->getImageMemoryRequirements(*m_DepthImage);

        vk::MemoryAllocateInfo allocInfo;

        allocInfo
            .setAllocationSize(req.size)
            .setMemoryTypeIndex(
                selectMemoryTypeIndex(
                    m_PhysicalDevice, 
                    req.memoryTypeBits, 
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                )
            );

        m_DepthBuffer = m_Device->allocateMemoryUnique(allocInfo);
        m_Device->bindImageMemory(
            *m_DepthImage, 
            *m_DepthBuffer, 
            0 // offset
        );

        vk::ImageViewCreateInfo viewInfo;
        vk::ImageSubresourceRange range;

        range
            .setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)
            .setBaseArrayLayer(0)
            .setLayerCount    (1)
            .setBaseMipLevel  (0)
            .setLevelCount    (1);

        viewInfo
            .setImage           (*m_DepthImage)
            .setViewType        (vk::ImageViewType::e2D)
            .setFormat          (depthFormat)
            .setSubresourceRange(range);

        m_DepthView = m_Device->createImageViewUnique(viewInfo);
    }

    vk::UniqueShaderModule Graphics::loadShader(const std::filesystem::path& p) const {
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

    vk::UniquePipeline Graphics::createSimpleGraphicsPipeline(
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

        vk::PipelineColorBlendAttachmentState    colorBlendAttachment;

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