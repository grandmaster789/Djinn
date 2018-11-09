#pragma once

#include "core/system.h"

#include <vector>

namespace djinn::display {
	class Window;
}

namespace djinn::input {
    class Keyboard;
    class Mouse;
    class Gamepad;
}

namespace djinn {
    class Input :
        public core::System
    {
    public:
		using Keyboard = input::Keyboard;
		using Mouse    = input::Mouse;
		using Gamepad  = input::Gamepad;

        Input();

        void init() override;
        void update() override;
        void shutdown() override;

        void unittest() override;

		void registerDevice(Keyboard* kbd);
		void unregisterDevice(Keyboard* kbd);
		
		std::vector<Keyboard*> getKeyboards() const;


    private:
		std::vector<Keyboard*> m_Keyboards;
    };
}
