/** @file
 *  @brief Platform-specific functions to integrate with the Vulkan render engine */
#pragma once

#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Defines.h>

struct PlatformStateT;
struct VulkanContextT;

/**
 * Retrieve the list of instance extensions required to work with the current platform.
 * @param extensionNames A DynArray of strings that will be appended to.
 */
void Platform_Vulkan_GetRequiredInstanceExtensions(DynArrayT extensionNames);

/**
 * Create a Vulkan surface for the current platform.
 * @param platform The current platform state.
 * @param context The current Vulkan context.
 * @return TRUE upon success, FALSE on failure.
 */
B8 Platform_Vulkan_CreateSurface(struct PlatformStateT* platform, struct VulkanContextT* context);
