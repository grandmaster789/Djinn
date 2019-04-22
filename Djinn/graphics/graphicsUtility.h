#pragma once

#include <cstdint>
#include <vector>

#include "third_party.h"

namespace djinn::graphics {
    bool areInstanceLayersAvailable    (const std::vector<const char*>& names);
    bool areInstanceExtensionsAvailable(const std::vector<const char*>& names);
    bool areDeviceExtensionsAvailable  (const std::vector<const char*>& names, vk::PhysicalDevice gpu);

    uint32_t selectMemoryTypeIndex(
        vk::PhysicalDevice      gpu,
        uint32_t                typeBits,
        vk::MemoryPropertyFlags properties
    );
}
