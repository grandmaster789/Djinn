#include "log_category.h"
#include <atomic>

namespace djinn::core {
    namespace {
        std::atomic<bool> g_LogCategoryEnabled[5]{
            {true},  // DEBUG
            {true},  // MESSAGE
            {true},  // WARNING
            {true},  // ERROR_
            {true}   // FATAL
        };

        int selectLogState(eLogCategory category) {
            switch (category) {
            case eLogCategory::DEBUG: return 0;
            case eLogCategory::MESSAGE: return 1;
            case eLogCategory::WARNING: return 2;
            case eLogCategory::ERROR_: return 3;
            case eLogCategory::FATAL: return 4;
            default: throw std::runtime_error("Unsupported log category");
            }
        }
    }  // namespace

    void enableAllLogCategories() {
        for (auto& cat : g_LogCategoryEnabled)
            cat.store(true);
    }

    void disableAllLogCategories() {
        for (auto& cat : g_LogCategoryEnabled)
            cat.store(false);
    }

    void setGlobalLogCategory(eLogCategory category, bool enabled) {
        int idx = selectLogState(category);
        g_LogCategoryEnabled[idx].store(enabled);
    }

    bool isGlobalLogCategoryEnabled(eLogCategory category) {
        int idx = selectLogState(category);
        return g_LogCategoryEnabled[idx].load();
    }

    std::ostream& operator<<(std::ostream& os, const eLogCategory& category) {
        // Chosen so you'll have an easy time spotting them in the actual log

        // clang-format off
		switch (category) {
		case eLogCategory::DEBUG:   os << "[dbg] ";         break;
		case eLogCategory::MESSAGE: os << "      ";         break;  // this should be the majority during debugging. Should be very non-conspicous
		case eLogCategory::WARNING: os << "*wrn* ";         break;
		case eLogCategory::ERROR_:  os << "< ERROR >     "; break;  // extra noticable
		case eLogCategory::FATAL:   os << "<## FATAL ##> "; break;  // extra noticable
		}
        // clang-format on

        return os;
    }
}  // namespace djinn::core
