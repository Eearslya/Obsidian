/** @file
 *  @brief Engine entrypoint, contains main(). */
#pragma once

#include <Obsidian/Core/Application.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Defines.h>

/** Function to be implemented by user application. */
extern B8 GetApplicationInfo(ApplicationCreateInfo* createInfo);

int main(int argc, const char** argv) {
	int status = 0;
	ApplicationCreateInfo info;
	Application app = NULL;

	if (!Memory_Initialize()) {
		status = 2;
		goto Shutdown;
	}

	// Initialize the logger.
	Logger_Initialize();

	if (GetApplicationInfo(&info) == FALSE) {
		status = 1;
		goto Shutdown;
	}

	if (Application_Create(&info, &app) == FALSE) {
		status = 1;
		goto Shutdown;
	}

	if (!Application_Run(app)) {
		LogE("Application exited abnormally.");
		status = 1;
	}

Shutdown:
	Application_Shutdown(app);
	Logger_Shutdown();
	Memory_Shutdown();

	return status;
}
