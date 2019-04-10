#include "vk_debug.h"
#include "core/logger.h"

namespace {
    // Perhaps some kind of vulkan extension loader can be used to get these?
    PFN_vkCreateDebugReportCallbackEXT  pfn_vkCreateDebugReportCallbackEXT  = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT = nullptr;

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

             if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)               gLogDebug   << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)             gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)               gLogError   << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) gLogWarning << "[" << layerPrefix << "] Code " << code << " : " << message;
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)         gLog        << "[" << layerPrefix << "] Code " << code << " : " << message;

        return VK_FALSE;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                          instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugReportCallbackEXT*           pCallback
) {
    if (pfn_vkCreateDebugReportCallbackEXT)
        return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    else
        // [NOTE] this is expected to be nothrow + noexcept
        //        so use an error code instead
        return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance               instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks*   pAllocator
) {
    if (pfn_vkDestroyDebugReportCallbackEXT)
        pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);

    // [NOTE] this is expected to be nothrow + noexcept    
    //        so silent fail because of the void return type
}

namespace djinn::graphics {
    VkDebug::VkDebug(const vk::Instance& instance) :
        m_Instance(instance)
    {
        if (!pfn_vkCreateDebugReportCallbackEXT) {
            pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)instance.getProcAddr("vkCreateDebugReportCallbackEXT");

            if (!pfn_vkCreateDebugReportCallbackEXT)
                throw std::runtime_error("Failed to set vkCreateDebugReportCallbackEXT");
        }

        if (!pfn_vkDestroyDebugReportCallbackEXT) {
            pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)instance.getProcAddr("vkDestroyDebugReportCallbackEXT");

            if (!pfn_vkDestroyDebugReportCallbackEXT)
                throw std::runtime_error("Failed to set vkDestroyDebugReportCallbackEXT");
        }

        vk::DebugReportCallbackCreateInfoEXT info = {};

        info
            .setFlags(
                vk::DebugReportFlagBitsEXT::eDebug |
                vk::DebugReportFlagBitsEXT::eError |
                //vk::DebugReportFlagBitsEXT::eInformation | // this one is kinda spammy
                vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                vk::DebugReportFlagBitsEXT::eWarning
            )
            .setPfnCallback(report_to_log);

        m_DebugReportCallback = m_Instance.createDebugReportCallbackEXTUnique(info);
    }
}