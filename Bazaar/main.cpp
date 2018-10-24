#include "core/engine.h"
#include "app/application.h"
#include "display/display.h"
#include "input/input.h"
#include <iostream>

#include "util/enum.h"

using namespace djinn;

enum class FooBar {
    aaa,
    bbb,
    ccc,
    ddd,
    eee,
    fff
};

using FooBarIterator = util::EnumIterator<FooBar, FooBar::aaa, FooBar::fff>;

std::ostream& operator << (std::ostream& os, const FooBar& fb) {
    switch (fb) {
    case FooBar::aaa: os << "aaa"; break;
    case FooBar::bbb: os << "bbb"; break;
    case FooBar::ccc: os << "ccc"; break;
    case FooBar::ddd: os << "ddd"; break;
    case FooBar::eee: os << "eee"; break;
    case FooBar::fff: os << "fff"; break;
    }

    return os;
}

class Bazaar :
    public app::Application
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
        static int count = 0;
        
        if (++count > 10'000)
            m_Engine->stop();
    }

    void shutdown() override {
        System::shutdown();
    }
};

int main() {
    for (const auto& val : FooBarIterator()) {
        std::cout << val << "\n";
    }

	auto& engine = Engine::instance();

    engine.enable<Display>();
    engine.enable<Input>();

    engine.setApplication<Bazaar>();

    engine.run();
}