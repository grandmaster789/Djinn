#include "system.h"
#include "logger.h"
#include "util/algorithm.h"

namespace djinn::core {
	System::System(const std::string& name): m_Name(name) {}

	void System::init() {
		gLog << "Initializing [" << getName() << "]";
	}

	void System::update() {}

	void System::shutdown() {
		gLog << "Shutting down [" << getName() << "]";
	}

	const std::string& System::getName() const {
		return m_Name;
	}

	const System::Dependencies& System::getDependencies() const {
		return m_Dependencies;
	}

	void System::addDependency(const std::string& systemName) {
		using namespace std;

		if (!util::contains(m_Dependencies, systemName))
			m_Dependencies.push_back(systemName);
		else
			gLogWarning << getName() << " ~ ignoring duplicate dependency: " << systemName;
	}

	const System::Settings& System::getSettings() const {
		return m_Settings;
	}

	bool System::isInitialized() const {
		return (m_Engine != nullptr);
	}

	Engine* System::getEngine() const {
		return m_Engine;
	}

	std::ostream& operator<<(std::ostream& os, const System& s) {
		os << s.getName() << "\n";

		os << "Dependencies: \n";
		for (const auto& dep : s.getDependencies()) os << "\t" << dep << "\n";

		os << "Settings: \n";
		for (const auto& setting : s.getSettings())
			os << "\t" << setting.m_SerializeFn().dump() << "\n";  // should print { "key": value }

		return os;
	}
}  // namespace djinn::core
