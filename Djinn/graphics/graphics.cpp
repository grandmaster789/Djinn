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

	// moderately accurate method for getting the surface extension names
	std::vector<const char*> getAvailableWSIExtensions() {
		std::vector<const char*> extensions;

		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_MIR_KHR)
		extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
		extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
		extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

		return extensions;
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

	Graphics::Window*
	    Graphics::createWindow(int width, int height, bool windowed, int displayDevice) {
		m_Windows.push_back(
		    std::make_unique<Window>(width, height, windowed, displayDevice, this));

		return m_Windows.back().get();
	}

	void Graphics::initVulkan() {
		std::vector<const char*> layers;
		std::vector<const char*> extensions = getAvailableWSIExtensions();

		vk::ApplicationInfo           appInfo;
		vk::InstanceCreateInfo        instInfo;
		vk::Win32SurfaceCreateInfoKHR surfaceInfo;

		appInfo.setPApplicationName("Vulkan program")
		    .setApplicationVersion(1)
		    .setPEngineName(nullptr)
		    .setEngineVersion(1)
		    .setApiVersion(VK_API_VERSION_1_0);

		instInfo.setFlags(vk::InstanceCreateFlags())
		    .setPApplicationInfo(&appInfo)
		    .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		    .setPpEnabledExtensionNames(extensions.data())
		    .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		    .setPpEnabledLayerNames(layers.data());

		m_Instance = vk::createInstanceUnique(instInfo);
	}
}  // namespace djinn
