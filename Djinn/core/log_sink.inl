#pragma once

#include "log_sink.h"

namespace djinn::core {
	template <typename T>
	LogSink::LogSink(T&& impl) {
		m_Wrapper = std::unique_ptr<Concept>(new Model<T>(std::forward<T>(impl)));
	}

	template <typename T>
	LogSink::Model<T>::Model(T impl): m_Impl(std::forward<T>(impl)) {}

	template <typename T>
	void LogSink::Model<T>::write(const LogMessage::MetaInfo& meta, const std::string& message) {
		m_Impl(meta, message);
	}
}  // namespace djinn::core
