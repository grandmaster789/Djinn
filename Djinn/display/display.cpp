#include "display.h"

// ------ Vulkan debug reports ------
namespace {
    PFN_vkCreateDebugReportCallbackEXT  pfn_vkCreateDebugReportCallbackEXT = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT = nullptr;

    using string        = std::string;
    using ExtensionList = std::vector<vk::ExtensionProperties>;
    using LayerList     = std::vector<vk::LayerProperties>;

    bool isExtensionAvailable(
        const string&        name,
        const ExtensionList& availableExtensions
    ) {
        using djinn::util::contains_if;
        using vk::ExtensionProperties;

        return contains_if(
            availableExtensions,
            [&](const ExtensionProperties& extension) {
                return (name == extension.extensionName);
            }
        );
    }

    bool isLayerAvailable(
        const string&    name,
        const LayerList& availableLayers
    ) {
        using djinn::util::contains_if;
        using vk::LayerProperties;

        return contains_if(
            availableLayers,
            [&](const LayerProperties& layer) {
              return (name == layer.layerName);
            }
        );
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL report_to_log(
        VkDebugReportFlagsEXT      flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t                   object,
        size_t                     location,
        int32_t                    code,
        const char*                layerPrefix,
        const char*                message,
        void*                      userdata
    ) {
        (void)objectType;
        (void)object;
        (void)location;
        (void)userdata;

             if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)               gLogDebug << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)             gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)               gLogError << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)         gLog << "[" << layerPrefix << "] Code " << code << " : " << message;

        return VK_FALSE;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugReportCallbackEXT*                 pCallback
) {
    if (pfn_vkCreateDebugReportCallbackEXT)
        return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    else
        // [NOTE] this is expected to be nothrow, noexcept
        //        so use an error code instead
        return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                   instance,
    VkDebugReportCallbackEXT     callback,
    const VkAllocationCallbacks* pAllocator
) {
    if (pfn_vkDestroyDebugReportCallbackEXT)
        pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    // [NOTE] this is expected to be nothrow, noexcept    
    //        so silent fail because of the void return type
}

namespace djinn {
    Display::Display():
        System("Display")
    {
    }

    void Display::init() {
        System::init();

    }

    void Display::update() {
        
    }

    void Display::shutdown() {
        System::shutdown();
    }

    void Display::unittest() {
    }
}