#pragma once

#include "log_message.h"

namespace djinn {
	namespace core {
		template <typename T>
		LogMessage& LogMessage::operator << (const T& message) {
			mBuffer << std::boolalpha << message;
			return *this;
		}
	}
}