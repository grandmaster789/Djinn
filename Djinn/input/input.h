#pragma once

#include "core/system.h"

#include <vector>

namespace djinn::input {
    class Keyboard;
    class Mouse;
    class Gamepad;
}

namespace djinn {
    /*
        [NOTE] The mouse/keyboard objects are owned by a Window... seems weird, but may not be an issue
        [NOTE] mouse/keyboard events are notification-based, while gamepad is done via polling
    */
    class Input :
        public core::System
    {
    public:
        using KeyboardList = std::vector<input::Keyboard*>;

        Input();

        void init() override;
        void update() override;
        void shutdown() override;

        void unittest() override;

    private:
        KeyboardList m_Keyboards;
    };
}
