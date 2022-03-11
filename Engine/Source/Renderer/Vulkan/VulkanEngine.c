#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/Vulkan/Common.h>
#include <Obsidian/Renderer/Vulkan/VulkanDebug.h>
#include <Obsidian/Renderer/Vulkan/VulkanEngine.h>
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
	if (!Vulkan_LoadGlobalFunctions()) { return FALSE; }

	// Gather required instance extensions
	const char** instanceExtensions = DynArray_Create(const char*);
	Platform_Vulkan_GetRequiredInstanceExtensions((DynArrayT) &instanceExtensions);

	// Create instance
	const B8 instanceCreated = VulkanInstance_Create(&Vulkan, (ConstDynArrayT) &instanceExtensions) == VK_SUCCESS;
	DynArray_Destroy(&instanceExtensions);
	if (!instanceCreated) { return FALSE; }

	// Create debug messenger, if applicable
	if (Vulkan.Validation) {
		if (VulkanDebug_CreateMessenger(&Vulkan) != VK_SUCCESS) {
			LogW("[Vulkan] Failed to create Vulkan debug messenger.");
			Vulkan.Validation = FALSE;
		}
	}

	// Create platform surface
	const B8 surfaceCreated = Platform_Vulkan_CreateSurface(platform, &Vulkan);
	if (!surfaceCreated) {
		LogE("[Vulkan] Failed to create Vulkan surface!");

		return FALSE;
	}
	LogD("[Vulkan] Surface created.");

	return TRUE;
}

void RenderEngine_Vulkan_Shutdown(RenderEngine engine) {
	if (Vulkan.Instance) {
		if (Vulkan.Surface) { Vulkan.vk.DestroySurfaceKHR(Vulkan.Instance, Vulkan.Surface, &Vulkan.Allocator); }
		if (Vulkan.Validation) { VulkanDebug_DestroyMessenger(&Vulkan); }
		VulkanInstance_Destroy(&Vulkan);
	}
}

B8 RenderEngine_Vulkan_BeginFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}

B8 RenderEngine_Vulkan_EndFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}
