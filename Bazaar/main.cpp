#include "core/engine.h"
#include "app/application.h"
#include "display/display.h"
#include "input/input.h"
#include "input/keyboard.h"
#include <iostream>

#include "util/enum.h"

using namespace djinn;

class Bazaar :
    public app::Application,
	public MessageHandler<input::Keyboard::OnKeyPressed>
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

		case eKey::space:
			gLog << "Space pressed";
			break;
		}
	}
};

int main() {
	auto& engine = Engine::instance();

    engine.enable<Display>();
    engine.enable<Input>();

    engine.setApplication<Bazaar>();

    engine.run();
}