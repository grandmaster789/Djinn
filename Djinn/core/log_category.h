#pragma once

#include <ostream>

namespace djinn::core {
	enum class eLogCategory {
		DEBUG,
		MESSAGE,
		WARNING,
		ERROR_,  // ERROR is a pretty common macro, so that's off the table
		FATAL
	};

	// global log level functions
	void enableAllLogCategories();
	void disableAllLogCategories();
	void setGlobalLogCategory(eLogCategory category, bool enabled);
	bool isGlobalLogCategoryEnabled(eLogCategory category);

	std::ostream& operator<<(std::ostream& os, const eLogCategory& category);
}  // namespace djinn::core
