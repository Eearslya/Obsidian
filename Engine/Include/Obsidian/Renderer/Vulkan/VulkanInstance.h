/** @file
 *  @brief VkInstance and related functions */
#pragma once

#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

/**
 * Create a VkInstance with the specified extensions required.
 * @param context The active Vulkan context.
 * @param instanceExtensionsArray A const char* DynArray with the names of required instance extensions.
 * @return The result of the instance creation.
 */
VkResult VulkanInstance_Create(VulkanContext* context, ConstDynArrayT instanceExtensionsArray);

/**
 * Destroy a VkInstance.
 * @param context The active Vulkan context.
 * @param instance The instance to destroy.
 */
void VulkanInstance_Destroy(VulkanContext* context);
