#include "vk_ostream.h"
#include <ostream>

namespace vk {
    std::ostream& operator<<(std::ostream& os, PhysicalDeviceType pdt) {
        // clang-format off
        switch (pdt) {
        case vk::PhysicalDeviceType::eOther:         os << "other";          break;
        case vk::PhysicalDeviceType::eIntegratedGpu: os << "integrated GPU"; break;
        case vk::PhysicalDeviceType::eDiscreteGpu:   os << "discrete GPU";   break;
        case vk::PhysicalDeviceType::eVirtualGpu:    os << "virtual GPU";    break;
        case vk::PhysicalDeviceType::eCpu:           os << "CPU";            break;
        default: 
            throw std::runtime_error("Unsupported physical device type");
        }
        // clang-format on

        return os;
    }
}  // namespace vk
