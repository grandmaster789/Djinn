#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"

#include <memory>

namespace djinn {
    class Display :
        public core::System
    {
    private:

    public:
        Display();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;
    };
}
