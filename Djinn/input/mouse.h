#pragma once

#include <utility>
#include <iosfwd>

namespace djinn {
    class Input;

    namespace context {
        class Window;
    }

    namespace input {
        // [TODO] when I've got some math basics in place, the XY stuff should be some
        //        kind of point struct (plus directions perhaps?)
        // [TODO] do I want dragging support?

        // [NOTE] X/Y coordinates are normalized to [-1..1] within the associated window

        class Mouse {
        public:
            using Window = context::Window;

            enum class eButton {
                left,
                right,
                middle
            };

            Mouse(Input* baseSystem);
            ~Mouse();

            Mouse             (const Mouse&) = delete;
            Mouse& operator = (const Mouse&) = delete;
            Mouse             (Mouse&&)      = delete;
            Mouse& operator = (Mouse&&)      = delete;

            bool isDown(eButton button) const;
            bool isUp(eButton button) const;

            std::pair<float, float> getPosition() const;

            void setButtonState(eButton button, bool pressed);
            void setPosition(float x, float y);

            void doDoubleClick (eButton button);
            void doScroll(int amount);
            void doEnter(Window* w);
            void doLeave(Window* w);

            // --------------------- Events -----------------------
            struct OnMoved {
                Mouse* m_Mouse;
                float  m_X; 
                float  m_Y;

                float m_DeltaX;
                float m_DeltaY;
            };

            struct OnButtonPressed {
                Mouse*  m_Mouse;
                float   m_X;
                float   m_Y; 
                eButton m_Button;
            };

            struct OnButtonReleased {
                Mouse*  m_Mouse;
                float   m_X;
                float   m_Y;
                eButton m_Button;
            };

            struct OnDoubleClick {
                Mouse*  m_Mouse;
                float   m_X;
                float   m_Y;
                eButton m_Button;
            };

            struct OnScroll {
                Mouse* m_Mouse;
                int    m_ScrollAmount;
            };

            struct OnEnterWindow {
                Mouse*  m_Mouse;
                Window* m_Window;
            };

            struct OnLeaveWindow {
                Mouse*  m_Mouse;
                Window* m_Window;
            };

        private:
            Input* m_Manager;

            bool  m_Buttons[3] = {};
            float m_X = 0;
            float m_Y = 0;
        };

        std::ostream& operator << (std::ostream& os, const Mouse::eButton& button);
        
        std::ostream& operator << (std::ostream& os, const Mouse::OnMoved& mm);
        std::ostream& operator << (std::ostream& os, const Mouse::OnButtonPressed& bp);
        std::ostream& operator << (std::ostream& os, const Mouse::OnButtonReleased& br);
        std::ostream& operator << (std::ostream& os, const Mouse::OnDoubleClick& bc);
        std::ostream& operator << (std::ostream& os, const Mouse::OnScroll& ms);
        std::ostream& operator << (std::ostream& os, const Mouse::OnEnterWindow& ew);
        std::ostream& operator << (std::ostream& os, const Mouse::OnLeaveWindow& lw);
    }
}
