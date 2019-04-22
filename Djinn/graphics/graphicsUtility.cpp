#include "graphicsUtility.h"
#include "util/algorithm.h"

namespace {
    using LayerList = std::vector<vk::LayerProperties>;
    using ExtensionList = std::vector<vk::ExtensionProperties>;

    bool isExtensionAvailable(
        const std::string& name,
        const ExtensionList& availableExtensions
    ) {
        using djinn::util::contains_if;
        using vk::ExtensionProperties;

        return contains_if(
            availableExtensions,
            [&](const ExtensionProperties & extension) {
                return (name == extension.extensionName);
            }
        );
    }

    bool isLayerAvailable(
        const std::string& name,
        const LayerList& availableLayers
    ) {
        using djinn::util::contains_if;
        using vk::LayerProperties;

        return contains_if(
            availableLayers,
            [&](const LayerProperties & layer) {
                return (name == layer.layerName);
            }
        );
    }
}

namespace djinn::graphics {
    bool areInstanceLayersAvailable(const std::vector<const char*>& names) {
        auto availableInstanceLayerProperies = vk::enumerateInstanceLayerProperties();

        // verify the required layers/extensions are available
        for (const auto& layerName : names)
            if (!isLayerAvailable(layerName, availableInstanceLayerProperies))
                return false;

        return true;
    }

    bool areInstanceExtensionsAvailable(const std::vector<const char*>& names) {
        auto availableInstanceExtensions = vk::enumerateInstanceExtensionProperties();

        for (const auto& extensionName : names)
            if (!isExtensionAvailable(extensionName, availableInstanceExtensions))
                return false;

        return true;
    }

    bool areDeviceExtensionsAvailable(const std::vector<const char*>& names, vk::PhysicalDevice device) {
        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();

        for (const auto& requirement : names)
            if (!isExtensionAvailable(requirement, availableDeviceExtensions))
                return false;

        return true;
    }

    uint32_t selectMemoryTypeIndex(
        vk::PhysicalDevice      gpu,
        uint32_t                typeBits,
        vk::MemoryPropertyFlags properties
    ) {
        auto props = gpu.getMemoryProperties();

        for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
            if ((typeBits & 1) == 1) {
                if ((props.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }

            typeBits >>= 1; // NOTE not entirely sure about this
        }

        return 0;
    }
}