#include "core/engine.h"
#include "app/application.h"
#include "renderer/renderer.h"
#include "renderer/window.h"
#include "input/input.h"
#include "input/keyboard.h"
#include <iostream>

#include "util/variant.h"

using namespace djinn;

class Bazaar :
    public app::Application,
    public MessageHandler<input::Keyboard::OnKeyPress>
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

    void operator()(const input::Keyboard::OnKeyPress& kp) {
        switch (kp.m_Key) {
        case GLFW_KEY_ESCAPE:
            m_Engine->stop();
            break;

        default:
            gLog << "Key pressed: " << kp.m_Key;
            break;
        }
    }
};

int main() {

	auto& engine = Engine::instance();

    engine.enable<Renderer>();
    engine.enable<Input>();

    engine.setApplication<Bazaar>();

    engine.run();
}