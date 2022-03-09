#include <Obsidian/Core/Clock.h>
#include <Obsidian/Platform/Platform.h>

void Clock_Start(Clock* clock) {
	clock->StartTime = Platform_GetAbsoluteTime();
	clock->Elapsed   = 0.0;
}

void Clock_Stop(Clock* clock) {
	clock->StartTime = 0.0;
}

void Clock_Update(Clock* clock) {
	if (clock->StartTime != 0.0) { clock->Elapsed = Platform_GetAbsoluteTime() - clock->StartTime; }
}
