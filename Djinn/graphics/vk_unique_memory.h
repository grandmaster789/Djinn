#pragma once

#include "third_party.h"

namespace djinn::graphics {
    struct VkUniqueMemory {
        vk::UniqueBuffer       m_Buffer;
        vk::UniqueDeviceMemory m_Memory;
    };
}
