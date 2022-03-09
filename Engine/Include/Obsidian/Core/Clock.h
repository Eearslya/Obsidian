#pragma once

#include <Obsidian/Defines.h>

typedef struct Clock {
	F64 StartTime;
	F64 Elapsed;
} Clock;

OAPI void Clock_Start(Clock* clock);
OAPI void Clock_Stop(Clock* clock);
OAPI void Clock_Update(Clock* clock);
