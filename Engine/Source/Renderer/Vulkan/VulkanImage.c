#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/Vulkan/VulkanDevice.h>
#include <Obsidian/Renderer/Vulkan/VulkanImage.h>
#include <Obsidian/Renderer/Vulkan/VulkanStrings.h>

VkResult VulkanImage_Create(VulkanContext* context, const VulkanImage_CreateInfo* createInfo, VulkanImage* image) {
	Memory_Zero(image, sizeof(VulkanImage));

	// First determine our image's create info
	const VkImageCreateInfo imageCI = {.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	                                   .pNext                 = NULL,
	                                   .flags                 = 0,
	                                   .imageType             = createInfo->Type,
	                                   .format                = createInfo->Format,
	                                   .extent                = createInfo->Extent,
	                                   .mipLevels             = 1,
	                                   .arrayLayers           = 1,
	                                   .samples               = VK_SAMPLE_COUNT_1_BIT,
	                                   .tiling                = VK_IMAGE_TILING_OPTIMAL,
	                                   .usage                 = createInfo->Usage,
	                                   .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
	                                   .queueFamilyIndexCount = 0,
	                                   .pQueueFamilyIndices   = NULL,
	                                   .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

	// Try and create the image
	VkImage vkImage             = VK_NULL_HANDLE;
	const VkResult createResult = context->vk.CreateImage(context->Device, &imageCI, &context->Allocator, &vkImage);
	if (createResult != VK_SUCCESS) {
		LogE("[VulkanImage] Failed to create Vulkan Image! (%s)", VulkanString_VkResult(createResult));
		VulkanImage_Destroy(context, image);

		return createResult;
	}
	image->Image      = vkImage;
	image->CreateInfo = imageCI;

	// Determine what memory type this image requires
	VkMemoryRequirements memoryReq = {};
	context->vk.GetImageMemoryRequirements(context->Device, vkImage, &memoryReq);
	const I32 memoryType =
		VulkanDevice_FindMemoryType(context, memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (memoryType == -1) {
		LogE("[VulkanImage] Failed to find a valid memory type when creating image!");
		VulkanImage_Destroy(context, image);

		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	// Try to allocate the needed memory
	// TODO: Needs a proper allocator that can suballocate larger blocks of memory.
	const VkMemoryAllocateInfo imageAI = {.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                      .pNext           = NULL,
	                                      .allocationSize  = memoryReq.size,
	                                      .memoryTypeIndex = memoryType};
	VkDeviceMemory vkMemory            = VK_NULL_HANDLE;
	const VkResult allocResult = context->vk.AllocateMemory(context->Device, &imageAI, &context->Allocator, &vkMemory);
	if (allocResult != VK_SUCCESS) {
		LogE("[VulkanImage] Failed to allocate memory for image! (%s)", VulkanString_VkResult(allocResult));
		VulkanImage_Destroy(context, image);

		return allocResult;
	}
	image->Memory = vkMemory;

	// Finally, bind the allocated memory to our new image
	const VkResult bindResult = context->vk.BindImageMemory(context->Device, vkImage, vkMemory, 0);
	if (bindResult != VK_SUCCESS) {
		LogE("[VulkanImage] Failed to bind image to memory! (%s)", VulkanString_VkResult(bindResult));
		VulkanImage_Destroy(context, image);

		return bindResult;
	}

	LogD("[Vulkan] Image created.");

	return VK_SUCCESS;
}

void VulkanImage_Destroy(VulkanContext* context, VulkanImage* image) {
	if (image->Memory) {
		context->vk.FreeMemory(context->Device, image->Memory, &context->Allocator);
		image->Memory = VK_NULL_HANDLE;
	}
	if (image->Image) {
		context->vk.DestroyImage(context->Device, image->Image, &context->Allocator);
		image->Image = VK_NULL_HANDLE;
	}
}

VkImageAspectFlags VulkanImage_FormatAspect(VkFormat format) {
	switch (format) {
		case VK_FORMAT_UNDEFINED:
			return 0;

		case VK_FORMAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT;

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return VK_IMAGE_ASPECT_DEPTH_BIT;

		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

VkResult VulkanImageView_Create(VulkanContext* context,
                                const VulkanImageView_CreateInfo* createInfo,
                                VulkanImageView* view) {
	Memory_Zero(view, sizeof(VulkanImageView));

	const VkImageViewCreateInfo viewCI = {.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	                                      .pNext            = NULL,
	                                      .flags            = 0,
	                                      .image            = createInfo->Image->Image,
	                                      .viewType         = createInfo->Type,
	                                      .format           = createInfo->Format,
	                                      .components       = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
	                                                           .g = VK_COMPONENT_SWIZZLE_IDENTITY,
	                                                           .b = VK_COMPONENT_SWIZZLE_IDENTITY,
	                                                           .a = VK_COMPONENT_SWIZZLE_IDENTITY},
	                                      .subresourceRange = {.aspectMask = VulkanImage_FormatAspect(createInfo->Format),
	                                                           .baseMipLevel   = createInfo->BaseMipLevel,
	                                                           .levelCount     = createInfo->MipLevels,
	                                                           .baseArrayLayer = createInfo->BaseArrayLayer,
	                                                           .layerCount     = createInfo->ArrayLayers}};

	VkImageView vkImageView   = VK_NULL_HANDLE;
	const VkResult viewResult = context->vk.CreateImageView(context->Device, &viewCI, &context->Allocator, &vkImageView);

	if (viewResult == VK_SUCCESS) {
		view->Image      = createInfo->Image->Image;
		view->View       = vkImageView;
		view->CreateInfo = viewCI;

		LogD("[Vulkan] ImageView created.");
	}

	return viewResult;
}

void VulkanImageView_Destroy(VulkanContext* context, VulkanImageView* view) {
	if (view->View) { context->vk.DestroyImageView(context->Device, view->View, &context->Allocator); }
	view->Image = NULL;
	view->View  = VK_NULL_HANDLE;
}

VulkanImage_CreateInfo VulkanImageCI_Immutable2D(VkFormat format, VkExtent2D extent) {
	const VulkanImage_CreateInfo ci = {.Type   = VK_IMAGE_TYPE_2D,
	                                   .Format = format,
	                                   .Extent = {.width = extent.width, .height = extent.height, .depth = 1},
	                                   .Usage  = VK_IMAGE_USAGE_SAMPLED_BIT};

	return ci;
}
