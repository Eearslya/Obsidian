#include <Obsidian/Core/Logger.h>
#include <Obsidian/Renderer/Vulkan/VulkanDebug.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void* pUserData) {
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LogE("[Vulkan] ERROR: %s", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LogW("[Vulkan] WARNING: %s", pCallbackData->pMessage);
			break;
		default:
			LogD("[Vulkan] %s", pCallbackData->pMessage);
			break;
	}

	return VK_FALSE;
}

void VulkanDebug_PopulateMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT* info) {
	info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info->pNext = NULL;
	info->flags = 0;
	info->messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
	                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	info->pfnUserCallback = VulkanDebug_Callback;
	info->pUserData       = NULL;
}

VkResult VulkanDebug_CreateMessenger(VulkanContext* context) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	VulkanDebug_PopulateMessengerInfo(&createInfo);

	if (context->vk.CreateDebugUtilsMessengerEXT == NULL) { return VK_ERROR_EXTENSION_NOT_PRESENT; }

	const VkResult createResult = context->vk.CreateDebugUtilsMessengerEXT(
		context->Instance, &createInfo, &context->Allocator, &context->DebugMessenger);

	if (createResult == VK_SUCCESS) { LogD("[Vulkan] Debug messenger created."); }

	return createResult;
}

void VulkanDebug_DestroyMessenger(VulkanContext* context) {
	if (context->vk.DestroyDebugUtilsMessengerEXT) {
		context->vk.DestroyDebugUtilsMessengerEXT(context->Instance, context->DebugMessenger, &context->Allocator);
	}
}
