#include "keyboard.h"
#include "core/mediator.h"
#include "input.h"
#include <ostream>
#include <type_traits>

namespace djinn::input {
	Keyboard::Keyboard(Input* manager): m_Manager(manager) {
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
				broadcast(OnKeyPressed{this, key});
			else
				broadcast(OnKeyReleased{this, key});
		}
	}

	std::ostream& operator<<(std::ostream& os, const Keyboard::eKey& key) {
		using eKey = Keyboard::eKey;

		switch (key) {
		case eKey::a: os << "A"; break;
		case eKey::b: os << "B"; break;
		case eKey::c: os << "C"; break;
		case eKey::d: os << "D"; break;
		case eKey::e: os << "E"; break;
		case eKey::f: os << "F"; break;
		case eKey::g: os << "G"; break;
		case eKey::h: os << "H"; break;
		case eKey::i: os << "I"; break;
		case eKey::j: os << "J"; break;
		case eKey::k: os << "K"; break;
		case eKey::l: os << "L"; break;
		case eKey::m: os << "M"; break;
		case eKey::n: os << "N"; break;
		case eKey::o: os << "O"; break;
		case eKey::p: os << "P"; break;
		case eKey::q: os << "Q"; break;
		case eKey::r: os << "R"; break;
		case eKey::s: os << "S"; break;
		case eKey::t: os << "T"; break;
		case eKey::u: os << "U"; break;
		case eKey::v: os << "V"; break;
		case eKey::w: os << "W"; break;
		case eKey::x: os << "X"; break;
		case eKey::y: os << "Y"; break;
		case eKey::z: os << "Z"; break;

		case eKey::_0: os << "0"; break;
		case eKey::_1: os << "1"; break;
		case eKey::_2: os << "2"; break;
		case eKey::_3: os << "3"; break;
		case eKey::_4: os << "4"; break;
		case eKey::_5: os << "5"; break;
		case eKey::_6: os << "6"; break;
		case eKey::_7: os << "7"; break;
		case eKey::_8: os << "8"; break;
		case eKey::_9: os << "9"; break;

		case eKey::f1: os << "F1"; break;
		case eKey::f2: os << "F2"; break;
		case eKey::f3: os << "F3"; break;
		case eKey::f4: os << "F4"; break;
		case eKey::f5: os << "F5"; break;
		case eKey::f6: os << "F6"; break;
		case eKey::f7: os << "F7"; break;
		case eKey::f8: os << "F8"; break;
		case eKey::f9: os << "F9"; break;
		case eKey::f10: os << "F10"; break;
		case eKey::f11: os << "F11"; break;
		case eKey::f12: os << "F12"; break;

		case eKey::semicolon: os << ";"; break;
		case eKey::questionmark: os << "?"; break;
		case eKey::tilde: os << "~"; break;
		case eKey::brace_open: os << "{"; break;
		case eKey::vertical_pipe: os << "|"; break;
		case eKey::brace_close: os << "}"; break;
		case eKey::double_quote: os << "\""; break;
		case eKey::oem_8:
			os << "oem_8";
			break;  // none of the keys on my keyboard correspond to this one

		case eKey::plus: os << "+"; break;
		case eKey::minus: os << "-"; break;
		case eKey::comma: os << ","; break;
		case eKey::period: os << "."; break;

		case eKey::ctrl: os << "(ctrl)"; break;
		case eKey::alt: os << "(alt)"; break;
		case eKey::shift: os << "(shift)"; break;
		case eKey::space: os << "(space)"; break;

		case eKey::tab: os << "(tab)"; break;
		case eKey::enter: os << "(enter)"; break;
		case eKey::escape: os << "(escape)"; break;
		case eKey::backspace: os << "(backspace)"; break;

		case eKey::up: os << "(up)"; break;
		case eKey::left: os << "(left)"; break;
		case eKey::right: os << "(right)"; break;
		case eKey::down: os << "(down)"; break;
		case eKey::pg_up: os << "(page up)"; break;
		case eKey::pg_down: os << "(page down)"; break;
		case eKey::home: os << "(home)"; break;
		case eKey::end: os << "(end)"; break;
		case eKey::ins: os << "(insert)"; break;
		case eKey::del:
			os << "(delete)";
			break;

			// some keys that remain - caps lock, scroll lock, pause, printscreen,
			//                         windows, contextmenu

		default: os << "<< Undefined >>"; break;
		}

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Keyboard::OnKeyPressed& kp) {
		os << "Key pressed: " << kp.key;

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Keyboard::OnKeyReleased& kr) {
		os << "Key released: " << kr.key;

		return os;
	}
}  // namespace djinn::input
