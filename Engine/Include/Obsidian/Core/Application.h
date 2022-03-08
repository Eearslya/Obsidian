#pragma once

#include <Obsidian/Defines.h>

/** Represents the engine's running application. */
typedef struct ApplicationT* Application;

/** Callback functions which will be used by the engine throughout the application's lifetime. */
typedef struct ApplicationCallbacksT {
	B8 (*Initialize)(Application app);                         /**< Called during initial setup of the application. */
	B8 (*Update)(Application app, F32 deltaTime);              /**< Called once per update tick. */
	B8 (*Render)(Application app, F32 deltaTime);              /**< Called once per frame. */
	void (*Shutdown)(Application app);                         /**< Called upon application exit. */
	void (*OnResized)(Application app, U32 width, U32 height); /**< Called when the platform's window is resized. */
} ApplicationCallbacks;

/** Information required to create an application. */
typedef struct ApplicationCreateInfoT {
	I16 WindowX;                    /**< Initial X position of the main window. */
	I16 WindowY;                    /**< Initial Y position of the main window. */
	I16 WindowW;                    /**< Initial width of the main window. */
	I16 WindowH;                    /**< Initial height of the main window. */
	const char* Name;               /**< Name of the application. */
	ApplicationCallbacks Callbacks; /**< Application lifecycle callbacks. */
	void* UserData;                 /**< Pointer to any user-specified data. See Application_GetUserData(). */
} ApplicationCreateInfo;

/**
 * Create the application.
 * @param createInfo A pointer to a struct of information used to create the application.
 * @param[out] app The created application handle.
 * @return TRUE on successful creation, FALSE otherwise.
 */
OAPI B8 Application_Create(const ApplicationCreateInfo* createInfo, Application* app);

/**
 * Run the application.
 * @param app The application to run.
 * @return TRUE upon clean exit, FALSE if an error occurred.
 */
OAPI B8 Application_Run(Application app);

/**
 * Set the UserData pointer.
 * @param app The application to save to.
 * @param ptr The pointer to save.
 * @sa Application_GetUserData()
 */
OAPI void Application_SetUserData(Application app, void* ptr);

/**
 * Retrieves the UserData pointer passed in with ApplicationCreateInfo.
 * @param app The application to fetch the pointer for.
 * @return The user-specified data pointer.
 * @sa Application_Create(), ApplicationCreateInfo
 */
OAPI void* Application_GetUserData(Application app);
