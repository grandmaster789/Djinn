#include "input.h"
#include "util/algorithm.h"

namespace djinn {
    Input::Input():
        System("Input")
    {
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

	void Input::registerDevice(Keyboard* kbd) {
		if (!util::contains(m_Keyboards, kbd))
			m_Keyboards.push_back(kbd);
		else
			gLogWarning << "Duplicate keyboard registration, discarding...";
	}

	void Input::unregisterDevice(Keyboard* kbd) {
		auto it = util::find(m_Keyboards, kbd);

		if (it != std::end(m_Keyboards))
			m_Keyboards.erase(it);
		else
			gLogWarning << "Cannot unregister unlisted keyboard, ignoring...";
	}

	std::vector<Input::Keyboard*> Input::getKeyboards() const {
		return m_Keyboards;
	}
}