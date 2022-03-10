/** @file
 *  @brief Platform-specific functions to integrate with the Vulkan render engine */
#pragma once

#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Defines.h>

/**
 * Retrieve the list of instance extensions required to work with the current platform.
 * @param extensionNames A DynArray of strings that will be appended to.
 */
void Platform_Vulkan_GetRequiredInstanceExtensions(DynArrayT extensionNames);
