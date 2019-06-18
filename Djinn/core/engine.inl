#pragma once

#include "engine.h"
#include <type_traits>

namespace djinn {
    template <typename T, typename... tArgs>
    void Engine::enable(tArgs... args) {
        static_assert(
            std::is_base_of_v<core::System, T>,
            "Only objects that inherit from System may be enabled");

        auto s = std::make_unique<T>(std::forward<tArgs>(args)...);
        m_SystemMap.insert(s.get());
        m_Systems.push_back(std::move(s));

        ++m_UninitializedSystems;
    }

    template <typename T>
    void Engine::disable() {
        using namespace std;

        T* raw = m_SystemMap.get<T>();

        if (raw == nullptr)
            throw std::runtime_error("System not found");

        auto it = find_if(begin(m_Systems), end(m_Systems), [raw](const SystemPtr& ptr) {
            return (ptr.get() == raw);
        });

        if (it == end(m_Systems))
            throw std::runtime_error("System list and mapping are out of sync");

        if (!raw->isInitialized())
            --m_UninitializedSystems;  // disabled before it was initialized

        m_SystemMap.remove<T>();
        m_Systems.erase(it);
    }

    template <typename T>
    T* Engine::get() const {
        return m_SystemMap.get<T>();
    }

    template <typename T, typename... tArgs>
    void Engine::setApplication(tArgs... args) {
        if (m_Application) {
            gLog << "Switching application";
            m_Application->shutdown();
        }

        m_Application = std::make_unique<T>(std::forward<tArgs>(args)...);
    }
}  // namespace djinn
