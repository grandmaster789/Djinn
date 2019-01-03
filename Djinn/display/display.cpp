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

                auto& mainWindow = m_Windows.front();
                auto imageIndex  = m_VkDevice->acquireNextImageKHR(
                    *mainWindow->m_SwapChain,
                    std::numeric_limits<uint64_t>::max(), 
                    *m_ImageAvailable,
                    {} // fence
                );

                vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

                vk::SubmitInfo submitInfo;
                submitInfo
                    .setWaitSemaphoreCount  (1)
                    .setPWaitSemaphores     (&*m_ImageAvailable)
                    .setPWaitDstStageMask   (& waitStageMask)
                    .setCommandBufferCount  (1)
                    .setPCommandBuffers     (&*m_CommandBuffers[imageIndex.value])
                    .setSignalSemaphoreCount(1)
                    .setPSignalSemaphores   (&*m_RenderCompleted);

                m_GraphicsQueue.submit(
                    submitInfo, 
                    {} // fence
                );

                auto presentInfo = vk::PresentInfoKHR{ 
                    1, 
                    &*m_RenderCompleted,
                    1,
                    &*mainWindow->m_SwapChain, 
                    &imageIndex.value 
                };

                m_PresentQueue.presentKHR(presentInfo);

                m_VkDevice->waitIdle();
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
            // [TODO] maybe prefer separate families if possible?
            auto selectedFamilies = scanFamilies(
                availableQueueFamilies,
                { 
                    vk::QueueFlagBits::eGraphics,
                    vk::QueueFlagBits::eTransfer
                }
            );

            m_GraphicsFamilyIdx = *selectedFamilies[vk::QueueFlagBits::eGraphics];
            m_TransferFamilyIdx = *selectedFamilies[vk::QueueFlagBits::eTransfer];

            // right now we only have either 1 family for both queue types
            // or 2 families... when we get more, this should be revisited
            uint32_t numQueueInfos;

            uint32_t drawQueueIdx;
            uint32_t transferQueueIdx;
            uint32_t presentQueueIdx;

            if (m_GraphicsFamilyIdx == m_TransferFamilyIdx) {
                // 1 family supports both queue types
				// [NOTE] my laptop (Intel UHD 620) supports just one queue with one queue family. All types though...
				uint32_t queueCount = 2;

                numQueueInfos    = 1;                
                drawQueueIdx     = 0;
                transferQueueIdx = 1;

				if (availableQueueFamilies.size() == 1) {
					// check if the one available queue family has enough queues available
					// not sure if this is the general case
					if (availableQueueFamilies.back().queueCount < 2) {
						queueCount = 1;
						transferQueueIdx = 0;
					}
				}

				queue_info[0]
					.setQueueCount(queueCount)
					.setQueueFamilyIndex(m_GraphicsFamilyIdx)
					.setPQueuePriorities(priorities);
            }
            else {
                // separate families per queue type
                numQueueInfos    = 2;
                drawQueueIdx     = 0;
                transferQueueIdx = 0;

                queue_info[0]
                    .setQueueCount(1)
                    .setQueueFamilyIndex(m_GraphicsFamilyIdx)
                    .setPQueuePriorities(&priorities[0]);

                queue_info[1]
                    .setQueueCount(1)
                    .setQueueFamilyIndex(m_TransferFamilyIdx)
                    .setPQueuePriorities(&priorities[1]);
            }

            vk::DeviceCreateInfo info;

            info
                .setQueueCreateInfoCount   (numQueueInfos)
                .setPQueueCreateInfos      (queue_info)
                .setEnabledLayerCount      ((uint32_t)requiredDeviceLayers.size())
                .setPpEnabledLayerNames    (          requiredDeviceLayers.data())
                .setEnabledExtensionCount  ((uint32_t)requiredDeviceExtensions.size())
                .setPpEnabledExtensionNames(          requiredDeviceExtensions.data());

            m_VkDevice = m_VkPhysicalDevice.createDeviceUnique(info);

            m_PresentFamilyIdx = m_GraphicsFamilyIdx;
            presentQueueIdx = drawQueueIdx;

            m_GraphicsQueue = m_VkDevice->getQueue(m_GraphicsFamilyIdx, drawQueueIdx);
            m_TransferQueue = m_VkDevice->getQueue(m_TransferFamilyIdx, transferQueueIdx);
            m_PresentQueue  = m_VkDevice->getQueue(m_PresentFamilyIdx,  presentQueueIdx);
        }

        // assemble vtx + frag shaders
        // for now, embed triangle vertex data in the shader itself
        {
            std::string vtxShaderSource = R"(
                #version 450
                #extension GL_ARB_separate_shader_objects: enable

                out gl_PerVertex {
                    vec4 gl_Position;
                };

                layout(location = 0) out vec3 fragColor;
            
                vec2 positions[3] = vec2[](
                    vec2( 0.0, -0.5),
                    vec2( 0.5,  0.5),
                    vec2(-0.5,  0.5)
                );

                vec3 colors[3] = vec3[](
                    vec3(1.0, 0.0, 0.0),
                    vec3(0.0, 1.0, 0.0),
                    vec3(0.0, 0.0, 1.0)
                );

                void main() {
                    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
                    fragColor   = colors[gl_VertexIndex];
                }
            )";

            std::string fragShaderSource = R"(
                #version 450
                #extension GL_ARB_separate_shader_objects: enable

                layout(location = 0) in vec3 fragColor;
                layout(location = 0) out vec4 outColor;

                void main() {
                    outColor = vec4(fragColor, 1.0);
                }
            )";

            shaderc::Compiler compiler;
            shaderc::CompileOptions options;

            options.SetOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_performance);

            auto vtxShaderResult = compiler.CompileGlslToSpv(
                vtxShaderSource, 
                shaderc_shader_kind::shaderc_vertex_shader, 
                "Embedded vertex shader"
            );

            if (vtxShaderResult.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
                gLogError << "Failed to compile vertex shader: " << vtxShaderResult.GetErrorMessage();
                return;
            }
            
            std::vector<uint32_t> vtxShaderSpirv(
                vtxShaderResult.cbegin(),
                vtxShaderResult.cend()
            );

            vk::ShaderModuleCreateInfo vtxShaderInfo;
            vtxShaderInfo
                .setCodeSize(vtxShaderSpirv.size() * sizeof(uint32_t))
                .setPCode   (vtxShaderSpirv.data());

            m_VertexShader = m_VkDevice->createShaderModuleUnique(vtxShaderInfo);

            auto fragShaderResult = compiler.CompileGlslToSpv(
                fragShaderSource,
                shaderc_shader_kind::shaderc_fragment_shader,
                "Embedded fragment shader"
            );

            if (fragShaderResult.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
                gLogError << "Failed to compile fragment shader: " << fragShaderResult.GetErrorMessage();
                return;
            }

            std::vector<uint32_t> fragShaderSpirv(
                fragShaderResult.cbegin(),
                fragShaderResult.cend()
            );

            vk::ShaderModuleCreateInfo fragShaderInfo;
            fragShaderInfo
                .setCodeSize(fragShaderSpirv.size() * sizeof(uint32_t))
                .setPCode   (fragShaderSpirv.data());

            m_FragmentShader = m_VkDevice->createShaderModuleUnique(fragShaderInfo);
        }

        // set up a pipeline
        {
            vk::PipelineShaderStageCreateInfo vtxStageInfo;
            vk::PipelineShaderStageCreateInfo fragStageInfo;
            
            vtxStageInfo
                .setStage (vk::ShaderStageFlagBits::eVertex)
                .setModule(*m_VertexShader)
                .setPName ("main");

            fragStageInfo
                .setStage (vk::ShaderStageFlagBits::eFragment)
                .setModule(*m_FragmentShader)
                .setPName ("main");

            std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = {
                vtxStageInfo,
                fragStageInfo
            };
            
            vk::PipelineVertexInputStateCreateInfo vtxInputInfo;

            vtxInputInfo
                .setVertexAttributeDescriptionCount(0)
                .setPVertexAttributeDescriptions   (nullptr)
                .setVertexBindingDescriptionCount  (0)
                .setPVertexBindingDescriptions     (nullptr);

            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;

            inputAssemblyInfo
                .setTopology              (vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable(false);

            const auto& mainWindow = m_Windows.front();

            uint32_t width  = mainWindow->getWidth();
            uint32_t height = mainWindow->getHeight();

            vk::Viewport viewport(
                0.0f,                       // x
                0.0f,                       // y
                static_cast<float>(width),  // width
                static_cast<float>(height), // height
                0.0f,                       // mindepth
                1.0f                        // maxdepth 
            );

            vk::Rect2D scissor(
                { 0, 0 }, 
                { width, height }
            );

            vk::PipelineViewportStateCreateInfo viewportStateInfo;
            viewportStateInfo
                .setScissorCount (1)
                .setPScissors    (&scissor)
                .setViewportCount(1)
                .setPViewports   (&viewport);

            vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
            rasterizerInfo
                .setDepthClampEnable       (false)
                .setRasterizerDiscardEnable(false)
                .setPolygonMode            (vk::PolygonMode::eFill)
                .setFrontFace              (vk::FrontFace::eCounterClockwise)
                .setLineWidth              (1.0f);

            vk::PipelineMultisampleStateCreateInfo multisamplingInfo;
            multisamplingInfo
                .setRasterizationSamples (vk::SampleCountFlagBits::e1)
                .setAlphaToOneEnable     (false)
                .setAlphaToCoverageEnable(false)
                .setMinSampleShading     (1.0f);

            vk::PipelineColorBlendAttachmentState colorBlendAttachmentInfo;
            colorBlendAttachmentInfo
                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eZero)
                .setColorBlendOp       (vk::BlendOp::eAdd)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                .setAlphaBlendOp       (vk::BlendOp::eAdd)
                .setColorWriteMask(
                    vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA
                );

            vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
            colorBlendInfo
                .setLogicOpEnable  (false)
                .setLogicOp        (vk::LogicOp::eCopy)
                .setAttachmentCount(1)
                .setPAttachments   (&colorBlendAttachmentInfo);

            m_PipelineLayout = m_VkDevice->createPipelineLayoutUnique({});

            vk::AttachmentDescription colorAttachmentDesc;
            colorAttachmentDesc
                .setFormat       (vk::Format::eB8G8R8A8Unorm)
                .setSamples      (vk::SampleCountFlagBits::e1)
                .setLoadOp       (vk::AttachmentLoadOp::eClear)
                .setStoreOp      (vk::AttachmentStoreOp::eStore)
                .setFinalLayout  (vk::ImageLayout::ePresentSrcKHR);

            vk::AttachmentReference colorAttachmentRef;
            colorAttachmentRef
                .setAttachment(0)
                .setLayout    (vk::ImageLayout::eColorAttachmentOptimal);

            vk::SubpassDescription subpassDesc;
            subpassDesc
                .setPipelineBindPoint   (vk::PipelineBindPoint::eGraphics)
                .setInputAttachmentCount(0)
                .setPInputAttachments   (nullptr)
                .setColorAttachmentCount(1)
                .setPColorAttachments   (&colorAttachmentRef);

            m_ImageAvailable  = m_VkDevice->createSemaphoreUnique({});
            m_RenderCompleted = m_VkDevice->createSemaphoreUnique({});

            vk::SubpassDependency subpassDep;
            subpassDep
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(
                    vk::AccessFlagBits::eColorAttachmentRead |
                    vk::AccessFlagBits::eColorAttachmentWrite
                );
            
            vk::RenderPassCreateInfo renderPassInfo;
            renderPassInfo
                .setAttachmentCount(1)
                .setPAttachments   (&colorAttachmentDesc)
                .setSubpassCount   (1)
                .setPSubpasses     (&subpassDesc);

            m_RenderPass = m_VkDevice->createRenderPassUnique(renderPassInfo);

            vk::GraphicsPipelineCreateInfo graphicsPipelineInfo;
            graphicsPipelineInfo
                .setStageCount         (static_cast<uint32_t>(pipelineStages.size()))
                .setPStages            (pipelineStages.data())
                .setPVertexInputState  (&vtxInputInfo)
                .setPInputAssemblyState(&inputAssemblyInfo)
                .setPTessellationState (nullptr)
                .setPViewportState     (&viewportStateInfo)
                .setPRasterizationState(&rasterizerInfo)
                .setPMultisampleState  (&multisamplingInfo)
                .setPDepthStencilState (nullptr)
                .setPColorBlendState   (&colorBlendInfo)
                .setPDynamicState      (nullptr)
                .setLayout             (*m_PipelineLayout)
                .setRenderPass         (*m_RenderPass)
                .setSubpass            (0);

            m_Pipeline = m_VkDevice->createGraphicsPipelineUnique({}, graphicsPipelineInfo);

            m_FrameBuffers.resize(mainWindow->m_SwapChainImages.size());

            for (size_t i = 0; i < m_FrameBuffers.size(); ++i) {
                vk::FramebufferCreateInfo frameBufferInfo;
                frameBufferInfo
                    .setRenderPass     (*m_RenderPass)
                    .setAttachmentCount(1)
                    .setPAttachments   (&(*mainWindow->m_SwapChainViews[i]))
                    .setWidth          (width)
                    .setHeight         (height)
                    .setLayers         (1);

                m_FrameBuffers[i] = m_VkDevice->createFramebufferUnique(frameBufferInfo);
            }

            vk::CommandPoolCreateInfo commandPoolInfo;
            commandPoolInfo
                .setQueueFamilyIndex(m_GraphicsFamilyIdx);

            m_CommandPool    = m_VkDevice->createCommandPoolUnique(commandPoolInfo);

            vk::CommandBufferAllocateInfo commandBufferInfo;
            commandBufferInfo
                .setCommandPool       (*m_CommandPool)
                .setLevel             (vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(static_cast<uint32_t>(m_FrameBuffers.size()));

            m_CommandBuffers = m_VkDevice->allocateCommandBuffersUnique(commandBufferInfo);

            for (size_t i = 0; i < m_CommandBuffers.size(); ++i) {
                vk::CommandBufferBeginInfo beginInfo;

                m_CommandBuffers[i]->begin(beginInfo);

                vk::ClearValue clearValues;

                vk::RenderPassBeginInfo renderPassBeginInfo;
                renderPassBeginInfo
                    .setRenderPass     (*m_RenderPass)
                    .setFramebuffer    (*m_FrameBuffers[i])
                    .setRenderArea     (vk::Rect2D(
                                            { 0, 0 }, 
                                            { width, height }
                                        ))
                    .setClearValueCount(1)
                    .setPClearValues   (&clearValues);

                m_CommandBuffers[i]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                m_CommandBuffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);
                m_CommandBuffers[i]->draw(3, 1, 0, 0);
                m_CommandBuffers[i]->endRenderPass();
                m_CommandBuffers[i]->end();
            }                
        }
    }
}