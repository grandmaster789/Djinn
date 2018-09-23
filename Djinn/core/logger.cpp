#include "logger.h"
#include "log_sink.h"

namespace djinn {
	namespace core {
		Logger::Logger(const std::string& filename) {
			add(makeConsoleSink());
			add(makeFileSink(filename));
		}

		LogMessage Logger::operator()(
			eLogCategory category,
			const std::string& filename,
			int line
		) {
			return LogMessage(
				this,
				category,
				filename,
				line
			);
		}

		void Logger::add(LogSink sink) {
			using namespace std;

			auto it = find(begin(mSinks), end(mSinks), sink);

			if (it == end(mSinks))
				mSinks.push_back(std::forward<LogSink>(sink));
			// else // silently fail if the sink is already present
		}

		void Logger::remove(LogSink sink) {
			using namespace std;

			auto it = find(begin(mSinks), end(mSinks), sink);

			if (it != end(mSinks))
				mSinks.erase(it);
			// else // silently fail if the sink is not actually present
		}

		void Logger::removeAll() {
			mSinks.clear();
		}

		size_t Logger::getNumSinks() const {
			return mSinks.size();
		}

		void Logger::flush(const LogMessage* message) {
			std::string msg = message->mBuffer.str(); // collapse the buffer into a regular string

			for (auto& sink : mSinks)
				sink.write(message->mMetaInfo, msg);
		}

		Logger& Logger::instance() {
			static Logger gLogger("djinn.log");
			return gLogger;
		}
	}
}