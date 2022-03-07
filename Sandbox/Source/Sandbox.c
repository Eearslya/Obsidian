#include <Obsidian/Obsidian.h>

typedef struct GameStateT {
} GameState;

static GameState Game;

B8 Game_Initialize(Application app) {
	LogI("Application initialized.");

	return TRUE;
}

B8 Game_Update(Application app, F32 deltaTime) {
	return TRUE;
}

B8 Game_Render(Application app, F32 deltaTime) {
	return TRUE;
}

void Game_OnResized(Application app, U32 width, U32 height) {}

B8 GetApplicationInfo(ApplicationCreateInfo* createInfo) {
	createInfo->WindowX              = -1;
	createInfo->WindowY              = -1;
	createInfo->WindowW              = 1600;
	createInfo->WindowH              = 900;
	createInfo->Name                 = "Sandbox";
	createInfo->Callbacks.Initialize = Game_Initialize;
	createInfo->Callbacks.Update     = Game_Update;
	createInfo->Callbacks.Render     = Game_Render;
	createInfo->Callbacks.OnResized  = Game_OnResized;
	createInfo->UserData             = &Game;

	return TRUE;
}
