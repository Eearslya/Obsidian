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

	VkExtent2D swapchainExtent = {};
	Platform_GetFramebufferSize(context->Platform, &swapchainExtent.width, &swapchainExtent.height);

	const VkSwapchainCreateInfoKHR swapchainCI = {.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	                                              .pNext                 = NULL,
	                                              .flags                 = 0,
	                                              .surface               = context->Surface,
	                                              .minImageCount         = 0,
	                                              .imageFormat           = 0,
	                                              .imageColorSpace       = 0,
	                                              .imageExtent           = swapchainExtent,
	                                              .imageArrayLayers      = 1,
	                                              .imageUsage            = 0,
	                                              .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
	                                              .queueFamilyIndexCount = 0,
	                                              .pQueueFamilyIndices   = NULL,
	                                              .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	                                              .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	                                              .presentMode           = VK_PRESENT_MODE_FIFO_KHR,
	                                              .clipped               = VK_FALSE,
	                                              .oldSwapchain          = context->Swapchain.Swapchain};

	return VK_SUCCESS;
}