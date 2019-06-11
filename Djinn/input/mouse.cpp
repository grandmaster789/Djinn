#include "mouse.h"
#include "core/mediator.h"
#include "input.h"
#include <ostream>
#include <type_traits>

namespace djinn::input {
	Mouse::Mouse(Input* manager): m_Manager(manager) {
		m_Manager->registerDevice(this);
	}

	Mouse::~Mouse() {
		m_Manager->unregisterDevice(this);
	}

	bool Mouse::isDown(eButton button) const {
		return m_Buttons[static_cast<std::underlying_type_t<eButton>>(button)];
	}

	bool Mouse::isUp(eButton button) const {
		return !isDown(button);
	}

	std::pair<float, float> Mouse::getPosition() const {
		return std::make_pair(m_X, m_Y);
	}

	void Mouse::setButtonState(eButton button, bool isPressed) {
		bool current = isDown(button);

		if (current != isPressed) {
			m_Buttons[static_cast<std::underlying_type_t<eButton>>(button)] = isPressed;

			if (isPressed)
				broadcast(OnButtonPressed{this, m_X, m_Y, button});
			else
				broadcast(OnButtonReleased{this, m_X, m_Y, button});
		}
	}

	void Mouse::setPosition(float x, float y) {
		float oldX = m_X;
		float oldY = m_Y;

		m_X = x;
		m_Y = y;

		broadcast(OnMoved{this, x, y, x - oldX, y - oldY});
	}

	void Mouse::doDoubleClick(eButton button) {
		broadcast(OnDoubleClick{this, m_X, m_Y, button});
	}

	void Mouse::doScroll(int amount) {
		broadcast(OnScroll{this, amount});
	}

	void Mouse::doEnter(Window* w) {
		broadcast(OnEnterWindow{this, w});
	}

	void Mouse::doLeave(Window* w) {
		broadcast(OnLeaveWindow{this, w});
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::eButton& button) {
		using eButton = Mouse::eButton;

		switch (button) {
		case eButton::left: os << "(LMB)"; break;
		case eButton::right: os << "(RMB)"; break;
		case eButton::middle: os << "(MMB)"; break;
		default: os << "<< unknown button >>";
		}

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnMoved& mm) {
		os << '(' << mm.m_X << ", " << mm.m_Y << ") " << '[' << mm.m_DeltaX << ", " << mm.m_DeltaY
		   << ')';

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnButtonPressed& bp) {
		os << "Mouse button pressed: " << bp.m_Button;

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnButtonReleased& br) {
		os << "Mouse button released: " << br.m_Button;

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnDoubleClick& bc) {
		os << "Double click: " << bc.m_Button;

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnScroll& ms) {
		os << "Scroll: " << ms.m_ScrollAmount;

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnEnterWindow&) {
		os << "Entered window";

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mouse::OnLeaveWindow&) {
		os << "Exited window";

		return os;
	}
}  // namespace djinn::input
