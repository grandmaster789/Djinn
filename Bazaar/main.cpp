#include "core/engine.h"
#include "app/application.h"
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
        ++i;
        std::cout << ".";

        if (i > 10'000)
            m_Engine->stop();
    }

    void shutdown() override {
        System::shutdown();
    }

    void unittest() override {
    }

private:
    int i = 0;
};

int main() {
    auto& engine = djinn::Engine::instance();

    engine.setApplication<Bazaar>();
    engine.run();
}