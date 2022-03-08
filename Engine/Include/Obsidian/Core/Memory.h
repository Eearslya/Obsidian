/** @file
 *  @brief Memory subsystem */
#pragma once

#include <Obsidian/Defines.h>

/** Used to classify what a memory allocation will be used for. */
typedef enum MemoryTag {
	MemoryTag_Unknown,  /**< Default value, should never be used. */
	MemoryTag_Internal, /**< Used internally by the memory subsystem. */

	// Data Structures
	MemoryTag_Array,            /**< Used by static-sized arrays. */
	MemoryTag_BinarySearchTree, /**< Used by binary search trees. */
	MemoryTag_Dictionary,       /**< Used by dictionaries/hash maps. */
	MemoryTag_DynamicArray,     /**< Used by dynamically-sized arrays. */
	MemoryTag_RingQueue,        /**< Used by ring queues. */
	MemoryTag_String,           /**< Used by strings. */

	// Working Memory
	MemoryTag_Application, /**< Used by the engine application. */
	MemoryTag_Game,        /**< Used by the running game. */
	MemoryTag_Job,         /**< Used by the asynchronous job system. */

	// Renderer
	MemoryTag_Renderer,         /**< Used by the renderer. */
	MemoryTag_Texture,          /**< Used for texture/image data. */
	MemoryTag_MaterialInstance, /**< Used for material data. */

	// Entity-Component System
	MemoryTag_Scene,      /**< Used for the Scene system. */
	MemoryTag_Entity,     /**< Used for a Scene Entity. */
	MemoryTag_EntityNode, /**< Used for a Scene Entity's Nodes. */
	MemoryTag_Transform,  /**< Used for the Transform components. */

	MemoryTag_End /**< Count of total memory tags. */
} MemoryTag;

/**
 * Initialize the memory subsystem.
 * @return TRUE on success, FALSE otherwise.
 */
OAPI B8 Memory_Initialize();

/**
 * Shutdown the memory subsystem.
 */
OAPI void Memory_Shutdown();

/**
 * Allocate a block of memory from the platform.
 * @param size The number of bytes to allocate.
 * @param tag The tag which this memory relates to.
 * @return NULL upon allocation failure, otherwise a pointer to the requested block of memory.
 */
OAPI void* Memory_Allocate(size_t size, MemoryTag tag);

/**
 * Allocate a block of memory from the platform with a specific allocation.
 * @param size The number of bytes to allocate.
 * @param align The alignment to use when allocating.
 * @param tag The tag which this memory relates to.
 * @return NULL upon allocation failure, otherwise a pointer to the requested block of memory.
 */
OAPI void* Memory_AllocateAligned(size_t size, U8 align, MemoryTag tag);

/**
 * Free a previously requested block of memory.
 * @param ptr A pointer previously returned by Memory_Allocate() or Memory_AllocateAligned().
 */
OAPI void Memory_Free(void* ptr);

/**
 * Copy bytes from one area of memory to another.
 * @param dst A pointer to the destination memory.
 * @param src A pointer to the source memory.
 * @param bytes The number of bytes to copy.
 */
OAPI void Memory_Copy(void* dst, const void* src, size_t bytes);

/**
 * Set a block of memory to a specified value.
 * @param dst A pointer to the destination memory.
 * @param value The value to set each byte to.
 * @param bytes The number of bytes to write.
 */
OAPI void Memory_Set(void* dst, U8 value, size_t bytes);

/**
 * Set a block of memory to zero.
 * @param dst A pointer to the destination memory.
 * @param bytes The number of bytes to write.
 */
OAPI void Memory_Zero(void* dst, size_t bytes);

/**
 * Log the current memory usage to console.
 */
OAPI void Memory_LogUsage();
