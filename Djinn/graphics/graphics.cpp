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
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			gLogDebug << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			gLogError << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
		else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			gLog << "[" << layerPrefix << "] Code " << code << " : " << message;

		return VK_FALSE;
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
		if (m_Window) {
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

			if (!m_Window) return;
		}
	}

	void Graphics::shutdown() {
		System::shutdown();

		m_Window.reset();
	}

	void Graphics::unittest() {}

	void Graphics::close(Window* w) {
		if (w == m_Window.get()) m_Window.reset();
	}

	Graphics::Window*
	    Graphics::createWindow(int width, int height, bool windowed, int displayDevice) {
		m_Window = std::make_unique<Window>(width, height, windowed, displayDevice, this);

		return m_Window.get();
	}
}  // namespace djinn
