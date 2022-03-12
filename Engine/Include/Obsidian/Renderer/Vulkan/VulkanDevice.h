/** @file
 *  @brief Vulkan physical and logical devices */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

/**
 * Select our physical device and create the logical device.
 * @param context The active Vulkan context.
 * @return The result of the device creation.
 */
VkResult VulkanDevice_Create(VulkanContext* context);

/**
 * Destroy the Vulkan device.
 * @param context The active Vulkan context.
 */
void VulkanDevice_Destroy(VulkanContext* context);

/**
 * Return a valid memory type for an allocation with the given requirements.
 * @param context The active Vulkan context.
 * @param typeFilter A bitmask of valid memory types, as returned by vkGet*MemoryRequirements.
 * @param properties A bitmask of valid memory properties.
 * @return The memory type index to use, or -1 if no memory type could be found.
 */
I32 VulkanDevice_FindMemoryType(VulkanContext* context, U32 typeFilter, VkMemoryPropertyFlags properties);