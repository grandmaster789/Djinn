#include "log_sink.h"
#include "rang.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

namespace djinn::core {
    LogSink::LogSink(LogSink&& sink):
    	mWrapper(std::move(sink.mWrapper))
    {
    }
    
    LogSink& LogSink::operator = (LogSink&& sink) {
    	mWrapper = std::move(sink.mWrapper);
    	return *this;
    }
    
    bool LogSink::operator == (const LogSink& sink) const {
    	return (sink.mWrapper.get() == mWrapper.get());
    }
    
    void LogSink::write(
    	const LogMessage::MetaInfo& info,
    	const std::string& message
    ) {
    	mWrapper->write(info, message);
    }
    
    LogSink makeConsoleSink() {
    	return LogSink(
    		[](
    			const LogMessage::MetaInfo& info, 
    			const std::string& message
    		) {
    			switch (info.mCategory) {
    			case eLogCategory::DEBUG:	std::cout << rang::fgB::green;						break;
    			case eLogCategory::WARNING: std::cout << rang::fgB::yellow;						break;
    			case eLogCategory::ERROR_:	std::cout << rang::fgB::red;						break;
    			case eLogCategory::FATAL:   std::cout << rang::style::blink << rang::fgB::red;	break;
    			// 'message' gets the default color/style scheme
    			}
    
    			std::cout << info.mCategory << message << "\n";
    			std::cout << rang::style::reset;
    		}
    	);
    }
    
    namespace {
    	struct FileSink {
    		FileSink(const std::experimental::filesystem::path& path) {
    			mFile = std::make_unique<std::ofstream>(path.string());
    
    			if (!mFile->good()) {
    				std::string message = "Failed to open file sink: ";
    				message.append(path.string());
    				throw std::runtime_error(message);
    			}
    		}
    
    		FileSink(const FileSink&) = delete;
    		FileSink& operator = (const FileSink&) = delete;
    		
    		FileSink(FileSink&&) = default;
    		FileSink& operator = (FileSink&&) = default;
    
    		void operator()(
    			const LogMessage::MetaInfo& meta,
    			const std::string& message
    		) const {
    			using namespace std::chrono;
                using std::experimental::filesystem::path;
    
    			auto now = system_clock::now();
    			auto time_t = system_clock::to_time_t(now);
    			tm localtime;
    			localtime_s(&localtime, &time_t);
    
    			(*mFile)
    				<< std::put_time(&localtime, "[%H:%M:%S] ")
    				<< meta.mCategory
    				<< message
    				<< " ("
    				<< path(meta.mSourceFile).filename().string() // strip the source data to just the filename, disarding the rest of the path
    				<< ":"
    				<< meta.mSourceLine
    				<< ")\n";
    		}
    
    		std::unique_ptr<std::ofstream> mFile; // use a unique_ptr so it can be mvoed around
    	};
    }
    
    LogSink makeFileSink(const std::experimental::filesystem::path& path) {
    	return FileSink(path);
    }
}
