#pragma once

#include "third_party.h"

namespace djinn::graphics {
    struct SwapchainDetails {
        vk::SurfaceCapabilitiesKHR        m_Capabilities = {};
        std::vector<vk::SurfaceFormatKHR> m_Formats;
        std::vector<vk::PresentModeKHR>   m_PresentModes;
    };
}
