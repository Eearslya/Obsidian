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
