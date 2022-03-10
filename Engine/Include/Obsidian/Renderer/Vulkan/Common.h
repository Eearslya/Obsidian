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
} VulkanFunctions;

typedef struct VulkanContextT {
	VulkanFunctions vk;
	VkAllocationCallbacks Allocator;

	VkInstance Instance;
} VulkanContext;
