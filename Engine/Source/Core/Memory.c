#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>
#include <Obsidian/Platform/Platform.h>
#include <stdio.h>

static const char* MemoryTagNames[MemoryTag_End] = {"Unknown",
                                                    "Internal",
                                                    "Array",
                                                    "BinarySearchTree",
                                                    "Dictionary",
                                                    "DynamicArray",
                                                    "RingQueue",
                                                    "String",
                                                    "Application",
                                                    "Game",
                                                    "Job",
                                                    "Renderer",
                                                    "Texture",
                                                    "MaterialInstance",
                                                    "Scene",
                                                    "Entity",
                                                    "EntityNode",
                                                    "Transform"};

struct AllocationT {
	U64 Size;
	U32 Alignment;
	U32 Tag;
};

struct MemoryStatsT {
	size_t TotalAllocations;
	size_t TotalAllocatedBytes;
	size_t AllocationsByTag[MemoryTag_End];
	size_t AllocatedBytesByTag[MemoryTag_End];
};
static struct MemoryStatsT MemoryStats;

/** Determine how many bytes we need to add to the allocation to fit our metadata, while keeping the requested
 * alignment. */
static size_t GetTrackingOverhead(U8 align) {
	return (align < sizeof(struct AllocationT)) ? sizeof(struct AllocationT) : align;
}

/** Get the allocation metadata from the user-visible pointer. */
static struct AllocationT* GetAllocationMetadata(void* ptr) {
	return (struct AllocationT*) (ptr - sizeof(struct AllocationT));
}

static void FormatMemoryUsage(char* buffer, size_t bufferSize, size_t bytes) {
	const U64 gib = 1024 * 1024 * 1024;
	const U64 mib = 1024 * 1024;
	const U64 kib = 1024;

	if (bytes >= gib) {
		snprintf(buffer, bufferSize, "%.2f GiB", (float) bytes / (float) gib);
	} else if (bytes >= mib) {
		snprintf(buffer, bufferSize, "%.2f MiB", (float) bytes / (float) mib);
	} else if (bytes >= kib) {
		snprintf(buffer, bufferSize, "%.2f KiB", (float) bytes / (float) kib);
	} else {
		snprintf(buffer, bufferSize, "%lld B", bytes);
	}
}

B8 Memory_Initialize() {
	Memory_Zero(&MemoryStats, sizeof(MemoryStats));

	return TRUE;
}

void Memory_Shutdown() {
#if OBSIDIAN_DEBUG == 1
	// Memory leak check
	if (MemoryStats.TotalAllocatedBytes != 0) {
		LogW("[Memory] %lld bytes are still allocated at program termination!", MemoryStats.TotalAllocatedBytes);
	}
#endif
}

void* Memory_Allocate(size_t size, MemoryTag tag) {
	// An allocation of 0 bytes is treated the same as if it were unaligned.
	return Memory_AllocateAligned(size, 0, tag);
}

void* Memory_AllocateAligned(size_t size, U8 align, MemoryTag tag) {
	// Refuse to allocate a block of 0 bytes.
	if (size == 0) { return NULL; }

	// Validate the memory tag used.
	AssertMsg(tag < MemoryTag_End, "Invalid memory tag!");
	if (tag == MemoryTag_Unknown) {
		LogW("[Memory] Allocating %lld bytes under 'Unknown'. Consider classifying this allocation.");
	}

	const size_t trackingOverhead = GetTrackingOverhead(align);
	const size_t actualSize       = size + trackingOverhead;

	// Allocate the requested memory, plus a block large enough for our metadata.
	// If alignment is 0, perform a normal allocation instead.
	void* ptr = NULL;
	if (align == 0) {
		ptr = Platform_Alloc(actualSize);
	} else {
		ptr = Platform_AllocAligned(actualSize, align);
	}
	if (ptr == NULL) {
		LogE("[Memory] Failed to allocate %lld bytes for %s!", size, MemoryTagNames[tag]);
		return NULL;
	}

	// The pointer the user gets back will skip over our metadata.
	void* returnPtr = ptr + trackingOverhead;

	// Metadata will be placed immediately before the user data so it can be easily located when freeing.
	// This may leave empty bytes at the beginning of the actual allocation.
	struct AllocationT* tracking = GetAllocationMetadata(returnPtr);
	tracking->Size               = size;
	tracking->Alignment          = align;
	tracking->Tag                = tag;

	// Update memory statistics.
	MemoryStats.TotalAllocations++;
	MemoryStats.AllocationsByTag[tag]++;
	MemoryStats.TotalAllocatedBytes += actualSize;
	MemoryStats.AllocatedBytesByTag[MemoryTag_Internal] += trackingOverhead;
	MemoryStats.AllocatedBytesByTag[tag] += size;

	return returnPtr;
}

void* Memory_Reallocate(void* ptr, size_t size) {
	if (ptr == NULL) { return NULL; }

	// Fetch allocation metadata and find our actual pointer.
	struct AllocationT* tracking  = GetAllocationMetadata(ptr);
	const size_t trackingOverhead = GetTrackingOverhead(tracking->Alignment);
	const size_t oldSize          = tracking->Size;
	const size_t actualSize       = oldSize + trackingOverhead;
	void* actualPtr               = ptr - trackingOverhead;

	// Calculate our new size.
	const size_t newActualSize = size + trackingOverhead;

	// Reallocate and get our new pointer.
	void* newActualPtr = NULL;
	if (tracking->Alignment == 0) {
		newActualPtr = Platform_Realloc(actualPtr, newActualSize);
	} else {
		newActualPtr = Platform_ReallocAligned(actualPtr, tracking->Alignment, newActualSize);
	}
	if (newActualPtr == NULL) {
		LogE("[Memory] Failed to reallocate '%s' memory from %lld to %lld bytes!",
		     MemoryTagNames[tracking->Tag],
		     tracking->Size,
		     size);

		return NULL;
	}
	void* returnPtr = newActualPtr + trackingOverhead;

	// Update metadata and statistics.
	struct AllocationT* newTracking = GetAllocationMetadata(returnPtr);
	newTracking->Size               = size;
	MemoryStats.TotalAllocatedBytes -= actualSize;
	MemoryStats.TotalAllocatedBytes += newActualSize;
	MemoryStats.AllocatedBytesByTag[newTracking->Tag] -= oldSize;
	MemoryStats.AllocatedBytesByTag[newTracking->Tag] += size;

	return returnPtr;
}

void Memory_Free(void* ptr) {
	if (ptr == NULL) { return; }

	// Fetch allocation metadata and find our actual pointer.
	struct AllocationT* tracking  = GetAllocationMetadata(ptr);
	const size_t trackingOverhead = GetTrackingOverhead(tracking->Alignment);
	const size_t actualSize       = tracking->Size + trackingOverhead;
	void* actualPtr               = ptr - trackingOverhead;

#if OBSIDIAN_DEBUG == 1
	// Perform sanity checks against double-frees.
	if (MemoryStats.TotalAllocations == 0) {
		LogE("[Memory] Possible double-free: Freeing allocation with 0 total tracked allocations!");
	}
	if (MemoryStats.TotalAllocatedBytes < actualSize) {
		LogE("[Memory] Possible double-free: Freeing allocation of %lld bytes with %lld total tracked bytes!",
		     actualSize,
		     MemoryStats.TotalAllocatedBytes);
	}
	if (MemoryStats.AllocatedBytesByTag[tracking->Tag] < tracking->Size) {
		LogE("[Memory] Possible double-free: Freeing '%s' allocation of %lld bytes with %lld tracked bytes!",
		     MemoryTagNames[tracking->Tag],
		     actualSize,
		     MemoryStats.TotalAllocatedBytes);
	}
#endif

	// Update memory statistics.
	MemoryStats.TotalAllocations--;
	MemoryStats.TotalAllocatedBytes -= actualSize;
	MemoryStats.AllocationsByTag[tracking->Tag]--;
	MemoryStats.AllocatedBytesByTag[MemoryTag_Internal] -= trackingOverhead;
	MemoryStats.AllocatedBytesByTag[tracking->Tag] += tracking->Size;

	// Automatically deduce whether the allocation was aligned.
	if (tracking->Alignment == 0) {
		Platform_Free(actualPtr);
	} else {
		Platform_FreeAligned(actualPtr);
	}
}

void Memory_Copy(void* dst, const void* src, size_t bytes) {
	Platform_MemCopy(dst, src, bytes);
}

void Memory_Move(void* dst, const void* src, size_t bytes) {
	Platform_MemMove(dst, src, bytes);
}

void Memory_Set(void* dst, U8 value, size_t bytes) {
	Platform_MemSet(dst, value, bytes);
}

void Memory_Zero(void* dst, size_t bytes) {
	Platform_MemZero(dst, bytes);
}

void Memory_LogUsage() {
	char buffer[64];

	FormatMemoryUsage(buffer, 64, MemoryStats.TotalAllocatedBytes);
	LogD("[Memory] Current Memory Usage: %s (%lld allocations)", buffer, MemoryStats.TotalAllocations);

	for (U32 tag = 0; tag < MemoryTag_End; ++tag) {
		const size_t bytes = MemoryStats.AllocatedBytesByTag[tag];
		const size_t count = MemoryStats.AllocationsByTag[tag];
		if (count > 0 || bytes > 0) {
			FormatMemoryUsage(buffer, 64, bytes);
			LogD("[Memory] - %s: %s (%lld allocations)", MemoryTagNames[tag], buffer, count);
		}
	}
}
