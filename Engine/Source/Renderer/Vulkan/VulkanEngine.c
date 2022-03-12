#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/Vulkan/Common.h>
#include <Obsidian/Renderer/Vulkan/VulkanDebug.h>
#include <Obsidian/Renderer/Vulkan/VulkanDevice.h>
#include <Obsidian/Renderer/Vulkan/VulkanEngine.h>
#include <Obsidian/Renderer/Vulkan/VulkanImage.h>
#include <Obsidian/Renderer/Vulkan/VulkanInstance.h>
#include <Obsidian/Renderer/Vulkan/VulkanPlatform.h>
#include <Obsidian/Renderer/Vulkan/VulkanStrings.h>

static VulkanContext Vulkan;

static void* Vulkan_Allocate(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope) {
	return Memory_AllocateAligned(size, alignment, MemoryTag_Renderer);
}

static void* Vulkan_Reallocate(
	void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope) {
	return Memory_Reallocate(pOriginal, size);
}

static void Vulkan_Free(void* pUserData, void* pMemory) {
	Memory_Free(pMemory);
}

void Vulkan_ReportFailure(const char* expr, VkResult result, const char* msg, const char* file, int line) {
	const char* resultName = VulkanString_VkResult(result);

	LogF("[Vulkan] Vulkan function failed with %s: %s", resultName, expr);
	if (msg != NULL) { LogF("[Vulkan]     %s", msg); }
	LogF("[Vulkan]     At: %s:%d", file, line);

	Assert(result == VK_SUCCESS);
}

static B8 Vulkan_LoadGlobalFunctions() {
#define LoadGlobalFn(fn)                                                                         \
	do {                                                                                           \
		if ((Vulkan.vk.fn = (PFN_vk##fn) vkGetInstanceProcAddr(VK_NULL_HANDLE, "vk" #fn)) == NULL) { \
			LogE("[VulkanEngine] Failed to load global function '%s'!", "vk" #fn);                     \
			return FALSE;                                                                              \
		}                                                                                            \
	} while (0)
	LoadGlobalFn(EnumerateInstanceVersion);
	LoadGlobalFn(EnumerateInstanceExtensionProperties);
	LoadGlobalFn(EnumerateInstanceLayerProperties);
	LoadGlobalFn(CreateInstance);
#undef LoadGlobalFn

	return TRUE;
}

B8 RenderEngine_Vulkan_Initialize(RenderEngine engine, const char* appName, struct PlatformStateT* platform) {
	Memory_Zero(&Vulkan, sizeof(struct VulkanContextT));

	// Set up allocator callbacks
	{
		Vulkan.Allocator.pfnAllocation   = Vulkan_Allocate;
		Vulkan.Allocator.pfnReallocation = Vulkan_Reallocate;
		Vulkan.Allocator.pfnFree         = Vulkan_Free;
	}

	// Load global Vulkan functions
	if (!Vulkan_LoadGlobalFunctions()) {
		LogE("[Vulkan] Failed to load Vulkan global functions!");
		RenderEngine_Vulkan_Shutdown(engine);

		return FALSE;
	}

	// Create instance
	{
		// Gather required instance extensions
		const char** instanceExtensions = DynArray_Create(const char*);
		Platform_Vulkan_GetRequiredInstanceExtensions((DynArrayT) &instanceExtensions);

		const VkResult instanceResult = VulkanInstance_Create(&Vulkan, (ConstDynArrayT) &instanceExtensions);

		DynArray_Destroy(&instanceExtensions);

		if (instanceResult != VK_SUCCESS) {
			LogE("[Vulkan] Failed to create Vulkan instance! (%s)", VulkanString_VkResult(instanceResult));
			RenderEngine_Vulkan_Shutdown(engine);

			return FALSE;
		}
	}

	// Create debug messenger, if applicable
	if (Vulkan.Validation) {
		if (VulkanDebug_CreateMessenger(&Vulkan) != VK_SUCCESS) {
			LogW("[Vulkan] Failed to create Vulkan debug messenger. Application will continue without validation.");
			Vulkan.Validation = FALSE;
		}
	}

	// Create platform surface
	{
		const B8 surfaceCreated = Platform_Vulkan_CreateSurface(platform, &Vulkan);
		if (!surfaceCreated) {
			LogE("[Vulkan] Failed to create Vulkan surface!");
			RenderEngine_Vulkan_Shutdown(engine);

			return FALSE;
		}
		LogD("[Vulkan] Surface created.");
	}

	// Create logical device
	{
		const VkResult deviceResult = VulkanDevice_Create(&Vulkan);
		if (deviceResult != VK_SUCCESS) {
			LogE("[Vulkan] Failed to create Vulkan device! (%s)", VulkanString_VkResult(deviceResult));
			RenderEngine_Vulkan_Shutdown(engine);

			return FALSE;
		}
	}

	return TRUE;
}

void RenderEngine_Vulkan_Shutdown(RenderEngine engine) {
	VulkanDevice_Destroy(&Vulkan);
	if (Vulkan.Instance) {
		if (Vulkan.Surface) {
			Vulkan.vk.DestroySurfaceKHR(Vulkan.Instance, Vulkan.Surface, &Vulkan.Allocator);
			Vulkan.Surface = VK_NULL_HANDLE;
		}
		VulkanDebug_DestroyMessenger(&Vulkan);
		VulkanInstance_Destroy(&Vulkan);
		Vulkan.Instance = VK_NULL_HANDLE;
	}
}

B8 RenderEngine_Vulkan_BeginFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}

B8 RenderEngine_Vulkan_EndFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}
