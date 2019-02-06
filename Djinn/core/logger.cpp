#include "logger.h"
#include "log_sink.h"

namespace djinn::core {
    Logger::Logger(const std::string& filename) {
    	add(makeConsoleSink());
    	add(makeFileSink(filename));

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
		add(makeWindowsConsoleSink());
#endif
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
    
    	auto it = find(begin(m_Sinks), end(m_Sinks), sink);
    
    	if (it == end(m_Sinks))
    		m_Sinks.push_back(std::forward<LogSink>(sink));
    	// else // silently fail if the sink is already present
    }
    
    void Logger::remove(LogSink sink) {
    	using namespace std;
    
    	auto it = find(begin(m_Sinks), end(m_Sinks), sink);
    
    	if (it != end(m_Sinks))
    		m_Sinks.erase(it);
    	// else // silently fail if the sink is not actually present
    }
    
    void Logger::removeAll() {
    	m_Sinks.clear();
    }
    
    size_t Logger::getNumSinks() const {
    	return m_Sinks.size();
    }
    
    void Logger::flush(const LogMessage* message) {
    	std::string msg = message->m_Buffer.str(); // collapse the buffer into a regular string
    
    	for (auto& sink : m_Sinks)
    		sink.write(message->m_MetaInfo, msg);
    }
    
    Logger& Logger::instance() {
    	static Logger gLogger("djinn.log");
    	return gLogger;
    }
}