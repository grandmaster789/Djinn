#include "log_message.h"
#include "logger.h"

namespace djinn {
	namespace core {
		LogMessage::LogMessage(
			Logger* owner,
			eLogCategory category,
			const std::string& sourceFile,
			int sourceLineNumber
		):
			mOwner(owner),
			mMetaInfo {
				category,
				sourceFile,
				sourceLineNumber
			}
		{
		}

		LogMessage::~LogMessage() {
			if (mOwner)
				mOwner->flush(this);
		}

		LogMessage& LogMessage::operator << (std::ostream& (*fn)(std::ostream&)) {
			(*fn)(mBuffer);
			return *this;
		}
	}
}