#include <Obsidian/Core/Application.h>
#include <Obsidian/Core/Clock.h>
#include <Obsidian/Core/Event.h>
#include <Obsidian/Core/Input.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Platform/Platform.h>
#include <Obsidian/Renderer/Renderer.h>

struct ApplicationT {
	PlatformState Platform;
	ApplicationCallbacks Callbacks;
	B8 Running;
	Clock MainClock;
	F64 LastUpdate;
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

	// Initialize the rendering system.
	if (!Renderer_Initialize(createInfo->Name, (*app)->Platform)) {
		LogF("[Application] Failed to initialize Rendering system!");

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

	const F64 targetFps = 1.0 / 60.0;

	Clock_Start(&app->MainClock);
	Clock_Update(&app->MainClock);
	app->LastUpdate = app->MainClock.Elapsed;

	F64 runtime   = 0.0;
	U8 frameCount = 0;

	B8 badShutdown = FALSE;
	app->Running   = TRUE;
	while (app->Running) {
		Clock_Update(&app->MainClock);
		const F64 now            = app->MainClock.Elapsed;
		const F64 deltaTime      = now - app->LastUpdate;
		const F64 frameStartTime = Platform_GetAbsoluteTime();

		if (!Platform_Update(app->Platform)) { app->Running = FALSE; }

		if (!app->Callbacks.Update(app, deltaTime)) {
			LogF("[Application] Error encountered in appliation update loop.");
			app->Running = FALSE;
			badShutdown  = TRUE;
			break;
		}

		if (!app->Callbacks.Render(app, deltaTime)) {
			LogF("[Application] Error encountered in appliation render loop.");
			app->Running = FALSE;
			badShutdown  = TRUE;
			break;
		}

		RenderPacket packet = {.DeltaTime = deltaTime};
		Renderer_DrawFrame(&packet);

		Input_Update(deltaTime);

		const F64 frameEndTime = Platform_GetAbsoluteTime();
		const F64 frameTime    = frameEndTime - frameStartTime;
		runtime += frameTime;
		const F64 spareTime = targetFps - frameTime;
		if (spareTime > 0.0) {
			const U64 spareMs = spareTime * 1000;
			if (spareMs > 1) { Platform_Sleep(spareMs - 1); }
			frameCount++;
		}

		app->LastUpdate = now;
	}
	app->Running = FALSE;
	app->Callbacks.Shutdown(app);
	Renderer_Shutdown();
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
