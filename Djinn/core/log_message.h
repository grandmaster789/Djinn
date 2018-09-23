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
    	friend class Logger; // only allow Logger objects to construct a LogMessage object
    
    	LogMessage(
    		Logger* owner, 
    		eLogCategory category, 
    		const std::string& sourceFile, 
    		int sourceLineNumber
    	);
    
    	// no copy, no move
    	LogMessage(const LogMessage&) = delete;
    	LogMessage(LogMessage&&) = default;
    	LogMessage& operator = (const LogMessage&) = delete;
    	LogMessage& operator = (LogMessage&&) = default;
    
    public:
    	~LogMessage();
    
    	template <typename T>
    	LogMessage& operator << (const T& value);						// accumulate values, iostream style
    	LogMessage& operator << (std::ostream& (*fn)(std::ostream&));	// apply iostream manipulator functions (such as std::endl and std::boolalpha)
    
    	// MetaInfo is public so that any LogSink may make use of it
    	struct MetaInfo {
    		eLogCategory mCategory;
    		std::string  mSourceFile;
    		int			 mSourceLine;
    	};
    
    private:
    	std::ostringstream	mBuffer;
    	Logger*				mOwner;
    	MetaInfo			mMetaInfo;
    };
}

#include "log_message.inl"