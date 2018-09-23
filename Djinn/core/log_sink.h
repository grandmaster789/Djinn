#pragma once

#include "log_message.h"
#include <memory>
#include <filesystem>

namespace djinn::core {
    // Type-erased interface
    //
    // this will accept any type of backend that has implemented
    // ::operator()(const LogMessage::MetaInfo&, const std::string)
    class LogSink {
    public:
	    template <typename T>
	    LogSink(T&& impl);

	    // contains a std::unique_ptr, so move-only
	    LogSink             (const LogSink& sink) = delete;
	    LogSink& operator = (const LogSink& sink) = delete;
	    LogSink             (LogSink&& sink);
	    LogSink& operator = (LogSink&& sink);

	    bool operator == (const LogSink& sink) const; // implemented so that Logger can detect duplicate sinks

	    void write(
		    const LogMessage::MetaInfo& metaInfo,
		    const std::string& message
	    );

    private:
	    struct Concept {
		    virtual ~Concept() = default;				
		
		    virtual void write(
			    const LogMessage::MetaInfo& metaInfo,
			    const std::string& message
		    ) = 0;
	    };

	    template <typename T>
	    struct Model: Concept {
		    Model(T impl);
		
		    virtual void write(
			    const LogMessage::MetaInfo& metaInfo,
			    const std::string& message
		    ) override;

		    T mImpl;
	    };

	    std::unique_ptr<Concept> mWrapper;
    };

    LogSink makeConsoleSink();												// [logLevel] [message][\n] to std::cout
    LogSink makeFileSink(const std::experimental::filesystem::path& path);	// [timestamp] [loglevel] [message] ([sourcefile]:[linenumber])[\n]
}

#include "log_sink.inl"