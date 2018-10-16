#include "input.h"

namespace djinn {
    Input::Input():
        System("Input")
    {
		addDependency("Display");
    }

    void Input::init() {
        System::init();
    }

    void Input::update() {
    }

    void Input::shutdown() {
        System::shutdown();
    }

    void Input::unittest() {
    }
}