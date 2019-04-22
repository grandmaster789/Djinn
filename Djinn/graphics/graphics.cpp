#include "graphics.h"
#include "extensions.h"
#include "graphicsUtility.h"
#include "core/engine.h"
#include "util/flat_map.h"
#include "util/algorithm.h"
#include "math/trigonometry.h"

#include <fstream>

// ------ Vulkan debug reports ------
namespace {
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

    vk::Format selectColorFormat(vk::PhysicalDevice physical, vk::SurfaceKHR surface) {
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

        auto colorFormat = selectColorFormat(m_PhysicalDevice, *m_Surface);
        auto depthFormat = selectDepthFormat(m_PhysicalDevice);

        m_Renderpass = createSimpleRenderpass(colorFormat, depthFormat); // depends on the swapchain format                
        
        m_Swapchain = std::make_unique<Swapchain>(
            *m_Device,
             m_PhysicalDevice,
            *m_Surface,
             colorFormat,
             depthFormat,
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

        // right now, just one pool should suffice
        vk::DescriptorPoolSize simplePoolSize;
        
        simplePoolSize
            .setDescriptorCount(1);

        vk::DescriptorPoolCreateInfo dpci;
        dpci
            .setFlags        (vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets      (1)
            .setPoolSizeCount(1)
            .setPPoolSizes   (&simplePoolSize);

        m_DescriptorPool = m_Device->createDescriptorPoolUnique(dpci);

        // create a vertex buffer for a cube
        // 6 (faces) * 2 (triangles) * 3 (vertices) * 3 (coords) = 108 floats
        {
            static const float vertices[] = {
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,

                 1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                 1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,

                 1.0f,  1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                 1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,

                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,

                 1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,

                 1.0f, -1.0f, -1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,

                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                 1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,

                 1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f
            };

            // some typical vertex format
            // NOTE this needs to correspond to what's being specified
            //      at pipeline creation
            struct Vertex {
                float x, y, z, w;
                float r, g, b;
            };

            constexpr size_t vtxSize      = sizeof(Vertex); // might contain some padding! ofc, not in this case
            constexpr size_t numVertices  = sizeof(vertices) / (3 * sizeof(float));
            constexpr size_t numTriangles = numVertices / 3;

            // allocate buffer on the GPU
            vk::BufferCreateInfo bci;

            bci
                .setSize       (vtxSize* numVertices) // we need space for the colors as well
                .setUsage      (vk::BufferUsageFlagBits::eVertexBuffer)
                .setSharingMode(vk::SharingMode::eExclusive);

            m_Cube = m_Device->createBufferUnique(bci);

            auto req = m_Device->getBufferMemoryRequirements(*m_Cube);

            auto memoryIdx = graphics::selectMemoryTypeIndex(
                m_PhysicalDevice,
                req.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eHostVisible
            );

            vk::MemoryAllocateInfo mai;

            mai
                .setAllocationSize (req.size)
                .setMemoryTypeIndex(memoryIdx);

            m_CubeBuffer = m_Device->allocateMemoryUnique(mai);

            // transfer the vertex data and bind to the handle
            Vertex* mapped = static_cast<Vertex*>(
                m_Device->mapMemory(*m_CubeBuffer, 0, VK_WHOLE_SIZE)
            );

            for (int i = 0; i < numVertices; ++i) {
                mapped[i].x = vertices[i * 3 + 0];
                mapped[i].y = vertices[i * 3 + 1];
                mapped[i].z = vertices[i * 3 + 2];
                mapped[i].w = 1.0f;

                mapped[i].r = mapped[i].x;
                mapped[i].g = (i % 10) * 0.1f;
                mapped[i].b = mapped[i].z;
            }

            m_Device->unmapMemory(*m_CubeBuffer);
            m_Device->bindBufferMemory(*m_Cube, *m_CubeBuffer, 0);
        }

        // simple shader set assumes:
        // - a single uniform object with model/view/projection matrices
        // - vertex buffer input (in XYZW RGB vertex format)
        // - single output       (in RGBA format)
        //
        // the uniform buffer part is encoded here
        {
            // [NOTE] this currently relies on a post-build batch script to create correct locations
            m_SimpleVertexShader   = loadShader("assets/shaders/simple.vert.spv");
            m_SimpleFragmentShader = loadShader("assets/shaders/simple.frag.spv");

            // setup uniform
            {
                float zNear  = 0.1f;
                float zFar   = 1000.0f; // this might be pushing it; at this range we'll probably start to see Z-fighting
                float aspect = 
                    static_cast<float>(m_MainWindowSettings.m_Width) / 
                    static_cast<float>(m_MainWindowSettings.m_Height);

                /*auto projectionMatrix = glm::perspective(
                    math::deg2rad(45.0f), 
                    aspect, 
                    zNear, 
                    zFar
                );
                */

                float fov     = math::deg2rad(45.0f);
                float t       = 1.0f / tanf(fov * 0.5f);
                float nearFar = zNear - zFar;

                glm::mat4 projectionMatrix = {
                    t / aspect, 0, 0, 0,
                    0, t, 0, 0,
                    0, 0, (-zNear - zFar) / nearFar, (2 * zNear * zFar) / nearFar,
                    0, 0, 1, 0
                };

                // TODO make this a lookAt matrix
                glm::mat4 viewMatrix(1.0f); //this constructor makes it an identity matrix
                  
                // TODO this is a per-model thingie
                glm::mat4 modelMatrix = {
                    1, 0, 0, 0,
                    0, 1, 0, 2,
                    0, 0, 1, 10,
                    0, 0, 0, 1
                };

                vk::BufferCreateInfo bci;

                // should equal sizeof(float) * (4x4 matrix) * (mvp)
                //           == 4             * 16           * 3 = 192
                constexpr vk::DeviceSize bufferSize =
                    sizeof(projectionMatrix) +
                    sizeof(viewMatrix)       +
                    sizeof(modelMatrix);

                bci
                    .setSize       (bufferSize)
                    .setUsage      (vk::BufferUsageFlagBits::eUniformBuffer)
                    .setSharingMode(vk::SharingMode::eExclusive);
                    
                m_SimpleUniform = m_Device->createBufferUnique(bci);

                auto req = m_Device->getBufferMemoryRequirements(*m_SimpleUniform);

                auto memoryIdx = graphics::selectMemoryTypeIndex(
                    m_PhysicalDevice,
                    req.memoryTypeBits,
                    vk::MemoryPropertyFlagBits::eHostVisible
                );

                vk::MemoryAllocateInfo mai;

                mai
                    .setAllocationSize(req.size)
                    .setMemoryTypeIndex(memoryIdx);

                m_SimpleUniformBuffer = m_Device->allocateMemoryUnique(mai);

                // map, transfer and bind
                glm::mat4* mapped = static_cast<glm::mat4*>(
                    m_Device->mapMemory(*m_SimpleUniformBuffer, 0, VK_WHOLE_SIZE)
                );

                mapped[0] = modelMatrix;
                mapped[1] = viewMatrix;
                mapped[2] = projectionMatrix;

                m_Device->unmapMemory(*m_SimpleUniformBuffer);

                m_Device->bindBufferMemory(*m_SimpleUniform, *m_SimpleUniformBuffer, 0);
            }

            vk::DescriptorSetLayoutBinding dslb;

            dslb
                .setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex);

            vk::DescriptorSetLayoutCreateInfo dsci;

            dsci
                .setBindingCount(1)
                .setPBindings(&dslb);

            m_SimpleDescriptorLayout = m_Device->createDescriptorSetLayoutUnique(dsci);

            vk::DescriptorSetAllocateInfo dsai;
            
            dsai
                .setDescriptorPool    (*m_DescriptorPool)
                .setDescriptorSetCount(1)
                .setPSetLayouts       (&*m_SimpleDescriptorLayout);

            m_SimpleDescriptorSet = std::move(m_Device->allocateDescriptorSetsUnique(dsai).back()); // this returns a vector, but we just want the one

            vk::DescriptorBufferInfo dbi;

            dbi
                .setBuffer(*m_SimpleUniform)
                .setRange (VK_WHOLE_SIZE);

            vk::WriteDescriptorSet wds;
            wds
                .setDstSet         (*m_SimpleDescriptorSet)
                .setDstBinding     (0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType (vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo    (&dbi);

            m_Device->updateDescriptorSets(
                1,      // write descriptor count
                &wds,   // pWriteDescriptors
                0,      // descriptor copy count
                nullptr // pDescriptorCopies
            );

            vk::PipelineLayoutCreateInfo pli;

            pli
                .setSetLayoutCount(1)
                .setPSetLayouts   (&*m_SimpleDescriptorLayout);

            m_SimplePipelineLayout = m_Device->createPipelineLayoutUnique(pli);

            m_SimplePipeline = createSimpleGraphicsPipeline(
                *m_SimpleVertexShader,
                *m_SimpleFragmentShader,
                *m_SimplePipelineLayout
            );
        }
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
                if (m_Swapchain->getExtent() != vk::Extent2D(m_Window->getWidth(), m_Window->getHeight())) {
                    m_Swapchain = std::make_unique<Swapchain>(
                         *m_Device,
                          m_PhysicalDevice,
                         *m_Surface,
                          m_Swapchain->getImageFormat(),
                          m_Swapchain->getDepthFormat(),
                          m_GraphicsFamilyIdx,
                         *m_Renderpass,
                        &*m_Swapchain
                    );
                }

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

                    //m_GraphicsCommands->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_TrianglePipeline);
                    m_GraphicsCommands->bindPipeline(
                        vk::PipelineBindPoint::eGraphics, 
                        *m_SimplePipeline
                    );

                    /*
                    // draw calls go here
                    m_GraphicsCommands->draw(
                        3, // vertex count
                        1, // instance count
                        0, // first vertex
                        0  // first instance
                    );
                    */
                    m_GraphicsCommands->bindDescriptorSets(
                        vk::PipelineBindPoint::eGraphics,
                        *m_SimplePipelineLayout,
                        0,                       // first set
                        1,                       // number of descriptors
                        &*m_SimpleDescriptorSet, // the uniform descriptor set
                        0,                       // number of dynamic offsets
                        nullptr                  // pDynamicOffsets
                    );

                    vk::DeviceSize offsets = 0;
                    m_GraphicsCommands->bindVertexBuffers(
                        0, // first set
                        1, // number of bindings
                        &*m_Cube,
                        &offsets
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
            if (!graphics::areInstanceLayersAvailable(requiredLayers))
                throw std::runtime_error("Required layer not available");

            if (!graphics::areInstanceExtensionsAvailable(requiredExtensions))
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
            if (!graphics::areDeviceExtensionsAvailable(requiredDeviceExtensions, device))
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

        vk::RenderPassCreateInfo rp_info;

        rp_info
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments   (attachments.data())
            .setSubpassCount   (1)
            .setPSubpasses     (&subpassDesc);
        
        return m_Device->createRenderPassUnique(rp_info);
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

        vk::VertexInputBindingDescription vertexDescription;

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

        // this corresponds to the data spec we used during initialization,
        // but here we provide description details, where before we focused
        // on cpu-gpu data transfer
        struct Vertex {
            float x, y, z, w;
            float r, g, b;
        };

        constexpr uint32_t vertexSize = sizeof(Vertex);

        vertexDescription
            .setBinding  (0)
            .setStride   (vertexSize)
            .setInputRate(vk::VertexInputRate::eVertex);

        vk::VertexInputAttributeDescription vertexAttributes[2];

        vertexAttributes[0]
            .setLocation(0)
            .setBinding (0)
            .setFormat  (vk::Format::eR32G32B32A32Sfloat) // ~~ xyzw floats
            .setOffset  (0);

        vertexAttributes[1]
            .setLocation(1)
            .setBinding (0)
            .setFormat  (vk::Format::eR32G32B32Sfloat) // ~~ rgb floats
            .setOffset  (4 * sizeof(float));           // offset by xyzw

        vertexInputState
            .setVertexBindingDescriptionCount  (1)
            .setPVertexBindingDescriptions     (&vertexDescription)
            .setVertexAttributeDescriptionCount(2)
            .setPVertexAttributeDescriptions   (vertexAttributes);

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