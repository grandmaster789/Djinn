#include "graphics.h"
#include "core/engine.h"
#include "extensions.h"
#include "math/trigonometry.h"
#include "swapchain.h"
#include "util/algorithm.h"
#include "util/filesystem.h"
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

        if (!valid)
            throw std::runtime_error("Missing required layer(s)");
    }

    void checkInstanceExtensions(
        const std::vector<const char*>&             requiredExtensions,
        const std::vector<vk::ExtensionProperties>& availableExtensions) {
        using djinn::util::contains_if;

        bool valid = true;

        // log all missing layers
        for (const auto& name : requiredExtensions) {
            std::string s(name);

            if (!contains_if(availableExtensions, [s](const vk::ExtensionProperties& prop) {
                    return (s == prop.extensionName);
                })) {
                valid = false;
                gLogError << "Missing required layer: " << name;
            }
        }

        if (!valid)
            throw std::runtime_error("Missing required layer(s)");
    }
}  // namespace

namespace djinn {
    Graphics::Graphics():
        System("Graphics") {
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

        initLogicalDevice();  // depends on having an output surface
        initUniformBuffer();
        initPipelineLayouts();
        initRenderPass();

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

        initShaders(
            util::loadTextFile("shaders/basic.glsl.vert"),
            util::loadTextFile("shaders/basic.glsl.frag"));
        initFrameBuffers();
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
                if (msg.message == WM_QUIT)
                    m_Engine->stop();

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (m_Windows.empty())
                return;
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

    uint32_t Graphics::getGraphicsFamilyIdx() const {
        return m_GraphicsFamilyIdx;
    }

    uint32_t Graphics::getPresentFamilyIdx() const {
        return m_PresentFamilyIdx;
    }

    vk::SurfaceFormatKHR Graphics::getSurfaceFormat() const {
        return m_SurfaceFormat;
    }

    vk::Format Graphics::getDepthFormat() const {
        return m_DepthFormat;
    }

    Graphics::Window*
        Graphics::createWindow(int width, int height, bool windowed, int displayDevice) {
        m_Windows.push_back(std::make_unique<Window>(width, height, windowed, displayDevice, this));

        if (m_Windows.size() > 1)
            m_Windows.back()->initSwapchain();

        return m_Windows.back().get();
    }

    void Graphics::initVulkan() {
        {
            // setup Instance
            auto availableLayers     = vk::enumerateInstanceLayerProperties();
            auto availableExtensions = vk::enumerateInstanceExtensionProperties();

            // [NOTE] possibly the debug report should be optional

            std::vector<const char*> requiredLayers     = {};
            std::vector<const char*> requiredExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
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
                .setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
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
                    vk::DebugReportFlagBitsEXT::eError
                    //| vk::DebugReportFlagBitsEXT::eInformation // very spammy
                    | vk::DebugReportFlagBitsEXT::eWarning
                    | vk::DebugReportFlagBitsEXT::ePerformanceWarning
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

            gLog << "Driver v" << driver_major << "." << driver_minor << "." << driver_patch;

            auto api_major = VK_VERSION_MAJOR(props.apiVersion);
            auto api_minor = VK_VERSION_MINOR(props.apiVersion);
            auto api_patch = VK_VERSION_PATCH(props.apiVersion);

            gLog << "Vulkan v" << api_major << "." << api_minor << "." << api_patch;

            // fetch memory properties
            m_MemoryProps = m_PhysicalDevice.getMemoryProperties();
        }
    }

    void Graphics::initLogicalDevice() {
        // setup a draw queue, init logical device
        // [TODO] add compute, transfer queues etc
        {
            auto queueFamilyProps = m_PhysicalDevice.getQueueFamilyProperties();
            if (queueFamilyProps.empty())
                throw std::runtime_error("No queue family properties available");

            auto hasFlags = [](const vk::QueueFamilyProperties& prop, vk::QueueFlags flags) {
                return (prop.queueFlags & flags) == flags;
            };

            // [NOTE] there is also a getWin32PresentationSupportKHR, but this seems better
            auto supportsPresent = [this](uint32_t queueFamilyIdx, vk::SurfaceKHR surface) {
                return m_PhysicalDevice.getSurfaceSupportKHR(queueFamilyIdx, surface);
            };

            // try to find a graphics queue family that also supports presenting
            for (uint32_t i = 0; i < queueFamilyProps.size(); ++i) {
                if (hasFlags(queueFamilyProps[i], vk::QueueFlagBits::eGraphics)
                    && supportsPresent(i, getMainWindow()->getSurface())) {
                    m_GraphicsFamilyIdx = i;
                    m_PresentFamilyIdx  = i;
                    break;
                }
            }

            // TODO: fallback when separate queues are required

            if (m_GraphicsFamilyIdx == NOT_FOUND)
                throw std::runtime_error("No graphics queue family was found");

            float priorities[] = {0.0f};

            std::vector<vk::DeviceQueueCreateInfo> queueInfos;
            std::vector<const char*>               deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
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
            info.setQueueFamilyIndex(m_GraphicsFamilyIdx)
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            m_CommandPool = m_Device->createCommandPoolUnique(info);

            vk::CommandBufferAllocateInfo cbai;
            cbai.setCommandBufferCount(1)
                .setCommandPool(*m_CommandPool)
                .setLevel(vk::CommandBufferLevel::ePrimary);

            // [NOTE] allocateCommandBuffersUnique yields a vector...
            m_CommandBuffer = std::move(m_Device->allocateCommandBuffersUnique(cbai)[0]);
        }

        // try and select an appropriate format, colorspace
        // [NOTE] again, we're using the primary window surface here; prefer BGRA32
        {
            auto allFormats = m_PhysicalDevice.getSurfaceFormatsKHR(getMainWindow()->getSurface());

            if (allFormats.empty())
                throw std::runtime_error("No surface formats available");

            // special case -- when the driver reports 1 undefined format it actually supports all of them
            if (allFormats.size() == 1) {
                m_SurfaceFormat = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
            }
            else {
                vk::SurfaceFormatKHR preferred
                    = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};

                if (util::contains(allFormats, preferred))
                    m_SurfaceFormat = preferred;
                else
                    // if the preferred format is not available, just pick the first reported
                    m_SurfaceFormat = allFormats.front();
            }
        }

        // execute initialization commands
        {
            vk::CommandBufferBeginInfo cbbi;

            m_CommandBuffer->begin(cbbi);

            // init device queues
            m_GraphicsQueue = m_Device->getQueue(m_GraphicsFamilyIdx, 0);

            if (m_PresentFamilyIdx == m_GraphicsFamilyIdx)
                m_PresentQueue = m_GraphicsQueue;
            else
                m_PresentQueue = m_Device->getQueue(m_PresentFamilyIdx, 0);

            getMainWindow()->initSwapchain();

            m_CommandBuffer->end();
        }
    }

    void Graphics::initUniformBuffer() {
        // setup common camera matrices
        {
            float fov = glm::radians(45.0f);
            float w   = static_cast<float>(getMainWindow()->getWidth());
            float h   = static_cast<float>(getMainWindow()->getHeight());

            if (w > h)
                fov *= h / w;

            m_Projection = glm::perspective(fov, w / h, 0.1f, 100.0f);
            m_View       = glm::lookAt(
                glm::vec3(-5, 3, -10),  // camera at (-5, 3, -10) in world space
                glm::vec3(0, 0, 0),     // look towards the origin
                glm::vec3(0, 1, 0)      // y-axis is upwards
            );
            m_Model = glm::mat4(1.0f);

            // clang-format off
			// Vulkan clip space has inverted Y and half Z
			m_Clip = glm::mat4(
				1,  0,    0, 0,
				0, -1,    0, 0,
				0,  0, 0.5f, 0,
				0,  0, 0.5f, 1
			);
            // clang-format on

            m_MVP = m_Clip * m_Projection * m_View * m_Model;
        }

        {
            // request a buffer for the MVP matrix
            vk::BufferCreateInfo bci;

            bci
                .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setSize(sizeof(m_MVP));

            m_UniformBuffer = m_Device->createBufferUnique(bci);

            // make sure this is host visible and coherent
            auto reqs    = m_Device->getBufferMemoryRequirements(*m_UniformBuffer);
            auto memType = graphics::selectMemoryTypeIndex(
                m_PhysicalDevice,
                reqs.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eHostVisible
                    | vk::MemoryPropertyFlagBits::eHostCoherent);

            vk::MemoryAllocateInfo mai;
            mai.setAllocationSize(reqs.size).setMemoryTypeIndex(memType);

            m_UniformMemory = m_Device->allocateMemoryUnique(mai);

            // map the memory, copy the matrix and bind the buffer
            std::byte* pData;
            m_Device->mapMemory(
                *m_UniformMemory, 0, reqs.size, vk::MemoryMapFlags(), (void**)&pData);

            memcpy(pData, &m_MVP, sizeof(m_MVP));

            m_Device->unmapMemory(*m_UniformMemory);
            m_Device->bindBufferMemory(*m_UniformBuffer, *m_UniformMemory, 0);
        }
    }

    void Graphics::initPipelineLayouts() {
        vk::DescriptorSetLayoutBinding bindings[2];

        bindings[0]
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eVertex);

        bindings[1]
            .setBinding(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment);

        vk::DescriptorSetLayoutCreateInfo dsli;
        dsli
            .setBindingCount(2)
            .setPBindings(bindings);

        // [NOTE] not sure how to do this with multiple descriptor set layouts
        m_DescriptorSetLayout = m_Device->createDescriptorSetLayoutUnique(dsli);

        vk::PipelineLayoutCreateInfo pli;
        pli
            .setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr)
            .setSetLayoutCount(1)
            .setPSetLayouts(&*m_DescriptorSetLayout);  // seems sketchy

        m_PipelineLayout = m_Device->createPipelineLayoutUnique(pli);
    }

    void Graphics::initRenderPass() {
        // depends on swapchain and its depth buffer

        // create attachments for color and depth rendertargets
        vk::AttachmentDescription attachments[2];

        attachments[0]
            .setFormat(m_SurfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        attachments[1]
            .setFormat(m_DepthFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference color;
        color
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference depth;
        depth
            .setAttachment(1)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setInputAttachmentCount(0)
            .setPInputAttachments(nullptr)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&color)
            .setPResolveAttachments(nullptr)
            .setPDepthStencilAttachment(&depth)
            .setPreserveAttachmentCount(0)
            .setPPreserveAttachments(nullptr);

        vk::RenderPassCreateInfo info;
        info.setAttachmentCount(2)
            .setPAttachments(attachments)
            .setSubpassCount(1)
            .setPSubpasses(&subpass)
            .setDependencyCount(0)
            .setPDependencies(nullptr);

        m_RenderPass = m_Device->createRenderPassUnique(info);
    }

    void Graphics::initShaders(
        const std::string& vtxSrc,
        const std::string& fragSrc) {
        if (!vtxSrc.empty()) {
            auto vtxSpirv = GLSL_to_SPV(vk::ShaderStageFlagBits::eVertex, vtxSrc);

            m_ShaderStages[0]
                .setStage(vk::ShaderStageFlagBits::eVertex)
                .setPName("main");

            vk::ShaderModuleCreateInfo smci;
            smci
                .setCodeSize(vtxSpirv.size() * sizeof(uint32_t))
                .setPCode(vtxSpirv.data());

            m_VertexShader = m_Device->createShaderModuleUnique(smci);
        }

        if (!fragSrc.empty()) {
            auto fragSpirv = GLSL_to_SPV(vk::ShaderStageFlagBits::eFragment, fragSrc);

            m_ShaderStages[1]
                .setStage(vk::ShaderStageFlagBits::eFragment)
                .setPName("main");

            vk::ShaderModuleCreateInfo smci;
            smci
                .setCodeSize(fragSpirv.size() * sizeof(uint32_t))
                .setPCode(fragSpirv.data());

            m_FragShader = m_Device->createShaderModuleUnique(smci);
        }
    }

    void Graphics::initFrameBuffers() {
        vk::ImageView attachments[2];

        auto chain = getMainWindow()->getSwapchain();

        attachments[1] = chain->getDepthView();  // all of the framebuffers have the same depth view

        vk::FramebufferCreateInfo fci;

        fci
            .setRenderPass(*m_RenderPass)
            .setAttachmentCount(2)
            .setPAttachments(attachments)
            .setWidth(getMainWindow()->getWidth())
            .setHeight(getMainWindow()->getHeight())
            .setLayers(1);

        size_t numViews = chain->getNumColorViews();
        for (size_t i = 0; i < numViews; ++i) {
            attachments[0] = chain->getColorView(i);

            m_FrameBuffers.push_back(m_Device->createFramebufferUnique(fci));
        }
    }

    std::vector<uint32_t> Graphics::GLSL_to_SPV(
        const vk::ShaderStageFlagBits shaderType,
        const std::string&            shaderSrc) {
        shaderc::Compiler       compiler;
        shaderc::CompileOptions options;

        options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);
        options.SetWarningsAsErrors();
        options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, 0);
        options.SetOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_zero);

        // [TODO] we could set include resolving callbacks here
        // [TODO] we could add macro definitions here

        shaderc_shader_kind kind;

        switch (shaderType) {
        case vk::ShaderStageFlagBits::eVertex: kind = shaderc_shader_kind::shaderc_vertex_shader; break;
        case vk::ShaderStageFlagBits::eFragment: kind = shaderc_shader_kind::shaderc_fragment_shader; break;

        default:
            throw std::runtime_error("Unsupported shader type");
        }

        auto        prep = compiler.PreprocessGlsl(shaderSrc, kind, "in-memory", options);
        std::string processed;

        if (prep.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success) {
            processed = std::string(prep.cbegin(), prep.cend());
        }
        else {
            gLogError << prep.GetErrorMessage();

            throw std::runtime_error("Failed to preprocess shader");
        }

        auto result = compiler.CompileGlslToSpv(processed, kind, "in-memory", options);

        if (result.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success)
            return std::vector<uint32_t>(result.cbegin(), result.cend());
        else {
            gLogError << result.GetErrorMessage();

            throw std::runtime_error("Failed to compile shader");
        }
    }

    namespace graphics {
        uint32_t selectMemoryTypeIndex(
            vk::PhysicalDevice      gpu,
            uint32_t                typeBits,
            vk::MemoryPropertyFlags properties) {
            auto props = gpu.getMemoryProperties();

            for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
                if ((typeBits & 1) == 1) {
                    if ((props.memoryTypes[i].propertyFlags & properties) == properties)
                        return i;
                }

                typeBits >>= 1;  // NOTE not entirely sure about this
            }

            return 0;  // no match found
        }
    }  // namespace graphics
}  // namespace djinn
