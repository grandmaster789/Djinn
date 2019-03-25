#pragma once

#include "core/system.h"

namespace djinn {
    class Graphics;

    class Renderer:
        public core::System
    {
    public:
        Renderer();

        void init()     override;
        void update()   override;
        void shutdown() override;

        void unittest() override;

    private:
        
    };
}
