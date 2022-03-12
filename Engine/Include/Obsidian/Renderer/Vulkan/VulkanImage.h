/** @file
 *  @brief Vulkan images */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Vulkan/Common.h>

/** Information required to create a Vulkan image. */
typedef struct VulkanImage_CreateInfoT {
	VkImageType Type;        /**< The type of the image. */
	VkFormat Format;         /**< The format of the image. */
	VkExtent3D Extent;       /**< The size of the image in pixels. */
	VkImageUsageFlags Usage; /**< The usage flags that dictate how this image will be used. */
} VulkanImage_CreateInfo;

/**
 * Create a Vulkan image with the information specified.
 * @param context The active Vulkan context.
 * @param createInfo The information to use when creating the image.
 * @param image A pointer to where the new image information will be stored.
 * @return The result of the image creation.
 */
VkResult VulkanImage_Create(VulkanContext* context, const VulkanImage_CreateInfo* createInfo, VulkanImage* image);

/**
 * Destroy an existing Vulkan image.
 * @param context The active Vulkan context.
 * @param image The image to destroy.
 */
void VulkanImage_Destroy(VulkanContext* context, VulkanImage* image);

/**
 * Fill in an image creation structure for images that will only be read from (e.g. textures).
 * @param format The format to create the image with.
 * @param extent The size of the image in pixels.
 * @return The necessary creation info.
 */
VulkanImage_CreateInfo VulkanImageCI_Immutable2D(VkFormat format, VkExtent2D extent);