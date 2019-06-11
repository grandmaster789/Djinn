#pragma once

#include "core/mediator.h"
#include "core/system.h"
#include "third_party.h"

#include "window.h"

#include <memory>

/*
    Very marginal platform dependant stuff in here:
    - the vulkan win32 surface extension name is in here
    - win32 surface creation
    - win32 presentation support
    [TODO] multi-GPU support... don't have the hardware for that tho
*/

namespace djinn {
	class Graphics: public core::System {
	public:
		using Window = graphics::Window;

		using WindowPtr = std::unique_ptr<Window>;

		Graphics();

		void init() override;
		void update() override;
		void shutdown() override;

		void unittest() override;

		void close(Window* w);

	private:
		Window* createWindow(
		    int  width         = 1280,
		    int  height        = 720,
		    bool windowed      = true,
		    int  displaydevice = 0);

		WindowPtr m_Window;

		struct WindowSettings {
			int  m_Width         = 1280;
			int  m_Height        = 720;
			int  m_DisplayDevice = 0;
			bool m_Windowed      = true;  // only supporting borderless fullscreen windows right now
		} m_MainWindowSettings;
	};
}  // namespace djinn
