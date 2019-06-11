#pragma once

#include "core/system.h"

namespace djinn::app {
	class Application: public core::System {
	public:
		Application(const std::string& name);

		virtual void init()     = 0;
		virtual void update()   = 0;
		virtual void shutdown() = 0;

		void unittest() override;  // no real way to enable unit test for applications (yet)
	};
}  // namespace djinn::app
