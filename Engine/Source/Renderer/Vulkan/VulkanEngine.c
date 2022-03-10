#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/Vulkan/Common.h>
#include <Obsidian/Renderer/Vulkan/VulkanEngine.h>

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

B8 RenderEngine_Vulkan_Initialize(RenderEngine engine, const char* appName, struct PlatformStateT* platform) {
	Memory_Zero(&Vulkan, sizeof(struct VulkanContextT));

	// Set up allocator callbacks
	{
		Vulkan.Allocator.pfnAllocation   = Vulkan_Allocate;
		Vulkan.Allocator.pfnReallocation = Vulkan_Reallocate;
		Vulkan.Allocator.pfnFree         = Vulkan_Free;
	}

	// Load global Vulkan functions
	{
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
	}

	// Find Vulkan instance version
	U32 instanceVersion = 0;
	{
		if (Vulkan.vk.EnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS) {
			LogE("[VulkanEngine] Failed to fetch Vulkan instance version!");
			return FALSE;
		}
		LogI("[VulkanEngine] Vulkan Instance Version %d.%d.%d",
		     VK_VERSION_MAJOR(instanceVersion),
		     VK_VERSION_MINOR(instanceVersion),
		     VK_VERSION_PATCH(instanceVersion));
	}

	// Create instance
	{
		const VkApplicationInfo appInfo       = {.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		                                         .pNext              = NULL,
		                                         .pApplicationName   = appName,
		                                         .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		                                         .pEngineName        = "Obsidian",
		                                         .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
		                                         .apiVersion         = VK_API_VERSION_1_0};
		const VkInstanceCreateInfo instanceCI = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		                                         .pNext                   = NULL,
		                                         .flags                   = 0,
		                                         .pApplicationInfo        = &appInfo,
		                                         .enabledLayerCount       = 0,
		                                         .ppEnabledLayerNames     = NULL,
		                                         .enabledExtensionCount   = 0,
		                                         .ppEnabledExtensionNames = NULL};
		if (Vulkan.vk.CreateInstance(&instanceCI, &Vulkan.Allocator, &Vulkan.Instance) != VK_SUCCESS) {
			LogE("[VulkanEngine] Failed to create Vulkan instance!");
			return FALSE;
		}
		LogD("[VulkanEngine] Instance created.");
	}

#define LoadInstanceFn(fn)                                                                        \
	do {                                                                                            \
		if ((Vulkan.vk.fn = (PFN_vk##fn) vkGetInstanceProcAddr(Vulkan.Instance, "vk" #fn)) == NULL) { \
			LogE("[VulkanEngine] Failed to load instance function '%s'!", "vk" #fn);                    \
			return FALSE;                                                                               \
		}                                                                                             \
	} while (0)
	LoadInstanceFn(DestroyInstance);
#undef LoadInstanceFn

	return TRUE;
}

void RenderEngine_Vulkan_Shutdown(RenderEngine engine) {
	if (Vulkan.Instance) { Vulkan.vk.DestroyInstance(Vulkan.Instance, &Vulkan.Allocator); }
}

B8 RenderEngine_Vulkan_BeginFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}

B8 RenderEngine_Vulkan_EndFrame(RenderEngine engine, F64 deltaTime) {
	return TRUE;
}
