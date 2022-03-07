#include <Obsidian/Obsidian.h>

int main(int argc, const char** argv) {
	PlatformState platform = NULL;
	if (!Platform_Initialize(&platform, "Sandbox", -1, -1, 1600, 900)) { return 1; }

	while (Platform_Update(platform)) {}

	Platform_Shutdown(platform);

	return 0;
}
