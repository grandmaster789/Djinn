#include "extensions.h"

namespace {
	PFN_vkCreateDebugReportCallbackEXT  pfn_vkCreateDebugReportCallbackEXT  = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT = nullptr;
}  // namespace

namespace djinn::graphics {
	void loadInstanceExtensions(vk::Instance instance) {
		pfn_vkCreateDebugReportCallbackEXT
		    = (PFN_vkCreateDebugReportCallbackEXT)instance.getProcAddr(
		        "vkCreateDebugReportCallbackEXT");

		pfn_vkDestroyDebugReportCallbackEXT
		    = (PFN_vkDestroyDebugReportCallbackEXT)instance.getProcAddr(
		        "vkDestroyDebugReportCallbackEXT");
	}
}  // namespace djinn::graphics

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugReportCallbackEXT*                 pCallback) {
	if (pfn_vkCreateDebugReportCallbackEXT)
		return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
	else
		// [NOTE] this is expected to be nothrow + noexcept
		//        so use an error code instead
		return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                   instance,
    VkDebugReportCallbackEXT     callback,
    const VkAllocationCallbacks* pAllocator) {
	if (pfn_vkDestroyDebugReportCallbackEXT)
		pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);

	// [NOTE] this is expected to be nothrow + noexcept
	//        so silent fail because of the void return type
}
