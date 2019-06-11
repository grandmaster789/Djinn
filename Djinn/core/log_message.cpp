#include "log_message.h"
#include "logger.h"

namespace djinn::core {
	LogMessage::LogMessage(
	    Logger*            owner,
	    eLogCategory       category,
	    const std::string& sourceFile,
	    int                sourceLineNumber):
	    m_Owner(owner),
	    m_MetaInfo{category, sourceFile, sourceLineNumber} {}

	LogMessage::~LogMessage() {
		if (m_Owner) m_Owner->flush(this);
	}

	LogMessage& LogMessage::operator<<(std::ostream& (*fn)(std::ostream&)) {
		(*fn)(m_Buffer);
		return *this;
	}
}  // namespace djinn::core
