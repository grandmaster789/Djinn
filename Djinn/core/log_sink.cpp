#include "log_sink.h"
#include "rang.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace djinn::core {
	LogSink::LogSink(LogSink&& sink) noexcept: m_Wrapper(std::move(sink.m_Wrapper)) {}

	LogSink& LogSink::operator=(LogSink&& sink) noexcept {
		m_Wrapper = std::move(sink.m_Wrapper);
		return *this;
	}

	bool LogSink::operator==(const LogSink& sink) const {
		return (sink.m_Wrapper.get() == m_Wrapper.get());
	}

	void LogSink::write(const LogMessage::MetaInfo& info, const std::string& message) {
		m_Wrapper->write(info, message);
	}

	LogSink makeConsoleSink() {
		return LogSink([](const LogMessage::MetaInfo& info, const std::string& message) {
			switch (info.m_Category) {
			case eLogCategory::DEBUG: std::cout << rang::fgB::green; break;
			case eLogCategory::WARNING: std::cout << rang::fgB::yellow; break;
			case eLogCategory::ERROR_: std::cout << rang::fgB::red; break;
			case eLogCategory::FATAL:
				std::cout << rang::style::blink << rang::fgB::red;
				break;
				// 'message' gets the default color/style scheme
			}

			std::cout << info.m_Category << message << "\n";
			std::cout << rang::style::reset;
		});
	}

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
	LogSink makeWindowsConsoleSink() {
		return LogSink([](const LogMessage::MetaInfo& info, const std::string& message) {
			std::stringstream sstr;
			sstr << info.m_Category << message << "\n";
			OutputDebugStringA(sstr.str().c_str());
		});
	}
#endif

	namespace {
		struct FileSink {
			FileSink(const std::filesystem::path& path) {
				mFile = std::make_unique<std::ofstream>(path.string());

				if (!mFile->good()) {
					std::string message = "Failed to open file sink: ";
					message.append(path.string());
					throw std::runtime_error(message);
				}
			}

			FileSink(const FileSink&) = delete;
			FileSink& operator=(const FileSink&) = delete;

			FileSink(FileSink&&) = default;
			FileSink& operator=(FileSink&&) = default;

			void
			    operator()(const LogMessage::MetaInfo& meta, const std::string& message) const {
				using namespace std::chrono;
				using std::experimental::filesystem::path;

				auto now    = system_clock::now();
				auto time_t = system_clock::to_time_t(now);

				tm localtime;
				localtime_s(&localtime, &time_t);

				(*mFile)
				    << std::put_time(&localtime, "[%H:%M:%S] ") << meta.m_Category << message
				    << " ("
				    // strip the source data to just the filename, disarding the rest of the path
				    << path(meta.m_SourceFile).filename().string() << ":" << meta.m_SourceLine
				    << ")\n";
			}

			std::unique_ptr<std::ofstream> mFile;  // use a unique_ptr so it can be mvoed around
		};
	}  // namespace

	LogSink makeFileSink(const std::filesystem::path& path) {
		return FileSink(path);
	}
}  // namespace djinn::core
