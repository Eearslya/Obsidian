/** @file
 *  @brief Helper functions to convert Vulkan enums and flags to strings */
#pragma once

#include <Obsidian/Defines.h>
#include <vulkan/vulkan.h>

/**
 * Convert a VkResult into a string.
 * @param value The VkResult to convert.
 * @return A string representation of the result.
 */
const char* VulkanString_VkResult(VkResult value);
