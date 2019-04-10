#pragma once

#include "core/system.h"
#include "resource_allocator.h"

namespace djinn {
    /*
        Provide methods for loading data from disk; centralized
		access for shared resource data
    */
    class Resources :
        public core::System 
    {
	public:
        Resources();

        void init() override;
        void update() override;
        void shutdown() override;

        void unittest() override;

    private:

    };
}
