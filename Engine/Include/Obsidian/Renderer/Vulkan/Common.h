#pragma once

#include <Obsidian/Defines.h>
#include <vulkan/vulkan.h>

typedef struct VulkanFunctionsT {
	// Global Functions
	PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion;
	PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties;
	PFN_vkCreateInstance CreateInstance;

	// Instance Functions
	PFN_vkDestroyInstance DestroyInstance;

	// VK_EXT_debug_utils
	PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
	PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT;
	PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT;
	PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT;
	PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
	PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT;
	PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;
} VulkanFunctions;

typedef struct VulkanContextT {
	VulkanFunctions vk;
	VkAllocationCallbacks Allocator;

	B8 Validation;
	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
} VulkanContext;

void Vulkan_ReportFailure(const char* expr, VkResult result, const char* msg, const char* file, int line);
#define VkCheck(expr)                                                                      \
	do {                                                                                     \
		const VkResult res = (expr);                                                           \
		if (res != VK_SUCCESS) { Vulkan_ReportFailure(#expr, res, NULL, __FILE__, __LINE__); } \
	} while (0)
#define VkCheckMsg(expr, msg)                                                             \
	do {                                                                                    \
		const VkResult res = (expr);                                                          \
		if (res != VK_SUCCESS) { Vulkan_ReportFailure(#expr, res, msg, __FILE__, __LINE__); } \
	} while (0)
