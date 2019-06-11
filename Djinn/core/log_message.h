#pragma once

#include <ostream>
#include <sstream>
#include <string>

#include "log_category.h"

namespace djinn::core {
	class Logger;

	// LogMessage holds a string buffer, which accumulates a message iostream style
	// and is flushed to the parent Logger object during destruction
	// This allows automatically adding prefix/postfix strings at each point
	class LogMessage {
	private:
		friend class Logger;  // only allow Logger objects to construct a LogMessage object

		LogMessage(
		    Logger*            owner,
		    eLogCategory       category,
		    const std::string& sourceFile,
		    int                sourceLineNumber);

		LogMessage(const LogMessage&) = delete;
		LogMessage& operator=(const LogMessage&) = delete;
		LogMessage(LogMessage&&)                 = default;
		LogMessage& operator=(LogMessage&&) = default;

	public:
		~LogMessage();

		// accumulate values, iostream style
		// [NOTE] defaults to boolalpha on
		template <typename T>
		LogMessage& operator<<(const T& value);

		// apply iostream manipulator functions (such as std::endl and std::boolalpha)
		LogMessage& operator<<(std::ostream& (*fn)(std::ostream&));

		// MetaInfo is public so that any LogSink may make use of it
		struct MetaInfo {
			eLogCategory m_Category;
			std::string  m_SourceFile;
			int          m_SourceLine;
		};

	private:
		std::ostringstream m_Buffer;
		Logger*            m_Owner;
		MetaInfo           m_MetaInfo;
	};
}  // namespace djinn::core

#include "log_message.inl"
