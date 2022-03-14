/** @file
 *  @brief Vulkan swapchain */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

/**
 * Create the swapchain.
 * @param context The active Vulkan context.
 * @return The result of creating the swapchain.
 */
VkResult VulkanSwapchain_Create(VulkanContext* context);

/**
 * Destroy the swapchain.
 * @param context The active Vulkan context.
 */
void VulkanSwapchain_Destroy(VulkanContext* context);

/**
 * Recreate the swapchain.
 * @param context The active Vulkan context.
 * @return The result of recreating the swapchain.
 */
VkResult VulkanSwapchain_Recreate(VulkanContext* context);
