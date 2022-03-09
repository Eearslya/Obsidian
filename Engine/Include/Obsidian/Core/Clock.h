/** @file
 *  @brief Simple clock object */
#pragma once

#include <Obsidian/Defines.h>

/** Represents a "running" clock. */
typedef struct Clock {
	F64 StartTime; /**< Time the clock was started. */
	F64 Elapsed;   /**< Elapsed time in seconds, as of the last Clock_Update() call. */
} Clock;

/**
 * Start the clock.
 * @param clock The clock to start.
 */
OAPI void Clock_Start(Clock* clock);

/**
 * Stop the clock. Subsequent calls to Clock_Update() will not modify elapsed time.
 * @param clock The clock to stop.
 */
OAPI void Clock_Stop(Clock* clock);

/**
 * Update the clock. This function should be called any time you intend to read elapsed time.
 * @param clock The clock to update.
 */
OAPI void Clock_Update(Clock* clock);
