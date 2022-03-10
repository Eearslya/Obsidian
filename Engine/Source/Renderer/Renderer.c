#include <Obsidian/Core/Logger.h>
#include <Obsidian/Renderer/RenderEngine.h>
#include <Obsidian/Renderer/Renderer.h>

static RenderEngine Engine = NULL;

B8 Renderer_Initialize(const char* appName, struct PlatformStateT* platform) {
	// TODO: Choose render engine at runtime.
	const RenderEngineType engineType = RenderEngineType_Vulkan;

	if (!RenderEngine_Create(engineType, platform, &Engine)) {
		LogE("[Renderer] Failed to create render engine!");

		return FALSE;
	}

	if (!Engine->Initialize(Engine, appName, platform)) {
		LogE("[Renderer] Failed to initialize render engine!");

		return FALSE;
	}

	return TRUE;
}

void Renderer_Shutdown() {
	if (Engine) {
		Engine->Shutdown(Engine);
		RenderEngine_Destroy(Engine);
	}
}

static B8 BeginFrame(const RenderPacket* packet) {
	return Engine->BeginFrame(Engine, packet->DeltaTime);
}

static B8 EndFrame(const RenderPacket* packet) {
	return Engine->EndFrame(Engine, packet->DeltaTime);
}

B8 Renderer_DrawFrame(const RenderPacket* packet) {
	if (BeginFrame(packet)) {
		if (!EndFrame(packet)) {
			LogE("[Renderer] An error occurred when rendering frame!");

			return FALSE;
		}
	}

	return TRUE;
}
