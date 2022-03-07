#include <Obsidian/Core/Application.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Platform/Platform.h>

struct ApplicationT {
	PlatformState Platform;
	ApplicationCallbacks Callbacks;
	B8 Running;
	void* UserData;
};

B8 Application_Create(const ApplicationCreateInfo* createInfo, Application* app) {
	// Initialize the logger.
	Logger_Initialize();

	*app = Platform_Alloc(sizeof(struct ApplicationT));
	Platform_MemZero(*app, sizeof(struct ApplicationT));

	if (createInfo->Callbacks.Initialize == NULL || createInfo->Callbacks.Update == NULL ||
	    createInfo->Callbacks.Render == NULL) {
		LogF("[Application] Cannot create an application without callbacks for Initialize, Update, and Render!");

		return FALSE;
	}

	(*app)->Callbacks = createInfo->Callbacks;
	(*app)->UserData  = createInfo->UserData;

	if (!Platform_Initialize(&(*app)->Platform,
	                         createInfo->Name,
	                         createInfo->WindowX,
	                         createInfo->WindowY,
	                         createInfo->WindowW,
	                         createInfo->WindowH)) {
		LogF("[Application] Failed to initialize Platform layer!");

		return FALSE;
	}

	if (!(*app)->Callbacks.Initialize(*app)) {
		LogF("[Application] Application failed to initialize!");

		return FALSE;
	}

	if ((*app)->Callbacks.OnResized) { (*app)->Callbacks.OnResized(*app, createInfo->WindowW, createInfo->WindowH); }

	return TRUE;
}

B8 Application_Run(Application app) {
	AssertMsg(!app->Running, "Application is already running!");

	B8 badShutdown = FALSE;
	app->Running   = TRUE;
	while (app->Running) {
		if (!Platform_Update(app->Platform)) { app->Running = FALSE; }

		if (!app->Callbacks.Update(app, 0.0f)) {
			LogF("[Application] Error encountered in appliation update loop.");
			app->Running = FALSE;
			badShutdown  = TRUE;
			break;
		}

		if (!app->Callbacks.Render(app, 0.0f)) {
			LogF("[Application] Error encountered in appliation render loop.");
			app->Running = FALSE;
			badShutdown  = TRUE;
			break;
		}
	}
	app->Running = FALSE;
	Platform_Shutdown(app->Platform);

	return badShutdown == FALSE;
}

void* Application_GetUserData(Application app) {
	return app->UserData;
}
