#pragma once

#include "log_message.h"
#include "preprocessor.h"
#include <filesystem>
#include <memory>

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
		LogSink(const LogSink& sink) = delete;
		LogSink& operator=(const LogSink& sink) = delete;
		LogSink(LogSink&& sink) noexcept;
		LogSink& operator=(LogSink&& sink) noexcept;

		bool operator==(
		    const LogSink& sink) const;  // implemented so that Logger can detect duplicate sinks

		void write(const LogMessage::MetaInfo& metaInfo, const std::string& message);

	private:
		struct Concept {
			virtual ~Concept() = default;

			virtual void write(const LogMessage::MetaInfo& metaInfo, const std::string& message)
			    = 0;
		};

		template <typename T>
		struct Model: Concept {
			Model(T impl);

			virtual void
			    write(const LogMessage::MetaInfo& metaInfo, const std::string& message) override;

			T m_Impl;
		};

		std::unique_ptr<Concept> m_Wrapper;
	};

	// [logLevel] [message][\n] ~~> std::cout
	LogSink makeConsoleSink();

	// [timestamp] [loglevel] [message] ([sourcefile]:[linenumber])[\n] ~~> file path
	LogSink makeFileSink(const std::filesystem::path& path);

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
	// [logLevel] [message][\n] ~~> OutputDebugStringA
	LogSink makeWindowsConsoleSink();
#endif
}  // namespace djinn::core

#include "log_sink.inl"
