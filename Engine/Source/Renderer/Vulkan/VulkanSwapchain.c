#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Platform/Platform.h>
#include <Obsidian/Renderer/Vulkan/VulkanImage.h>
#include <Obsidian/Renderer/Vulkan/VulkanStrings.h>
#include <Obsidian/Renderer/Vulkan/VulkanSwapchain.h>

VkResult VulkanSwapchain_Create(VulkanContext* context) {
	Memory_Zero(&context->Swapchain, sizeof(VulkanSwapchain));

	const VkResult result = VulkanSwapchain_Recreate(context);
	if (result != VK_SUCCESS) {
		LogE("[VulkanSwapchain] Failed to create swapchain! (%s)", VulkanString_VkResult(result));
		VulkanSwapchain_Destroy(context);

		return result;
	}

	LogD("[Vulkan] Swapchain created.");

	return VK_SUCCESS;
}

/** Destroys all of the objects associated with an old swapchain while preserving anything the new swapchain can use. */
void VulkanSwapchain_Clean(VulkanContext* context) {
	VulkanImage_Destroy(context, &context->Swapchain.DepthImage);
	VulkanImageView_Destroy(context, &context->Swapchain.DepthView);
	if (context->Swapchain.Views) {
		const U32 viewCount = DynArray_Size(&context->Swapchain.Views);
		for (U32 i = 0; i < viewCount; ++i) {
			if (context->Swapchain.Views[i].View) {
				context->vk.DestroyImageView(context->Device, context->Swapchain.Views[i].View, &context->Allocator);
			}
		}
		DynArray_Destroy(&context->Swapchain.Views);
		context->Swapchain.Views = NULL;
	}
	if (context->Swapchain.Images) {
		DynArray_Destroy(&context->Swapchain.Images);
		context->Swapchain.Images = NULL;
	}
	if (context->Swapchain.Swapchain) {
		context->vk.DestroySwapchainKHR(context->Device, context->Swapchain.Swapchain, &context->Allocator);
		context->Swapchain.Swapchain = VK_NULL_HANDLE;
	}
}

void VulkanSwapchain_Destroy(VulkanContext* context) {
	VulkanSwapchain_Clean(context);
	if (context->Swapchain.SurfaceFormats) {
		DynArray_Destroy(&context->Swapchain.SurfaceFormats);
		context->Swapchain.SurfaceFormats = NULL;
	}
	if (context->Swapchain.PresentModes) {
		DynArray_Destroy(&context->Swapchain.PresentModes);
		context->Swapchain.PresentModes = NULL;
	}
}

VkResult VulkanSwapchain_Recreate(VulkanContext* context) {
	// First update all of our surface capability information.
	{
		context->vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(
			context->PhysicalDevice, context->Surface, &context->Swapchain.Capabilities);

		U32 formatCount = 0;
		context->vk.GetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice, context->Surface, &formatCount, NULL);
		if (context->Swapchain.SurfaceFormats) {
			DynArray_Resize(&context->Swapchain.SurfaceFormats, formatCount);
		} else {
			context->Swapchain.SurfaceFormats = DynArray_CreateWithSize(VkSurfaceFormatKHR, formatCount);
		}
		context->vk.GetPhysicalDeviceSurfaceFormatsKHR(
			context->PhysicalDevice, context->Surface, &formatCount, context->Swapchain.SurfaceFormats);

		U32 presentModeCount = 0;
		context->vk.GetPhysicalDeviceSurfacePresentModesKHR(
			context->PhysicalDevice, context->Surface, &presentModeCount, NULL);
		if (context->Swapchain.PresentModes) {
			DynArray_Resize(&context->Swapchain.PresentModes, presentModeCount);
		} else {
			context->Swapchain.PresentModes = DynArray_CreateWithSize(VkPresentModeKHR, presentModeCount);
		}
		context->vk.GetPhysicalDeviceSurfacePresentModesKHR(
			context->PhysicalDevice, context->Surface, &presentModeCount, context->Swapchain.PresentModes);
	}

	// Now, determine our swapchain parameters
	U32 swapchainImageCount            = 2;
	VkExtent2D swapchainExtent         = {.width = 0, .height = 0};
	VkSurfaceFormatKHR swapchainFormat = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		// Find the size of our framebuffer
		Platform_GetFramebufferSize(context->Platform, &swapchainExtent.width, &swapchainExtent.height);
		if (swapchainExtent.width > context->Swapchain.Capabilities.maxImageExtent.width) {
			swapchainExtent.width = context->Swapchain.Capabilities.maxImageExtent.width;
		}
		if (swapchainExtent.width < context->Swapchain.Capabilities.minImageExtent.width) {
			swapchainExtent.width = context->Swapchain.Capabilities.minImageExtent.width;
		}
		if (swapchainExtent.height > context->Swapchain.Capabilities.maxImageExtent.height) {
			swapchainExtent.height = context->Swapchain.Capabilities.maxImageExtent.height;
		}
		if (swapchainExtent.height < context->Swapchain.Capabilities.minImageExtent.height) {
			swapchainExtent.height = context->Swapchain.Capabilities.minImageExtent.height;
		}

		// Given the list of available formats, try to pick a format that we prefer
		const U32 formatCount = DynArray_Size(&context->Swapchain.SurfaceFormats);
		for (U32 i = 0; i < formatCount; ++i) {
			if (context->Swapchain.SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			    context->Swapchain.SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				swapchainFormat = context->Swapchain.SurfaceFormats[i];
				break;
			}
		}
		// But if we can't find a preferred format, just take the first one
		if (swapchainFormat.format == VK_FORMAT_UNDEFINED) { swapchainFormat = context->Swapchain.SurfaceFormats[0]; }
	}

	const VkSwapchainCreateInfoKHR swapchainCI = {.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	                                              .pNext                 = NULL,
	                                              .flags                 = 0,
	                                              .surface               = context->Surface,
	                                              .minImageCount         = swapchainImageCount,
	                                              .imageFormat           = swapchainFormat.format,
	                                              .imageColorSpace       = swapchainFormat.colorSpace,
	                                              .imageExtent           = swapchainExtent,
	                                              .imageArrayLayers      = 1,
	                                              .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	                                              .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
	                                              .queueFamilyIndexCount = 0,
	                                              .pQueueFamilyIndices   = NULL,
	                                              .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	                                              .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	                                              .presentMode           = swapchainPresentMode,
	                                              .clipped               = VK_FALSE,
	                                              .oldSwapchain          = context->Swapchain.Swapchain};

	// Dump debug info
	{
		LogT("[VulkanSwapchain] Swapchain parameters:");
		LogT("[VulkanSwapchain] - Minimum images: %u", swapchainCI.minImageCount);
		LogT("[VulkanSwapchain] - Format: %s", VulkanString_VkFormat(swapchainCI.imageFormat));
		LogT("[VulkanSwapchain] - Color Space: %s", VulkanString_VkColorSpaceKHR(swapchainCI.imageColorSpace));
		LogT("[VulkanSwapchain] - Extent: %u x %u", swapchainCI.imageExtent.width, swapchainCI.imageExtent.height);
	}

	// Create our swapchain
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	const VkResult swapchainResult =
		context->vk.CreateSwapchainKHR(context->Device, &swapchainCI, &context->Allocator, &swapchain);
	if (swapchainResult != VK_SUCCESS) {
		LogE("[VulkanSwapchain] Failed to create swapchain! (%s)", VulkanString_VkResult(swapchainResult));
		VulkanSwapchain_Destroy(context);

		return swapchainResult;
	}
	// If we had an old swapchain, clean it up
	if (context->Swapchain.Swapchain) { VulkanSwapchain_Clean(context); }
	context->Swapchain.Swapchain = swapchain;

	// Set up the fake create info for our swapchain images
	const VkImageCreateInfo imageCI = {.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	                                   .pNext       = NULL,
	                                   .flags       = 0,
	                                   .imageType   = VK_IMAGE_TYPE_2D,
	                                   .format      = swapchainCI.imageFormat,
	                                   .extent      = {swapchainCI.imageExtent.width, swapchainCI.imageExtent.height, 1},
	                                   .mipLevels   = 1,
	                                   .arrayLayers = swapchainCI.imageArrayLayers,
	                                   .samples     = VK_SAMPLE_COUNT_1_BIT,
	                                   .tiling      = VK_IMAGE_TILING_OPTIMAL,
	                                   .usage       = swapchainCI.imageUsage,
	                                   .sharingMode = swapchainCI.imageSharingMode,
	                                   .queueFamilyIndexCount = swapchainCI.queueFamilyIndexCount,
	                                   .pQueueFamilyIndices   = swapchainCI.pQueueFamilyIndices,
	                                   .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

	// Set up the create info for our swapchain image views
	VulkanImageView_CreateInfo viewCI = {.Image          = NULL,  // To be filled in for loop
	                                     .Type           = VK_IMAGE_VIEW_TYPE_2D,
	                                     .Format         = swapchainCI.imageFormat,
	                                     .BaseMipLevel   = 0,
	                                     .MipLevels      = 1,
	                                     .BaseArrayLayer = 0,
	                                     .ArrayLayers    = swapchainCI.imageArrayLayers};

	// Fetch all of our swapchain images
	U32 imageCount = 0;
	context->vk.GetSwapchainImagesKHR(context->Device, context->Swapchain.Swapchain, &imageCount, NULL);
	VkImage* images           = DynArray_CreateWithSize(VkImage, imageCount);
	context->Swapchain.Images = DynArray_CreateWithSize(VulkanImage, imageCount);
	context->Swapchain.Views  = DynArray_CreateWithSize(VulkanImageView, imageCount);
	context->vk.GetSwapchainImagesKHR(context->Device, context->Swapchain.Swapchain, &imageCount, images);

	// Assign them to our DynArray with the fake create info
	for (U32 i = 0; i < imageCount; ++i) {
		context->Swapchain.Images[i].Image      = images[i];
		context->Swapchain.Images[i].Memory     = VK_NULL_HANDLE;
		context->Swapchain.Images[i].CreateInfo = imageCI;
	}
	DynArray_Destroy(&images);

	// Create all of our swapchain image views
	for (U32 i = 0; i < imageCount; ++i) {
		viewCI.Image              = &context->Swapchain.Images[i];
		const VkResult viewResult = VulkanImageView_Create(context, &viewCI, &context->Swapchain.Views[i]);
		if (viewResult != VK_SUCCESS) {
			LogE("[VulkanSwapchain] Failed to create image view for swapchain image!");
			VulkanSwapchain_Destroy(context);

			return viewResult;
		}
	}

	// Determine our favorite depth format
	VkFormat depthFormat          = VK_FORMAT_UNDEFINED;
	const VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT,
	                                 VK_FORMAT_D32_SFLOAT_S8_UINT,
	                                 VK_FORMAT_D24_UNORM_S8_UINT,
	                                 VK_FORMAT_D16_UNORM,
	                                 VK_FORMAT_D16_UNORM_S8_UINT};
	const U32 depthFormatCount    = sizeof(depthFormats) / sizeof(*depthFormats);
	for (U32 i = 0; i < depthFormatCount; ++i) {
		VkFormatProperties props;
		context->vk.GetPhysicalDeviceFormatProperties(context->PhysicalDevice, depthFormats[i], &props);
		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			depthFormat = depthFormats[i];
			break;
		}
	}
	// Per Vulkan spec, at least one of the above formats MUST be supported, so this should never happen
	Assert(depthFormat != VK_FORMAT_UNDEFINED);

	// Create our depth image
	const VulkanImage_CreateInfo depthCI = {.Type   = VK_IMAGE_TYPE_2D,
	                                        .Format = depthFormat,
	                                        .Extent = imageCI.extent,
	                                        .Usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
	const VkResult imageResult           = VulkanImage_Create(context, &depthCI, &context->Swapchain.DepthImage);
	if (imageResult != VK_SUCCESS) {
		LogE("[VulkanSwapchain] Failed to create depth image for swapchain! (%s)", VulkanString_VkResult(imageResult));
		VulkanSwapchain_Destroy(context);

		return imageResult;
	}
	const VulkanImageView_CreateInfo depthViewCI = {.Image          = &context->Swapchain.DepthImage,
	                                                .Type           = VK_IMAGE_VIEW_TYPE_2D,
	                                                .Format         = depthCI.Format,
	                                                .BaseMipLevel   = 0,
	                                                .MipLevels      = 1,
	                                                .BaseArrayLayer = 0,
	                                                .ArrayLayers    = 1};
	const VkResult viewResult = VulkanImageView_Create(context, &depthViewCI, &context->Swapchain.DepthView);
	if (viewResult != VK_SUCCESS) {
		LogE("[VulkanSwapchain] Failed to create depth image view for swapchain! (%s)", VulkanString_VkResult(viewResult));
		VulkanSwapchain_Destroy(context);

		return viewResult;
	}

	return VK_SUCCESS;
}
