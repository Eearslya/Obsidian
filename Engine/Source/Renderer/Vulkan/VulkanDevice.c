#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/Vulkan/VulkanDevice.h>
#include <Obsidian/Renderer/Vulkan/VulkanStrings.h>

static void VulkanDevice_EnumerateGPU(VulkanContext* context, VkPhysicalDevice gpu, PhysicalDeviceInfo* info) {
	Memory_Zero(info, sizeof(PhysicalDeviceInfo));

	info->GPU = gpu;
	context->vk.GetPhysicalDeviceFeatures(gpu, &info->Features);
	context->vk.GetPhysicalDeviceMemoryProperties(gpu, &info->Memory);
	context->vk.GetPhysicalDeviceProperties(gpu, &info->Properties);

	U32 extensionCount = 0;
	context->vk.EnumerateDeviceExtensionProperties(gpu, NULL, &extensionCount, NULL);
	info->Extensions = DynArray_CreateWithSize(VkExtensionProperties, extensionCount);
	context->vk.EnumerateDeviceExtensionProperties(gpu, NULL, &extensionCount, info->Extensions);

	U32 familyCount = 0;
	context->vk.GetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, NULL);
	info->QueueFamilies = DynArray_CreateWithSize(VkQueueFamilyProperties, familyCount);
	context->vk.GetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, info->QueueFamilies);
}

static void VulkanDevice_DumpGPUInfo(const PhysicalDeviceInfo* info) {
	LogT("[VulkanDevice] - Name: %s", info->Properties.deviceName);
	LogT("[VulkanDevice] - Type: %s", VulkanString_VkPhysicalDeviceType(info->Properties.deviceType));

	const U64 extensionCount = DynArray_Size(&info->Extensions);
	LogT("[VulkanDevice] - Extensions (%d):", extensionCount);
	for (U64 i = 0; i < extensionCount; ++i) {
		LogT("[VulkanDevice]   - %s v%d", info->Extensions[i].extensionName, info->Extensions[i].specVersion);
	}

	LogT("[VulkanDevice] - Memory:");
	LogT("[VulkanDevice]   - Heaps (%d):", info->Memory.memoryHeapCount);
	for (U32 i = 0; i < info->Memory.memoryHeapCount; ++i) {
		const B8 dl      = (info->Memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) > 0;
		const char* unit = "B";
		F32 amount       = info->Memory.memoryHeaps[i].size;

		if (amount > 1024.0f) {
			unit = "KiB";
			amount /= 1024.0f;
		}
		if (amount > 1024.0f) {
			unit = "MiB";
			amount /= 1024.0f;
		}
		if (amount > 1024.0f) {
			unit = "GiB";
			amount /= 1024.0f;
		}
		LogT("[VulkanDevice]     %2u: %.2f %s %s", i, amount, unit, dl ? "(Device Local)" : "");
	}
	LogT("[VulkanDevice]   - Types (%d):", info->Memory.memoryTypeCount);
	LogT("[VulkanDevice]         / DL | HV | HC | HH | LA \\");
	for (U32 i = 0; i < info->Memory.memoryTypeCount; ++i) {
		const B8 dl = (info->Memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) > 0;
		const B8 hv = (info->Memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0;
		const B8 hc = (info->Memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) > 0;
		const B8 hh = (info->Memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) > 0;
		const B8 la = (info->Memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) > 0;
		LogT("[VulkanDevice]     %2u: | %s | %s | %s | %s | %s | Heap %u",
		     i,
		     dl ? "DL" : "  ",
		     hv ? "HV" : "  ",
		     hc ? "HC" : "  ",
		     hh ? "HH" : "  ",
		     la ? "LA" : "  ",
		     info->Memory.memoryTypes[i].heapIndex);
	}

	const U32 familyCount = DynArray_Size(&info->QueueFamilies);
	LogT("[VulkanDevice] - Queue Families (%u):", familyCount);
	LogT("[VulkanDevice]       / GFX | CMP | XFR \\");
	for (U32 i = 0; i < familyCount; ++i) {
		const B8 gfx = (info->QueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0;
		const B8 cmp = (info->QueueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) > 0;
		const B8 xfr = (info->QueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) > 0;
		LogT("[VulkanDevice]   %2u: | %s | %s | %s | (%u queues)",
		     i,
		     gfx ? "GFX" : "   ",
		     cmp ? "CMP" : "   ",
		     xfr ? "XFR" : "   ",
		     info->QueueFamilies[i].queueCount);
	}
}

static B8 VulkanDevice_CheckCompatibility(VulkanContext* context, const PhysicalDeviceInfo* info) {
	B8 graphicsQueue = FALSE;
	B8 transferQueue = FALSE;
	B8 computeQueue  = FALSE;
	B8 presentQueue  = FALSE;

	const U32 familyCount = DynArray_Size(&info->QueueFamilies);
	for (U32 i = 0; i < familyCount; ++i) {
		if (info->QueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueue    = TRUE;
			VkBool32 present = VK_FALSE;
			context->vk.GetPhysicalDeviceSurfaceSupportKHR(info->GPU, i, context->Surface, &present);
			if (present == VK_TRUE) { presentQueue = TRUE; }
		}
		if (info->QueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) { transferQueue = TRUE; }
		if (info->QueueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { computeQueue = TRUE; }
	}
	if (!graphicsQueue || !transferQueue || !computeQueue || !presentQueue) { return FALSE; }

	return TRUE;
}

static B8 VulkanDevice_SelectGPU(VulkanContext* context) {
	// Enumerate the available physical devices
	U32 gpuCount = 0;
	context->vk.EnumeratePhysicalDevices(context->Instance, &gpuCount, NULL);
	if (gpuCount == 0) {
		LogE("[VulkanDevice] No Vulkan-compatible GPUs were found.");

		return FALSE;
	}
	VkPhysicalDevice* gpus = DynArray_CreateWithSize(VkPhysicalDevice, gpuCount);
	context->vk.EnumeratePhysicalDevices(context->Instance, &gpuCount, gpus);
	LogD("[VulkanDevice] Found %d GPUs.", gpuCount);

	PhysicalDeviceInfo* gpuInfos = DynArray_CreateWithSize(PhysicalDeviceInfo, gpuCount);
	for (U32 i = 0; i < gpuCount; ++i) { VulkanDevice_EnumerateGPU(context, gpus[i], &gpuInfos[i]); }

#if OBSIDIAN_DEBUG == 1
	for (U32 i = 0; i < gpuCount; ++i) {
		LogT("[VulkanDevice] GPU %d:", i);
		VulkanDevice_DumpGPUInfo(&gpuInfos[i]);
	}
#endif

	for (U32 i = 0; i < gpuCount; ++i) {
		if (VulkanDevice_CheckCompatibility(context, &gpuInfos[i])) {
			LogI("[VulkanDevice] Selected GPU: %s", gpuInfos[i].Properties.deviceName);
			context->PhysicalDevice = gpuInfos[i].GPU;
			Memory_Copy(&context->DeviceInfo, &gpuInfos[i], sizeof(PhysicalDeviceInfo));
			// Nullify our DynArrays so they aren't destroyed during cleanup
			gpuInfos[i].Extensions    = NULL;
			gpuInfos[i].QueueFamilies = NULL;
			break;
		}
	}

	for (U32 i = 0; i < gpuCount; ++i) {
		if (gpuInfos[i].Extensions) { DynArray_Destroy(&gpuInfos[i].Extensions); }
		if (gpuInfos[i].QueueFamilies) { DynArray_Destroy(&gpuInfos[i].QueueFamilies); }
	}
	DynArray_Destroy(&gpuInfos);
	DynArray_Destroy(&gpus);

	return context->PhysicalDevice != VK_NULL_HANDLE;
}

static B8 VulkanDevice_FindQueue(ConstDynArrayT queueFamiliesArray,
                                 VkQueueFlagBits required,
                                 VkQueueFlags reject,
                                 DynArrayT queueCountsArray,
                                 U32* selectedFamily,
                                 U32* selectedIndex) {
	const U32 familyCount                              = DynArray_Size(queueFamiliesArray);
	const VkQueueFamilyProperties* const queueFamilies = *queueFamiliesArray;
	U32* queueCounts                                   = *queueCountsArray;
	for (U32 i = 0; i < familyCount; ++i) {
		if ((queueFamilies[i].queueFlags & required) > 0 && (queueFamilies[i].queueFlags & reject) == 0 &&
		    queueCounts[i] < queueFamilies[i].queueCount) {
			*selectedFamily = i;
			*selectedIndex  = queueCounts[i];
			++queueCounts[i];

			return TRUE;
		}
	}

	return FALSE;
}

VkResult VulkanDevice_Create(VulkanContext* context) {
	// Select first compatible GPU
	if (!VulkanDevice_SelectGPU(context)) {
		LogE("[VulkanDevice] Failed to find a compatible Vulkan GPU!");
		VulkanDevice_Destroy(context);

		return VK_ERROR_INCOMPATIBLE_DRIVER;
	}

	// Determine our queue assignments
	const U32 familyCount = DynArray_Size(&context->DeviceInfo.QueueFamilies);
	// Keep track of how many queues we have in each family, so we don't double-assign any.
	U32* queueCounts = DynArray_CreateWithSize(U32, familyCount);

	// Main graphics queue
	{
		// First find our graphics queue. Here we take whatever queue comes first.
		if (!VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            0,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.GraphicsFamily,
		                            &context->DeviceInfo.GraphicsIndex)) {
			AssertMsg(FALSE, "Unable to find graphics queue. This shouldn't be possible!");
		}
	}

	// Main compute queue
	{
		// Now we look for a compute queue. We first attempt to find a dedicated compute queue (i.e. a queue that only has
		// Compute), then we attempt to find a queue separate from graphics, and finally we take whatever we can get.
		if (!VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_COMPUTE_BIT,
		                            VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.ComputeFamily,
		                            &context->DeviceInfo.ComputeIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_COMPUTE_BIT,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.ComputeFamily,
		                            &context->DeviceInfo.ComputeIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_COMPUTE_BIT,
		                            0,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.ComputeFamily,
		                            &context->DeviceInfo.ComputeIndex)) {
			context->DeviceInfo.ComputeFamily = context->DeviceInfo.GraphicsFamily;
			context->DeviceInfo.ComputeIndex  = context->DeviceInfo.GraphicsIndex;
		}
	}

	// Main transfer queue
	{
		// Now we look for a transfer queue. We first attempt to find a dedicated transfer queue (i.e. a queue that only
		// has Transfer), then we attempt to find a queue separate from graphics, and finally we take the compute queue.
		if (!VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_TRANSFER_BIT,
		                            VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.TransferFamily,
		                            &context->DeviceInfo.TransferIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_TRANSFER_BIT,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.TransferFamily,
		                            &context->DeviceInfo.TransferIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_TRANSFER_BIT,
		                            0,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.TransferFamily,
		                            &context->DeviceInfo.TransferIndex)) {
			context->DeviceInfo.TransferFamily = context->DeviceInfo.ComputeFamily;
			context->DeviceInfo.TransferIndex  = context->DeviceInfo.ComputeIndex;
		}
	}

	// Secondary graphics queue
	{
		// Finally, we attempt to find one more graphics queue that we can use for asynchronous graphics processing (such as
		// mipmap generation). If we can't find one, we just use the main graphics queue.
		if (!VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.AsyncGraphicsFamily,
		                            &context->DeviceInfo.AsyncGraphicsIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            VK_QUEUE_COMPUTE_BIT,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.AsyncGraphicsFamily,
		                            &context->DeviceInfo.AsyncGraphicsIndex) &&
		    !VulkanDevice_FindQueue((ConstDynArrayT) &context->DeviceInfo.QueueFamilies,
		                            VK_QUEUE_GRAPHICS_BIT,
		                            0,
		                            (DynArrayT) &queueCounts,
		                            &context->DeviceInfo.AsyncGraphicsFamily,
		                            &context->DeviceInfo.AsyncGraphicsIndex)) {
			context->DeviceInfo.AsyncGraphicsFamily = context->DeviceInfo.AsyncGraphicsFamily;
			context->DeviceInfo.AsyncGraphicsIndex  = context->DeviceInfo.AsyncGraphicsIndex;
		}
	}

	LogT("[VulkanDevice] Using queue %u.%u for graphics.",
	     context->DeviceInfo.GraphicsFamily,
	     context->DeviceInfo.GraphicsIndex);
	LogT("[VulkanDevice] Using queue %u.%u for compute.",
	     context->DeviceInfo.ComputeFamily,
	     context->DeviceInfo.ComputeIndex);
	LogT("[VulkanDevice] Using queue %u.%u for transfer.",
	     context->DeviceInfo.TransferFamily,
	     context->DeviceInfo.TransferIndex);
	LogT("[VulkanDevice] Using queue %u.%u for async graphics.",
	     context->DeviceInfo.AsyncGraphicsFamily,
	     context->DeviceInfo.AsyncGraphicsIndex);

	U32 uniqueFamilyCount = 0;
	U32* uniqueFamilies   = DynArray_CreateWithCapacity(U32, familyCount);
	U32 maxCount          = 0;
	for (U32 i = 0; i < familyCount; ++i) {
		if (queueCounts[i] > 0) {
			DynArray_PushValue(&uniqueFamilies, i);
			++uniqueFamilyCount;
		}
		if (queueCounts[i] > maxCount) { maxCount = queueCounts[i]; }
	}
	F32* queuePriorities = DynArray_CreateWithSize(F32, maxCount);
	for (U32 i = 0; i < maxCount; ++i) { queuePriorities[i] = 1.0f; }
	VkDeviceQueueCreateInfo* queueCIs = DynArray_CreateWithSize(VkDeviceQueueCreateInfo, uniqueFamilyCount);
	for (U32 i = 0; i < uniqueFamilyCount; ++i) {
		queueCIs[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCIs[i].pNext            = NULL;
		queueCIs[i].flags            = 0;
		queueCIs[i].queueFamilyIndex = uniqueFamilies[i];
		queueCIs[i].queueCount       = queueCounts[uniqueFamilies[i]];
		queueCIs[i].pQueuePriorities = queuePriorities;
	}

	const VkDeviceCreateInfo deviceCI = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	                                     .pNext                   = NULL,
	                                     .flags                   = 0,
	                                     .queueCreateInfoCount    = uniqueFamilyCount,
	                                     .pQueueCreateInfos       = queueCIs,
	                                     .enabledLayerCount       = 0,
	                                     .ppEnabledLayerNames     = NULL,
	                                     .enabledExtensionCount   = 0,
	                                     .ppEnabledExtensionNames = NULL,
	                                     .pEnabledFeatures        = NULL};
	VkDevice device                   = VK_NULL_HANDLE;
	const VkResult deviceResult =
		context->vk.CreateDevice(context->PhysicalDevice, &deviceCI, &context->Allocator, &device);

	DynArray_Destroy(&uniqueFamilies);
	DynArray_Destroy(&queuePriorities);
	DynArray_Destroy(&queueCIs);
	DynArray_Destroy(&queueCounts);

	if (deviceResult == VK_SUCCESS) {
		context->Device = device;
		LogD("[Vulkan] Device created.");

#define LoadDeviceFn(fn, ext)                                                                                       \
	do {                                                                                                              \
		if ((context->vk.fn = (PFN_vk##fn) context->vk.GetDeviceProcAddr(context->Device, "vk" #fn)) == NULL && !ext) { \
			LogE("[VulkanEngine] Failed to load device function '%s'!", "vk" #fn);                                        \
			return FALSE;                                                                                                 \
		}                                                                                                               \
	} while (0)
		// Core 1.0
		LoadDeviceFn(AllocateMemory, FALSE);
		LoadDeviceFn(BindImageMemory, FALSE);
		LoadDeviceFn(CreateImage, FALSE);
		LoadDeviceFn(DestroyImage, FALSE);
		LoadDeviceFn(FreeMemory, FALSE);
		LoadDeviceFn(GetDeviceQueue, FALSE);
		LoadDeviceFn(GetImageMemoryRequirements, FALSE);
#undef LoadDeviceFn

		context->vk.GetDeviceQueue(
			context->Device, context->DeviceInfo.GraphicsFamily, context->DeviceInfo.GraphicsIndex, &context->Graphics);
		context->vk.GetDeviceQueue(
			context->Device, context->DeviceInfo.ComputeFamily, context->DeviceInfo.ComputeIndex, &context->Compute);
		context->vk.GetDeviceQueue(
			context->Device, context->DeviceInfo.TransferFamily, context->DeviceInfo.TransferIndex, &context->Transfer);
		context->vk.GetDeviceQueue(context->Device,
		                           context->DeviceInfo.AsyncGraphicsFamily,
		                           context->DeviceInfo.AsyncGraphicsIndex,
		                           &context->AsyncGraphics);
	}

	return deviceResult;
}

void VulkanDevice_Destroy(VulkanContext* context) {
	if (context->Device) {
		context->vk.DestroyDevice(context->Device, &context->Allocator);
		context->PhysicalDevice = VK_NULL_HANDLE;
		context->Device         = VK_NULL_HANDLE;
		context->Graphics       = VK_NULL_HANDLE;
		context->Compute        = VK_NULL_HANDLE;
		context->Transfer       = VK_NULL_HANDLE;
		context->AsyncGraphics  = VK_NULL_HANDLE;
	}
	if (context->DeviceInfo.Extensions) {
		DynArray_Destroy(&context->DeviceInfo.Extensions);
		context->DeviceInfo.Extensions = NULL;
	}
	if (context->DeviceInfo.QueueFamilies) {
		DynArray_Destroy(&context->DeviceInfo.QueueFamilies);
		context->DeviceInfo.QueueFamilies = NULL;
	}
}

I32 VulkanDevice_FindMemoryType(VulkanContext* context, U32 typeFilter, VkMemoryPropertyFlags properties) {
	for (U32 i = 0; i < context->DeviceInfo.Memory.memoryTypeCount; ++i) {
		if (typeFilter & (1 << i) && (context->DeviceInfo.Memory.memoryTypes->propertyFlags & properties) == properties) {
			return i;
		}
	}

	return -1;
}