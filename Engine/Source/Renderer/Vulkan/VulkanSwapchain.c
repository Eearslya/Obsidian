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

void VulkanSwapchain_Destroy(VulkanContext* context) {
	if (context->Swapchain.Images) {
		DynArray_Destroy(&context->Swapchain.Images);
		context->Swapchain.Images = NULL;
	}
	if (context->Swapchain.Swapchain) {
		context->vk.DestroySwapchainKHR(context->Device, context->Swapchain.Swapchain, &context->Allocator);
		context->Swapchain.Swapchain = VK_NULL_HANDLE;
	}
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

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	const VkResult swapchainResult =
		context->vk.CreateSwapchainKHR(context->Device, &swapchainCI, &context->Allocator, &swapchain);
	if (swapchainResult != VK_SUCCESS) { return swapchainResult; }
	if (context->Swapchain.Swapchain) {
		DynArray_Destroy(&context->Swapchain.Images);
		context->Swapchain.Images = NULL;
		context->vk.DestroySwapchainKHR(context->Device, context->Swapchain.Swapchain, &context->Allocator);
		context->Swapchain.Swapchain = VK_NULL_HANDLE;
	}
	context->Swapchain.Swapchain = swapchain;

	U32 imageCount = 0;
	context->vk.GetSwapchainImagesKHR(context->Device, context->Swapchain.Swapchain, &imageCount, NULL);
	context->Swapchain.Images = DynArray_CreateWithSize(VkImage, imageCount);
	context->vk.GetSwapchainImagesKHR(
		context->Device, context->Swapchain.Swapchain, &imageCount, context->Swapchain.Images);

	return swapchainResult;
}
