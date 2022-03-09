#include <Obsidian/Core/Application.h>
#include <Obsidian/Core/Event.h>
#include <Obsidian/Core/Input.h>
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

	// Create our application data.
	*app = Platform_Alloc(sizeof(struct ApplicationT));
	Platform_MemZero(*app, sizeof(struct ApplicationT));

	// Ensure we have the required callbacks. Some callbacks are optional.
	if (createInfo->Callbacks.Initialize == NULL || createInfo->Callbacks.Update == NULL ||
	    createInfo->Callbacks.Render == NULL || createInfo->Callbacks.Shutdown == NULL) {
		LogF("[Application] Cannot create an application without callbacks for Initialize, Update, Render, and Shutdown!");

		return FALSE;
	}

	// Copy data into our new application object.
	(*app)->Callbacks = createInfo->Callbacks;
	(*app)->UserData  = createInfo->UserData;

	// Initialize the system platform.
	if (!Platform_Initialize(&(*app)->Platform,
	                         createInfo->Name,
	                         createInfo->WindowX,
	                         createInfo->WindowY,
	                         createInfo->WindowW,
	                         createInfo->WindowH)) {
		LogF("[Application] Failed to initialize Platform layer!");

		return FALSE;
	}

	// Initialize the event system.
	if (!Event_Initialize()) {
		LogF("[Application] Failed to initialize Event system!");

		return FALSE;
	}

	// Initialize the input system.
	if (!Input_Initialize()) {
		LogF("[Application] Failed to initialize Input system!");

		return FALSE;
	}

	// Initialize the application.
	if (!(*app)->Callbacks.Initialize(*app)) {
		LogF("[Application] Application failed to initialize!");

		return FALSE;
	}

	// Send an event for the platform's starting size.
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

		Input_Update(0.0);
	}
	app->Running = FALSE;
	app->Callbacks.Shutdown(app);
	Input_Shutdown();
	Event_Shutdown();
	Platform_Shutdown(app->Platform);

	return badShutdown == FALSE;
}

void Application_Shutdown(Application app) {
	EventContext evt;
	if (Event_Fire(EventCode_ApplicationQuit, NULL, evt) == FALSE) { app->Running = FALSE; }
}

void Application_SetUserData(Application app, void* ptr) {
	app->UserData = ptr;
}

void* Application_GetUserData(Application app) {
	return app->UserData;
}
