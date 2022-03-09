#include <Obsidian/Obsidian.h>

typedef struct GameStateT {
	Application App;
} GameState;

B8 Game_OnKeyPressed(U16 code, void* sender, void* listener, EventContext evt) {
	GameState* game = (GameState*) listener;

	const Key key = evt.Data.U16[0];
	if (key == Key_Escape) { Application_Shutdown(game->App); }

	return FALSE;
}

B8 Game_Initialize(Application app) {
	LogI("Application initialized.");

	GameState* state = Memory_Allocate(sizeof(GameState), MemoryTag_Game);
	Application_SetUserData(app, state);

	state->App = app;

	Event_Register(EventCode_KeyPressed, state, Game_OnKeyPressed);

	Memory_LogUsage();

	return TRUE;
}

B8 Game_Update(Application app, F32 deltaTime) {
	return TRUE;
}

B8 Game_Render(Application app, F32 deltaTime) {
	return TRUE;
}

void Game_Shutdown(Application app) {
	GameState* state = (GameState*) Application_GetUserData(app);
	if (state) { Memory_Free(state); }
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
	createInfo->Callbacks.Shutdown   = Game_Shutdown;
	createInfo->Callbacks.OnResized  = Game_OnResized;
	createInfo->UserData             = NULL;

	return TRUE;
}
