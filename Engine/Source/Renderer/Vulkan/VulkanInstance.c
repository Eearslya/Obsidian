#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Core/String.h>
#include <Obsidian/Renderer/Vulkan/VulkanDebug.h>
#include <Obsidian/Renderer/Vulkan/VulkanInstance.h>

typedef struct VulkanInstanceExtension {
	VkExtensionProperties Extension;
	VkLayerProperties* Layer;
} VulkanInstanceExtension;

/**
 * Determine if the specified string exists in the DynArray.
 * @param listArray A const char* DynArray of strings to search.
 * @param entry The string to search for.
 * @return TRUE if the entry is present, FALSE otherwise.
 */
static B8 ListContains(ConstDynArrayT listArray, const char* entry) {
	const char* const* list = *listArray;
	const U64 count         = DynArray_Size(listArray);
	for (U64 i = 0; i < count; ++i) {
		if (String_Equal(list[i], entry)) { return TRUE; }
	}

	return FALSE;
}

/**
 * Determine of the specified layer is within the given list.
 * @param listArray A VkLayerProperties DynArray of available layers.
 * @param layer The layer to search for.
 * @return TRUE if the layer was found, FALSE otherwise.
 */
static B8 FindLayer(ConstDynArrayT listArray, const char* layer) {
	const VkLayerProperties* const list = *listArray;
	const U64 layerCount                = DynArray_Size(listArray);
	for (U64 i = 0; i < layerCount; ++i) {
		if (String_Equal(list[i].layerName, layer)) { return TRUE; }
	}

	return FALSE;
}

/**
 * Determine if an extension is available and what layer is required to enable it.
 * @param listArray A VulkanInstanceExtension DynArray of Vulkan extensions and their source layers.
 * @param extension The extension name to find.
 * @param[out] requiredLayer A pointer to layer properties that will represent what layer is needed to enable the
 * extension.
 * @return TRUE is the extension is found and available, FALSE otherwise.
 */
static B8 FindExtension(ConstDynArrayT listArray, const char* extension, VkLayerProperties** requiredLayer) {
	const VulkanInstanceExtension* const list = *listArray;
	const U64 extensionCount                  = DynArray_Size(listArray);
	for (U64 i = 0; i < extensionCount; ++i) {
		if (String_Equal(list[i].Extension.extensionName, extension)) {
			if (requiredLayer) { *requiredLayer = list[i].Layer; }
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Enable the specified extension.
 * @param extension The extension to enable.
 * @param availableExtension A VulkanInstanceExtension DynArray containing all available extensions and their required
 * layers.
 * @param enabledExtensions A const char* DynArray to append the extension to.
 * @param enabledLayers A const char* DynArray to append any required layers to.
 */
static void EnableExtension(const char* extension,
                            ConstDynArrayT availableExtensionsArray,
                            DynArrayT enabledExtensionsArray,
                            DynArrayT enabledLayersArray) {
	VkLayerProperties* layer = NULL;
	if (FindExtension(availableExtensionsArray, extension, &layer)) {
		// We found the extension, enable it
		DynArray_Push(enabledExtensionsArray, extension);
		// If the extension has a required layer, enable it too
		if (layer) {
			// ...unless the layer is already enabled
			if (!ListContains((ConstDynArrayT) enabledLayersArray, layer->layerName)) {
				DynArray_Push(enabledLayersArray, layer->layerName);
			}
		}
	}
}

VkResult VulkanInstance_Create(VulkanContext* context, ConstDynArrayT instanceExtensionsArray) {
	// Enumerate instance layers
	U32 availableLayerCount            = 0;
	VkLayerProperties* availableLayers = NULL;
	{
		context->vk.EnumerateInstanceLayerProperties(&availableLayerCount, NULL);
		availableLayers = DynArray_CreateWithSize(VkLayerProperties, availableLayerCount);
		context->vk.EnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

		// Log layers to console
		LogD("[VulkanInstance] Found %d instance layers.", availableLayerCount);
		for (U32 i = 0; i < availableLayerCount; ++i) {
			const VkLayerProperties* layer = &availableLayers[i];
			LogT("[VulkanInstance] - %s v%d - %s (Vulkan %d.%d.%d)",
			     layer->layerName,
			     layer->specVersion,
			     layer->description,
			     VK_API_VERSION_MAJOR(layer->specVersion),
			     VK_API_VERSION_MINOR(layer->specVersion),
			     VK_API_VERSION_PATCH(layer->specVersion));
		}
	}

	// Enumerate instance extensions
	U32 availableExtensionCount                  = 0;
	VulkanInstanceExtension* availableExtensions = NULL;
	{
		// First enumerate all core extensions
		U32 coreExtensionCount = 0;
		context->vk.EnumerateInstanceExtensionProperties(NULL, &coreExtensionCount, NULL);
		VkExtensionProperties* extensions = DynArray_CreateWithSize(VkExtensionProperties, coreExtensionCount);
		context->vk.EnumerateInstanceExtensionProperties(NULL, &coreExtensionCount, extensions);
		// Copy core extensions into our array
		availableExtensions = DynArray_CreateWithCapacity(VulkanInstanceExtension, coreExtensionCount);
		for (U32 i = 0; i < coreExtensionCount; ++i) {
			VulkanInstanceExtension ext = {.Extension = extensions[i], .Layer = NULL};
			DynArray_Push(&availableExtensions, ext);
		}
		availableExtensionCount = coreExtensionCount;

		// Next, enumerate the extensions from all of our layers
		for (U32 layerIndex = 0; layerIndex < availableLayerCount; ++layerIndex) {
			const VkLayerProperties* layer = &availableLayers[layerIndex];
			U32 layerExtensionCount        = 0;
			context->vk.EnumerateInstanceExtensionProperties(layer->layerName, &layerExtensionCount, NULL);
			DynArray_Resize(&extensions, layerExtensionCount);
			context->vk.EnumerateInstanceExtensionProperties(layer->layerName, &layerExtensionCount, extensions);

			// Copy layer extensions into our array, if they don't already exist.
			for (U32 i = 0; i < layerExtensionCount; ++i) {
				if (!FindExtension((ConstDynArrayT) &availableExtensions, extensions[i].extensionName, NULL)) {
					availableExtensionCount++;
					VulkanInstanceExtension ext = {.Extension = extensions[i], .Layer = &availableLayers[layerIndex]};
					DynArray_Push(&availableExtensions, ext);
				}
			}
		}

		DynArray_Destroy(&extensions);

		// Log extensions to console
		LogT("[VulkanInstance] Found %d instance extensions.", availableExtensionCount);
		for (U32 i = 0; i < availableExtensionCount; ++i) {
			const VulkanInstanceExtension* ext = &availableExtensions[i];
			if (ext->Layer == NULL) {
				LogT("[VulkanInstance] - %s v%d", ext->Extension.extensionName, ext->Extension.specVersion);
			} else {
				LogT("[VulkanInstance] - %s v%d (From %s)",
				     ext->Extension.extensionName,
				     ext->Extension.specVersion,
				     ext->Layer->layerName);
			}
		}
	}

	// By default, we don't enable any layers
	const char** enabledLayers = DynArray_Create(const char*);

	// Gather a list of all enabled extensions
	// Starting with the required extensions from the arguments
	U64 requiredExtensionCount = 0;
	if (instanceExtensionsArray) { requiredExtensionCount = DynArray_Size(instanceExtensionsArray); }
	const char* const* instanceExtensions = *instanceExtensionsArray;
	const char** enabledExtensions        = DynArray_CreateWithCapacity(const char*, requiredExtensionCount);
	for (U32 i = 0; i < requiredExtensionCount; ++i) {
		EnableExtension(instanceExtensions[i],
		                (ConstDynArrayT) &availableExtensions,
		                (DynArrayT) &enabledExtensions,
		                (DynArrayT) &enabledLayers);
	}

	// Enable Vulkan validation if applicable
#if OBSIDIAN_DEBUG == 1
	B8 enableValidation = TRUE;
	// First ensure that the required layers and extensions are available
	if (!FindLayer((ConstDynArrayT) &availableLayers, "VK_LAYER_KHRONOS_validation")) { enableValidation = FALSE; }
	if (!FindExtension((ConstDynArrayT) &availableExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, NULL)) {
		enableValidation = FALSE;
	}
	// If everything is in order, add the required extensions and layers to enabled
	if (enableValidation) {
		DynArray_PushValue(&enabledLayers, &"VK_LAYER_KHRONOS_validation");
		EnableExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		                (ConstDynArrayT) &availableExtensions,
		                (DynArrayT) &enabledExtensions,
		                (DynArrayT) &enabledLayers);
	} else {
		LogD("[VulkanInstance] Validation layer and/or debug utils extension not found. Validation will be disabled.");
	}
#else
	B8 enableValidation = FALSE;
#endif

	// Dump debug info
	const U32 enabledLayerCount     = DynArray_Size(&enabledLayers);
	const U32 enabledExtensionCount = DynArray_Size(&enabledExtensions);
	{
		if (enabledLayerCount > 0) {
			LogT("[VulkanInstance] Enabled layers (%d):", enabledLayerCount);
			for (U32 i = 0; i < enabledLayerCount; ++i) { LogT("[VulkanInstance] - %s", enabledLayers[i]); }
		}
		if (enabledExtensionCount > 0) {
			LogT("[VulkanInstance] Enabled extensions (%d):", enabledExtensionCount);
			for (U32 i = 0; i < enabledExtensionCount; ++i) { LogT("[VulkanInstance] - %s", enabledExtensions[i]); }
		}
	}

	VkResult createResult = VK_ERROR_UNKNOWN;

	// Validate requested extensions
	B8 extensionsPresent   = TRUE;
	const U64 enabledCount = DynArray_Size(&enabledExtensions);
	for (U32 i = 0; i < enabledCount; ++i) {
		if (!FindExtension((ConstDynArrayT) &availableExtensions, enabledExtensions[i], NULL)) {
			LogE("[VulkanInstance] Missing required instance extension '%s'!", instanceExtensions[i]);
			extensionsPresent = FALSE;
			createResult      = VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	// Finally, create our instance.
	VkInstance vkInstance = VK_NULL_HANDLE;
	if (extensionsPresent) {
		const VkApplicationInfo appInfo = {.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		                                   .pNext              = NULL,
		                                   .pApplicationName   = "ObsidianGame",
		                                   .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		                                   .pEngineName        = "Obsidian",
		                                   .engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0),
		                                   .apiVersion         = VK_API_VERSION_1_0};

		VkInstanceCreateInfo instanceCI = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		                                   .pNext                   = NULL,
		                                   .flags                   = 0,
		                                   .pApplicationInfo        = &appInfo,
		                                   .enabledLayerCount       = enabledLayerCount,
		                                   .ppEnabledLayerNames     = enabledLayers,
		                                   .enabledExtensionCount   = enabledExtensionCount,
		                                   .ppEnabledExtensionNames = enabledExtensions};

		VkDebugUtilsMessengerCreateInfoEXT debugCI = {};
		if (enableValidation) {
			VulkanDebug_PopulateMessengerInfo(&debugCI);
			instanceCI.pNext = &debugCI;
		}

		createResult = context->vk.CreateInstance(&instanceCI, &context->Allocator, &vkInstance);
	}

	// Cleanup
	DynArray_Destroy(&enabledExtensions);
	DynArray_Destroy(&enabledLayers);
	DynArray_Destroy(&availableExtensions);
	DynArray_Destroy(&availableLayers);

	if (createResult == VK_SUCCESS) {
		LogD("[Vulkan] Instance created.");
		context->Validation = enableValidation;
		context->Instance   = vkInstance;

		// Load all of our instance-level functions
#define LoadInstanceFn(fn, ext)                                                                        \
	do {                                                                                                 \
		if ((context->vk.fn = (PFN_vk##fn) vkGetInstanceProcAddr(vkInstance, "vk" #fn)) == NULL && !ext) { \
			LogE("[VulkanEngine] Failed to load instance function '%s'!", "vk" #fn);                         \
			return FALSE;                                                                                    \
		}                                                                                                  \
	} while (0)
		// Core 1.0
		LoadInstanceFn(CreateDevice, FALSE);
		LoadInstanceFn(DestroyDevice, FALSE);
		LoadInstanceFn(DestroyInstance, FALSE);
		LoadInstanceFn(EnumerateDeviceExtensionProperties, FALSE);
		LoadInstanceFn(EnumeratePhysicalDevices, FALSE);
		LoadInstanceFn(GetDeviceProcAddr, FALSE);
		LoadInstanceFn(GetPhysicalDeviceFeatures, FALSE);
		LoadInstanceFn(GetPhysicalDeviceFormatProperties, FALSE);
		LoadInstanceFn(GetPhysicalDeviceMemoryProperties, FALSE);
		LoadInstanceFn(GetPhysicalDeviceProperties, FALSE);
		LoadInstanceFn(GetPhysicalDeviceQueueFamilyProperties, FALSE);

		// VK_KHR_surface
		LoadInstanceFn(DestroySurfaceKHR, TRUE);
		LoadInstanceFn(GetPhysicalDeviceSurfaceCapabilitiesKHR, TRUE);
		LoadInstanceFn(GetPhysicalDeviceSurfaceFormatsKHR, TRUE);
		LoadInstanceFn(GetPhysicalDeviceSurfacePresentModesKHR, TRUE);
		LoadInstanceFn(GetPhysicalDeviceSurfaceSupportKHR, TRUE);

		// VK_EXT_debug_utils
		LoadInstanceFn(CmdBeginDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(CmdEndDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(CmdInsertDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(CreateDebugUtilsMessengerEXT, TRUE);
		LoadInstanceFn(DestroyDebugUtilsMessengerEXT, TRUE);
		LoadInstanceFn(QueueBeginDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(QueueEndDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(QueueInsertDebugUtilsLabelEXT, TRUE);
		LoadInstanceFn(SetDebugUtilsObjectNameEXT, TRUE);
		LoadInstanceFn(SetDebugUtilsObjectTagEXT, TRUE);
		LoadInstanceFn(SubmitDebugUtilsMessageEXT, TRUE);
#undef LoadInstanceFn
	}

	return createResult;
}

void VulkanInstance_Destroy(VulkanContext* context) {
	context->vk.DestroyInstance(context->Instance, &context->Allocator);
}
