#pragma once

#include "third_party.h"

namespace djinn::graphics {
    class VkDebug {
    public:
        VkDebug(const vk::Instance& instance);

    private:
        vk::Instance                     m_Instance;
        vk::UniqueDebugReportCallbackEXT m_DebugReportCallback;
    };
}