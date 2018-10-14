#pragma once

#include "dependencies.h"
#include "core/system.h"
#include "core/mediator.h"
#include "util/flat_map.h"

#include <memory>

namespace djinn {
    class Renderer:
        public core::System
    {
    public:
        Renderer();

        virtual void init() override;
        virtual void update() override;
        virtual void shutdown() override;

        virtual void unittest() override;

    private:
        
    };
}
