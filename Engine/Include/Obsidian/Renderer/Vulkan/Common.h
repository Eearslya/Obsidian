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
	PFN_vkCreateDevice CreateDevice;
	PFN_vkDestroyDevice DestroyDevice;
	PFN_vkDestroyInstance DestroyInstance;
	PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
	PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
	PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
	PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;

	// Device Functions
	PFN_vkAllocateMemory AllocateMemory;
	PFN_vkBindImageMemory BindImageMemory;
	PFN_vkCreateImage CreateImage;
	PFN_vkDestroyImage DestroyImage;
	PFN_vkFreeMemory FreeMemory;
	PFN_vkGetDeviceQueue GetDeviceQueue;
	PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;

	// VK_KHR_surface
	PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;

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

typedef struct PhysicalDeviceInfo {
	VkPhysicalDevice GPU;
	VkExtensionProperties* Extensions;
	VkPhysicalDeviceFeatures Features;
	VkPhysicalDeviceMemoryProperties Memory;
	VkPhysicalDeviceProperties Properties;
	VkQueueFamilyProperties* QueueFamilies;
	U32 GraphicsFamily;
	U32 GraphicsIndex;
	U32 TransferFamily;
	U32 TransferIndex;
	U32 ComputeFamily;
	U32 ComputeIndex;
	U32 AsyncGraphicsFamily;
	U32 AsyncGraphicsIndex;
} PhysicalDeviceInfo;

typedef struct VulkanImageT {
	VkImage Image;
	VkDeviceMemory Memory;
	VkImageCreateInfo CreateInfo;
} VulkanImage;

typedef struct VulkanContextT {
	VulkanFunctions vk;
	VkAllocationCallbacks Allocator;
	B8 Validation;

	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR Surface;
	VkPhysicalDevice PhysicalDevice;
	PhysicalDeviceInfo DeviceInfo;
	VkDevice Device;
	VkQueue Graphics;
	VkQueue Compute;
	VkQueue Transfer;
	VkQueue AsyncGraphics;
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
