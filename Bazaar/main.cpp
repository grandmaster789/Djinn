#include "core/engine.h"
#include "app/application.h"
#include "renderer/renderer.h"
#include <iostream>

class Bazaar :
    public djinn::app::Application
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

    void unittest() override {
    }
};

int main() {
    using namespace djinn;
    auto& engine = Engine::instance();

    engine.enable<Renderer>();
    engine.setApplication<Bazaar>();
    engine.run();
}