#pragma once

namespace djinn {
	class Input;

	namespace input {
		class Keyboard {
		public:
			// grouped (groups in random order though)
			enum class eKey {
				undefined,

				a, b, c, d, e, f, g, h, i,
				j, k, l, m, n, o, p, q, r,
				s, t, u, v, w, x, y, z,

				_1, _2, _3, _4, _5, _6,
				_7, _8, _9, _0,

				f1, f2, f3, f4, f5, f6, f7,
				f8, f9, f10, f11, f12,

				backquote, quote,
				comma, point,
				backslash, slash,
				semicolon, bracket_open, bracket_close,
				minus, equals,

				ctrl, alt, shift, space, tab, enter, escape,

				up, left, right, down,
				pg_up, pg_down, home, end, ins, del
			};

			Keyboard(Input* baseSystem);
			~Keyboard();

			Keyboard(const Keyboard&) = delete;

			bool isDown(eKey key) const;
			bool isUp(eKey key) const;

			void setKeyState(eKey key, bool pressed);

			// --------------------- Events -----------------------
			// [NOTE] if we want multi-keyboard support, these may not
			//        have enough of a payload
			struct OnKeyPressed { eKey key; };
			struct OnKeyReleased { eKey key; };

		private:
			Input* m_Manager = nullptr;

			bool m_Keys[256] = {};
		};
	}
}