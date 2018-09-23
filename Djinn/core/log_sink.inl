#pragma once

#include "log_sink.h"

namespace djinn::core {
    template <typename T>
    LogSink::LogSink(T&& impl) {
	    mWrapper = std::unique_ptr<Concept>(
		    new Model<T>(std::forward<T>(impl))
	    );
    }

    template <typename T>
    LogSink::Model<T>::Model(T impl):
	    mImpl(std::forward<T>(impl))
    {
    }

    template <typename T>
    void LogSink::Model<T>::write(
	    const LogMessage::MetaInfo& meta,
	    const std::string& message
    ) {
	    mImpl(meta, message);
    }
}