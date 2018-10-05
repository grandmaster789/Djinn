#include "engine.h"
#include "util/algorithm.h"
#include <fstream>

namespace djinn {
    namespace {
        static const std::string g_SystemConfigFilename("djinn.cfg");
    }

    Engine& Engine::instance() {
        static Engine result;
        return result;
    }

    Engine::Engine() {
        
    }

    void Engine::run() {
        // start by trying to load a config file
        std::ifstream config_stream(g_SystemConfigFilename.c_str());
        if (config_stream.good())
            config_stream >> m_SystemSettings;
        else
            gLogWarning << "No configuration file was found, using defaults for all subsystems";

        // next, try to load the application config
        if (m_Application) {
            std::string applicationName = m_Application->getName();
            std::string applicationConfigFilename = applicationName + std::string(".cfg");
            std::ifstream application_config_stream(applicationConfigFilename.c_str());

            if (application_config_stream.good())
                application_config_stream >> m_ApplicationSettings;
            else
                gLogWarning << "No configuration for application " << applicationName << " was found, using defaults";
        }

        // continue with initializing third-party libraries
        init_external_libraries();

        // and start the main loop
        // [NOTE] the code ends up pretty loopy, perhaps some restructuring is in order?
        m_Running = true;
        while (m_Running) {
            // This is done within the run loop so systems may be
            // enabled while running
            if (m_UninitializedSystems > 0) {
                static int s_prev_count = std::numeric_limits<int>::max();

                if (s_prev_count == m_UninitializedSystems)
                    throw std::runtime_error("Unable to make progress initializing systems; probably a cyclic dependency was introduced");

                s_prev_count = m_UninitializedSystems;
                init_systems();                
            }

            if (m_UninitializedSystems == 0) {
                if ( m_Application && 
                    !m_Application->isInitialized()
                )
                    init_application();

                for (auto& system: m_Systems)
                    system->update();

                if (m_Application) {
                    if (m_Application->isInitialized())
                        m_Application->update();
                }
                else {
                    gLog << "No application was set, going into unit test mode";
                    for (auto& system: m_Systems)
                        system->unittest();

                    gLog << "Unit test complete";
                    stop();
                }
            }
        }

        // done with the main loop, cleanup
        shutdown_application();
        shutdown_systems();
        shutdown_external_libraries();
    }

    void Engine::stop() {
        m_Running = false;
    }

    namespace {
        bool is_satisfied(
            const std::vector<std::string>& dependencies, 
            const std::vector<std::string>& available
        ) {
            return util::contains_all(available, dependencies);
        }
    }

    void Engine::init_systems() {
        // single pass over all the systems,
        // initializing anything that has its
        // dependencies satisfied

        if (m_UninitializedSystems == 0)
            return;

        for (auto& system : m_Systems) {
            if (!system->isInitialized()) {
                if (is_satisfied(system->getDependencies(), m_InitOrder)) {
                    // look up settings specific to this system
                    auto it = m_SystemSettings.find(system->getName());

                    if (it != m_SystemSettings.end()) {
                        for (const auto& entry : system->getSettings())
                            entry.m_DeserializeFn(*it);
                    }
                    else
                        gLogWarning << "No configuration for system " << system->getName() << " was found, using defaults";

                    try {
                        system->m_Engine = this;
                        system->init();

                        m_InitOrder.push_back(system->getName());
                        --m_UninitializedSystems;
                    }
                    catch (std::exception& ex) {
                        gLogError << ex.what();

                        system->m_Engine = nullptr;
                    }
                    catch (...) {
                        system->m_Engine = nullptr;
                    }
                }
            }
        }
    }

    void Engine::shutdown_systems() {
        for (
            auto it  = m_InitOrder.crbegin();
            it != m_InitOrder.crend();
            ++it
        ) {
            auto systemName = *it;
            auto jt = util::find_if(
                m_Systems, 
                [systemName](const SystemPtr& ptr) {
                    return ptr->getName() == systemName;
                }
            );

            // allow the system to clean up
            (*jt)->shutdown();

            // collect the systems' settings
            nlohmann::json settings;
            for (const auto& entry: (*jt)->getSettings()) {
                auto serializer = entry.m_SerializeFn();
                assert(serializer.size() == 1);

                settings.emplace(
                    serializer.begin().key(),
                    *serializer.begin()
                );
            }

            // merge with the other system settings
            m_SystemSettings[systemName] = settings;
        }

        m_SystemMap.clear();
        m_Systems.clear();

        {
            // save system settings to disk
            std::ofstream out(g_SystemConfigFilename.c_str());
            if (!out.good())
                gLogError << "Failed to open " << g_SystemConfigFilename << ", discarding settings";
            else
                out << m_SystemSettings.dump(4);
        }
    }

    void Engine::init_application() {
        // finally, if 'all' systems are initialized
        // check and see if the current application can be started
        // [NOTE] the application has its configuration separate from the systems
        if (
            (m_Application) &&
            (m_Application->m_Engine == nullptr) &&
            (is_satisfied(m_Application->getDependencies(), m_InitOrder))
            ) {
            for (const auto& entry : m_Application->getSettings())
                entry.m_DeserializeFn(m_ApplicationSettings);

            m_Application->m_Engine = this;
            m_Application->init();
        }
    }

    void Engine::shutdown_application() {
        if (
            m_Application &&
            m_Application->isInitialized()
        ) {
            // allow the application to clean up
            m_Application->shutdown();

            // save the application settings
            for (const auto& entry : m_Application->getSettings()) {
                auto serializer = entry.m_SerializeFn();
                assert(serializer.size() == 1);

                m_ApplicationSettings.emplace(
                    serializer.begin().key(),
                    *serializer.begin()
                );
            }

            std::string   applicationName = m_Application->getName();
            std::string   applicationConfigFilename = applicationName  + std::string(".cfg");
            std::ofstream out(applicationConfigFilename.c_str());

            if (!out.good())
                gLogError << "Failed to open " << applicationConfigFilename << ", discarding settings";
            else
                out << m_ApplicationSettings.dump(4);

			m_Application.reset(); // and we're done
        }
    }

    namespace {
        std::string glfwErrorCode(int code) {
            switch (code) {
            case GLFW_NOT_INITIALIZED:     return "GLFW not initialized";
            case GLFW_NO_CURRENT_CONTEXT:  return "No current context";
            case GLFW_INVALID_ENUM:        return "Invalid enum";
            case GLFW_INVALID_VALUE:       return "Invalid value";
            case GLFW_OUT_OF_MEMORY:       return "Out of memory";
            case GLFW_API_UNAVAILABLE:     return "API unavailable";
            case GLFW_VERSION_UNAVAILABLE: return "Version unavailable";
            case GLFW_PLATFORM_ERROR:      return "Platform error";
            case GLFW_FORMAT_UNAVAILABLE:  return "Format unavailable";
            case GLFW_NO_WINDOW_CONTEXT:   return "No window context";
            default:
                return "Unknown GLFW error";
            }
        }

        void glfwErrorCallback(int code, const char* description) {
            gLogError << glfwErrorCode(code) << ": " << description;
        }
    }

    void Engine::init_external_libraries() {
        {
            // GLFW
            glfwInit();
            glfwSetErrorCallback(glfwErrorCallback);

            gLogDebug << "GLFW v" << glfwGetVersionString();
        }
    }

    void Engine::shutdown_external_libraries() {
        {
            glfwTerminate();
        }
    }
}