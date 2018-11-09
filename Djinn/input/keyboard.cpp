#include <type_traits>
#include "core/mediator.h"
#include "keyboard.h"
#include "input.h"

namespace djinn::input {
	Keyboard::Keyboard(Input* manager) :
		m_Manager(manager)
	{
		m_Manager->registerDevice(this);
	}

	Keyboard::~Keyboard() {
		m_Manager->unregisterDevice(this);
	}

	bool Keyboard::isDown(eKey key) const {
		return m_Keys[static_cast<std::underlying_type_t<eKey>>(key)];
	}

	bool Keyboard::isUp(eKey key) const {
		return !isDown(key);
	}

	void Keyboard::setKeyState(eKey key, bool isPressed) {
		bool current = isDown(key);

		if (current != isPressed) {
			m_Keys[static_cast<std::underlying_type_t<eKey>>(key)] = isPressed;

			if (isPressed)
				broadcast(OnKeyPressed{ key });
			else
				broadcast(OnKeyReleased{ key });
		}
	}
}