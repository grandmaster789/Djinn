#pragma once

#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "third_party.h"

namespace djinn {
    class Engine;
}

namespace djinn::core {
    // mwh.. considering renaming 'System' to 'EngineModule' or something
    class System {
    private:
        struct VariableEntry {
            using SerializeFn   = std::function<nlohmann::json()>;
            using DeserializeFn = std::function<void(const nlohmann::json&)>;

            std::string   m_VariableName;   // just for debugging
            SerializeFn   m_SerializeFn;    // runtime -> JSON
            DeserializeFn m_DeserializeFn;  // JSON -> runtime
        };

    public:
        friend class Engine;
        friend class Settings;

        using Dependencies = std::vector<std::string>;
        using Settings     = std::vector<VariableEntry>;

        System(const std::string& systemName);
        virtual ~System() = default;

        System(const System&) = delete;
        System& operator=(const System&) = delete;
        System(System&&)                 = default;
        System& operator=(System&&) = default;

        virtual void init();
        virtual void update();
        virtual void shutdown();

        const std::string&  getName() const;
        const Dependencies& getDependencies() const;
        const Settings&     getSettings() const;
        bool                isInitialized() const;

        Engine* getEngine() const;

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

    std::ostream& operator<<(std::ostream& os, const System& s);
}  // namespace djinn::core

#include "system.inl"
