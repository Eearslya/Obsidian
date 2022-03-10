#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Renderer/RenderEngine.h>
#include <Obsidian/Renderer/Vulkan/VulkanEngine.h>

B8 RenderEngine_Create(RenderEngineType type, struct PlatformStateT* platform, RenderEngine* engine) {
	RenderEngine ptr = Memory_Allocate(sizeof(struct RenderEngineT), MemoryTag_Renderer);
	if (ptr == NULL) { return FALSE; }
	Memory_Zero(ptr, sizeof(struct RenderEngineT));

	switch (type) {
		case RenderEngineType_Vulkan:
			ptr->Initialize = RenderEngine_Vulkan_Initialize;
			ptr->Shutdown   = RenderEngine_Vulkan_Shutdown;
			ptr->BeginFrame = RenderEngine_Vulkan_BeginFrame;
			ptr->EndFrame   = RenderEngine_Vulkan_EndFrame;
			break;
		default:
			LogE("[RenderEngine] RenderEngineType %d is invalid or not yet implemented!", type);
			Memory_Free(ptr);

			return FALSE;
	}

	*engine = ptr;

	return TRUE;
}

void RenderEngine_Destroy(RenderEngine engine) {
	Memory_Free(engine);
}
