#pragma once

#include "app/application.h"
#include "system.h"
#include "util/typemap.h"
#include <memory>
#include <vector>
#include <atomic>

namespace djinn {
    class Engine {
    public:
        using ApplicationPtr = std::unique_ptr<app::Application>;
        using SystemPtr      = std::unique_ptr<core::System>;
        using SystemList     = std::vector<SystemPtr>;

        static Engine& instance();

    private:
        Engine();

    public:
        ~Engine() = default;

        Engine             (const Engine&) = delete;
        Engine& operator = (const Engine&) = delete;
        Engine             (Engine&&)      = delete;
        Engine& operator = (Engine&&)      = delete;

        // [NOTE] systems must be semi-unique (only one instance of each enabled at any given time)
        template <typename T, typename... tArgs>
        void enable(tArgs... args);

        template <typename T>
        void disable();

        template <typename T>
        T* get() const; // will return nullptr if not found

        // [NOTE] only one application may be active at any given time
        template <typename T, typename... tArgs>
        void setApplication(tArgs... args);

        void run();
        void stop();

    private:
        void init_external_libraries();
        void shutdown_external_libraries();

        void init_systems();
        void shutdown_systems();

        void init_application();
        void shutdown_application();

		core::System* find(const std::string& name) const;

        SystemList     m_Systems;
        util::TypeMap  m_SystemMap;
        ApplicationPtr m_Application;

        nlohmann::json m_SystemSettings;
        nlohmann::json m_ApplicationSettings;

        std::atomic_bool m_Running              = false;
        int              m_UninitializedSystems = 0;

        std::vector<std::string> m_InitOrder;
    };
}

#include "engine.inl"