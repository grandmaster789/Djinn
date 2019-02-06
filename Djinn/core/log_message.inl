#pragma once

#include "log_message.h"

namespace djinn::core {
    template <typename T>
    LogMessage& LogMessage::operator << (const T& message) {
    	m_Buffer << std::boolalpha << message;
    	return *this;
    }
}