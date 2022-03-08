#pragma once

#include <Obsidian/Core/Application.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Defines.h>

extern B8 GetApplicationInfo(ApplicationCreateInfo* createInfo);

int main(int argc, const char** argv) {
	if (!Memory_Initialize()) { return 1; }

	ApplicationCreateInfo info;
	if (GetApplicationInfo(&info) == FALSE) { return 1; }

	Application app;
	if (Application_Create(&info, &app) == FALSE) { return 1; }

	if (!Application_Run(app)) {
		LogE("Application exited abnormally.");
		return 1;
	}

	Memory_Shutdown();

	return 0;
}
