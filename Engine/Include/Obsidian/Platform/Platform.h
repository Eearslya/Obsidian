/** @file
 *  @brief Platform-specific interface for interacting with the operating system.
 */
#pragma once

#include <Obsidian/Defines.h>

/** Platform state object, used when interacting with the host hardware and operating system. */
typedef struct PlatformStateT* PlatformState;

/**
 *  Initialize the platform layer.
 *  @param[out] state A pointer to a PlatformState object. The function will allocate and initialize the object.
 *  @param appName The application's name.
 *  @param windowX The initial window's starting X position. A value of -1 indicates the window should be horizontally
 * centered.
 *  @param windowY The initial window's starting Y position. A value of -1 indicates the window should be vertically
 * centered.
 *  @param windowW The initial window's starting width.
 *  @param windowH The initial window's starting height.
 *  @return TRUE on success, FALSE on error.
 *  @sa Platform_Shutdown()
 */
OAPI B8
Platform_Initialize(PlatformState* state, const char* appName, I32 windowX, I32 windowY, I32 windowW, I32 windowH);

/**
 * Shuts down the platform layer.
 * @param state A PlatformState object, as previous initialized with Platform_Initialize().
 */
OAPI void Platform_Shutdown(PlatformState state);

/**
 * Update the platform layer.
 * @param state A PlatformState object previously returned by Platform_Initialize().
 * @return TRUE if the application should continue. FALSE if the application should terminate.
 */
OAPI B8 Platform_Update(PlatformState state);

/**
 * Allocate a memory block.
 * @param bytes Size of the desired memory block.
 * @return NULL upon failure, otherwise a pointer to the block of memory.
 * @sa Platform_Free()
 */
void* Platform_Alloc(size_t bytes);

/**
 * Allocate a memory block, aligned to a specified offset.
 * @param bytes Size of the desired memory block.
 * @param align The desired alignment.
 * @return NULL upon failure, otherwise a pointer to the block of memory.
 * @sa Platform_AlignedFree()
 */
void* Platform_AlignedAlloc(size_t bytes, U8 align);

/**
 * Free an allocated memory block.
 * @param ptr A pointer to the start of the allocated memory.
 * @sa Platform_Alloc()
 */
void Platform_Free(void* ptr);

/**
 * Free an allocated memory block that was aligned.
 * @param ptr A pointer to the start of the allocated memory.
 * @sa Platform_AlignedAlloc()
 */
void Platform_AlignedFree(void* ptr);

/**
 * Copy an area of memory to another area.
 * @param dst A pointer to the destination of the copy.
 * @param src A pointer to the source of the copy.
 * @param bytes The number of bytes to copy.
 */
void Platform_MemCopy(void* dst, const void* src, size_t bytes);

/**
 * Set all bytes in the specified area to a value.
 * @param ptr A pointer to the start of the memory area.
 * @param value The value to set each byte to.
 * @param bytes The number of bytes to write.
 * @sa Platform_MemZero()
 */
void Platform_MemSet(void* ptr, U8 value, size_t bytes);

/**
 * Set all bytes in the specified area to 0.
 * @param ptr A pointer to the start of the memory area.
 * @param bytes The number of bytes to write.
 * @sa Platform_MemSet()
 */
void Platform_MemZero(void* ptr, size_t bytes);

/**
 * Write a message to the console.
 * @param msg The string to output.
 */
void Platform_ConsoleOut(const char* msg);

/**
 * Write a message to the console's error output.
 * @param msg The string to output.
 */
void Platform_ConsoleError(const char* msg);

/**
 * Get the value of a monotonically increasing clock.
 * @return The current value of the clock.
 */
F64 Platform_GetAbsoluteTime();

/**
 * Pause execution of the current thread.
 * @param ms The number of milliseconds to sleep.
 */
void Platform_Sleep(U64 ms);
