#include "input.h"
#include "util/algorithm.h"

namespace djinn {
	Input::Input(): System("Input") {}

	void Input::init() {
		System::init();
	}

	void Input::update() {}

	void Input::shutdown() {
		System::shutdown();
	}

	void Input::unittest() {}

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

	void Input::registerDevice(Mouse* m) {
		if (!util::contains(m_Mice, m))
			m_Mice.push_back(m);
		else
			gLogWarning << "Duplicate mouse registration, discarding...";
	}

	void Input::unregisterDevice(Mouse* m) {
		auto it = util::find(m_Mice, m);

		if (it != std::end(m_Mice))
			m_Mice.erase(it);
		else
			gLogWarning << "Cannot unregister unlisted mouse, ignoring...";
	}

	const std::vector<Input::Keyboard*>& Input::getKeyboards() const {
		return m_Keyboards;
	}

	const std::vector<Input::Mouse*>& Input::getMice() const {
		return m_Mice;
	}
}  // namespace djinn
