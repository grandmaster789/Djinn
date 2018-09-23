#pragma once

#include "log_category.h"
#include "log_message.h"
#include "log_sink.h"

#include <vector>

namespace djinn {
	namespace core {
		class Logger {
		public:
			explicit Logger() = default; // allow for default construction, but make it explicit
			Logger(const std::string& filename); // this is the real default -- log to both a file and std::cout

			LogMessage operator()(
				eLogCategory       category,
				const std::string& sourceFilename,
				int                sourceLine
			);

			void add(LogSink sink);
			void remove(LogSink sink);
			void removeAll();
			size_t getNumSinks() const;

			void flush(const LogMessage* message);

			static Logger& instance(); // this class is provided both as a singleton and a regular object; :instance() yields the singleton, obviously

		private:
			std::vector<LogSink> mSinks;
		};
	}
}

// some macros that make it as painless as possible to log something
#define gLogCategory(category) (				\
	::djinn::core::Logger::instance()(			\
		::djinn::core::eLogCategory::category,	\
		__FILE__,								\
		__LINE__								\
	))

#define gLog		(gLogCategory(MESSAGE))

#define gLogDebug	(gLogCategory(DEBUG))
#define gLogMessage	(gLogCategory(MESSAGE))
#define gLogError	(gLogCategory(ERROR_))
#define gLogWarning (gLogCategory(WARNING))
#define gLogFatal	(gLogCategory(FATAL))