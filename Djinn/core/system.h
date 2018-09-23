#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <functional>

#include "dependencies.h"

namespace djinn {
    class Engine;
}

namespace djinn::core {
    class System {
    private:
    	struct VariableEntry {
             using SerializeFn   = std::function<nlohmann::json()>;
             using DeserializeFn = std::function<void(const nlohmann::json&)>;
             
    		std::string   m_VariableName;  // just for debugging
             SerializeFn   m_SerializeFn;   // runtime -> JSON
             DeserializeFn m_DeserializeFn; // JSON -> runtime
    	};
    
    public:
    	friend class Engine;
    	friend class Settings;
    
    	using Dependencies = std::vector<std::string>;
    	using Settings     = std::vector<VariableEntry>;
    
    	System(const std::string& systemName);
    	virtual ~System() = default;
    
    	System             (const System&) = delete;
    	System& operator = (const System&) = delete;
    	System             (System&&)      = default;
    	System& operator = (System&&)      = default;
    
    	virtual void init();
    	virtual void update();
    	virtual void shutdown();
    
         virtual void unittest() = 0;
    
    	const std::string&  getName()         const;
    	const Dependencies& getDependencies() const;
    	const Settings&     getSettings()     const;
         bool                isInitialized()   const;
    
    protected:
    	void addDependency(const std::string& systemName);
    
    	template <typename T>
    	void registerSetting(const std::string& jsonKey, T* variable);
    
    	Engine* m_Engine = nullptr;
    
    private:
    	std::string  m_Name;
    	Dependencies m_Dependencies;
    	Settings     m_Settings;
    };
    
    std::ostream& operator << (std::ostream& os, const System& s);
}

#include "system.inl"
