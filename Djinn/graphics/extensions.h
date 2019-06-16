#pragma once

#include "third_party.h"

namespace djinn::graphics {
	void loadInstanceExtensions(vk::Instance instance);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugReportCallbackEXT*                 pCallback);

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                   instance,
    VkDebugReportCallbackEXT     callback,
    const VkAllocationCallbacks* pAllocator);
