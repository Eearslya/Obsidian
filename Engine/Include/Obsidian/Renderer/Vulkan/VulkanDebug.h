/** @file
 *  @brief Vulkan debugging utilities */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

/**
 * Populate the given structure with information to set up debugging callbacks.
 * @param info A pointer to a debug messenger create info structure.
 */
void VulkanDebug_PopulateMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT* info);

/**
 * Create the debug messenger.
 * @param context The active Vulkan context.
 * @return The result of creating the debug messenger.
 */
VkResult VulkanDebug_CreateMessenger(VulkanContext* context);

/**
 * Destroy the debug messenger.
 * @param context The active Vulkan context.
 */
void VulkanDebug_DestroyMessenger(VulkanContext* context);
