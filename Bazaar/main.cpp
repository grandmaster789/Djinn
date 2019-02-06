#include "core/engine.h"
#include "app/application.h"
#include "context/context.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include <iostream>

#include "util/reflect.h"

using namespace djinn;

class Bazaar :
    public app::Application,
	public MessageHandler<input::Keyboard::OnKeyPressed>,
    //public MessageHandler<input::Keyboard::OnKeyReleased>,
    //public MessageHandler<input::Mouse::OnMoved>,
    public MessageHandler<input::Mouse::OnButtonPressed>,
    //public MessageHandler<input::Mouse::OnButtonReleased>,
    public MessageHandler<input::Mouse::OnDoubleClick>,
    public MessageHandler<input::Mouse::OnScroll>,
    public MessageHandler<input::Mouse::OnEnterWindow>,
    public MessageHandler<input::Mouse::OnLeaveWindow>
{
public:
    Bazaar():
        Application("Bazaar")
    {
    }

    void init() override {
        System::init();
    }

    void update() override {
    }

    void shutdown() override {
        System::shutdown();
    }

	void operator()(const input::Keyboard::OnKeyPressed& kp) {
		using eKey = input::Keyboard::eKey;

		switch (kp.key) {
		case eKey::escape:
			m_Engine->stop();
			break;

        default:
            gLog << kp;
		}
	}

    void operator()(const input::Keyboard::OnKeyReleased& kr) { gLog << kr; }

    void operator()(const input::Mouse::OnMoved& mm)          { gLog << mm; }
    void operator()(const input::Mouse::OnButtonPressed& bp)  { gLog << bp; }
    void operator()(const input::Mouse::OnButtonReleased& br) { gLog << br; }
    void operator()(const input::Mouse::OnDoubleClick& dc)    { gLog << dc; }
    void operator()(const input::Mouse::OnScroll& sc)         { gLog << sc; }

    void operator()(const input::Mouse::OnEnterWindow& ow) { gLog << ow; }
    void operator()(const input::Mouse::OnLeaveWindow& ow) { gLog << ow; }
};

int main() {
	auto& engine = Engine::instance();

    engine.enable<Context>();
    engine.enable<Input>();

    engine.setApplication<Bazaar>();

    engine.run();
}