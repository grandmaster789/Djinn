#pragma once

#include "core/system.h"

#include <vector>

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
        Input();

        void init() override;
        void update() override;
        void shutdown() override;

        void unittest() override;

    private:
    };
}
