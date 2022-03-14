#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

VkResult VulkanSwapchain_Create(VulkanContext* context);

void VulkanSwapchain_Destroy(VulkanContext* context);

VkResult VulkanSwapchain_Recreate(VulkanContext* context);