#pragma once

#include "system.h"
#include "logger.h"

namespace djinn {
	namespace core {
		template <typename T>
		void System::registerSetting(const std::string& jsonKey, T* variable) {
            VariableEntry entry;

			// [NOTE] this does not check for duplicates yet
			entry.m_SerializeFn = [=] {
				nlohmann::json result;
				result[jsonKey] = *variable;
				return result;
			};

			entry.m_DeserializeFn = [=](const nlohmann::json& settings) {
				auto it = settings.find(jsonKey);

				if (it != settings.end())
					*variable = *it;
				else {
					gLogDebug << jsonKey << "\n" << settings;
					gLogWarning << "Setting not found: " << getName() << ":" << jsonKey;
				}
			};

			m_Settings.push_back(std::move(entry));
		}
	}
}