#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>

typedef struct DynArrayMetadataT {
	U64 Capacity;  // Amount of elements the array has memory for.
	U64 Size;      // Amount of elements the array currently contains.
	U64 Stride;    // The size of each element.
} DynArrayMetadata;

// If we attempt to push to a dynamic array with no remaining capacity, mutltiply the capacity by this number.
static const F32 DynArrayResizeFactor = 1.5f;

// Get a pointer to the dynamic array's metadata.
static DynArrayMetadata* DynArrayGetMetadata(const void* dynArray) {
	return (DynArrayMetadata*) (dynArray - sizeof(DynArrayMetadata));
}

void* _DynArray_Create(U64 elementSize, U64 elementCount) {
	const size_t metadataSize = sizeof(DynArrayMetadata);
	const size_t arraySize    = elementSize * elementCount;
	const size_t totalSize    = metadataSize + arraySize;

	void* dynArray = Memory_Allocate(totalSize, MemoryTag_DynamicArray);
	if (dynArray == NULL) { return NULL; }

	// Zero-initialize all of the elements for convenience and safety.
	Memory_Zero(dynArray, totalSize);

	// Metadata is placed just before the dynamic array. The user is given the pointer just after the metadata ends.
	// This allows the user to use the pointer returned like a normal array of objects.
	void* returnPtr        = dynArray + metadataSize;
	DynArrayMetadata* meta = DynArrayGetMetadata(returnPtr);

	// Update our metadata.
	meta->Capacity = elementCount;
	meta->Size     = 0;
	meta->Stride   = elementSize;

	return returnPtr;
}

void _DynArray_Destroy(void** dynArray) {
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	// Pointer to the start of metadata is the same pointer we originally allocated.
	Memory_Free(meta);
}

U64 _DynArray_Capacity(const void** dynArray) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	return meta->Capacity;
}

U64 _DynArray_Size(const void** dynArray) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	return meta->Size;
}

U64 _DynArray_Stride(const void** dynArray) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	return meta->Stride;
}

B8 _DynArray_Trim(void** dynArray) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	// If capacity equals size already, there's nothing we need to do!
	if (meta->Capacity == meta->Size) { return TRUE; }

	const size_t metadataSize = sizeof(DynArrayMetadata);
	const size_t arraySize    = meta->Stride * meta->Size;
	const size_t totalSize    = metadataSize + arraySize;

	DynArrayMetadata* newMeta = Memory_Reallocate(meta, totalSize);
	// If we fail to reallocate, we return NULL here. The original dynamic array is still valid.
	if (newMeta == NULL) { return FALSE; }

	// Update metadata.
	newMeta->Capacity = newMeta->Size;
	// Careful of the pointer arithmetic.
	*dynArray = ((void*) newMeta) + metadataSize;

	return TRUE;
}

B8 _DynArray_Resize(void** dynArray, U64 elementCount) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	if (meta->Size == elementCount) { return TRUE; }

	// First ensure we have the capacity to create an array of this size.
	if (meta->Capacity < elementCount) {
		B8 reserved = DynArray_Reserve(dynArray, elementCount);
		if (!reserved) { return FALSE; }
		// Update the metadata pointer since we have reallocated.
		meta = DynArrayGetMetadata(*dynArray);
	}

	// If we have grown in size, we will zero-initialize all of the new elements.
	if (elementCount > meta->Size) {
		const U64 newElements = elementCount - meta->Size;   // Number of new elements
		const U64 newBytes    = newElements * meta->Stride;  // Number of bytes those elements take up
		void* firstEmpty =
			(*dynArray) + (meta->Stride * meta->Size);  // Pointer to the first new element that needs to be initialized
		Memory_Zero(firstEmpty, newBytes);
	}

	meta->Size = elementCount;

	return TRUE;
}

B8 _DynArray_Reserve(void** dynArray, U64 elementCount) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	// Only reallocate if the requested size is larger than current. Shrinking must be handled by _DynArray_Trim().
	if (meta->Capacity >= elementCount) { return TRUE; }

	const size_t metadataSize = sizeof(DynArrayMetadata);
	const size_t arraySize    = meta->Stride * elementCount;
	const size_t totalSize    = metadataSize + arraySize;

	DynArrayMetadata* newMeta = Memory_Reallocate(meta, totalSize);
	if (newMeta == NULL) { return FALSE; }

	newMeta->Capacity = elementCount;
	*dynArray         = ((void*) newMeta) + metadataSize;

	return TRUE;
}

void _DynArray_Push(void** dynArray, const void* element) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	// Simply use the insert function to insert at the end of the array.
	_DynArray_Insert(dynArray, meta->Size, element);
}

void _DynArray_Pop(void** dynArray, void* element) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	// Use the extract function to take from the end of the array.
	_DynArray_Extract(dynArray, meta->Size - 1, element);
}

void _DynArray_Insert(void** dynArray, U64 index, const void* element) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	AssertMsg(index <= meta->Size, "DynArray insertion index is out of bounds!");

	// First ensure we have the capacity to insert into the array.
	if (meta->Capacity <= meta->Size) {
		// If not, use the resize factor to determine our new size. The resize factor is used rather than simply
		// incrementing by one to reduce the number of copies needed during times of many insertions back-to-back.
		U64 newCount = (F32) meta->Capacity * DynArrayResizeFactor;
		// Because of integer rounding, it is possible that the previous multiplication returns the exact same capacity.
		// Therefore, we need to make sure we at least increase by 1 no matter what.
		if (newCount == meta->Capacity) { newCount = meta->Capacity + 1; }
		const B8 resized = _DynArray_Reserve(dynArray, newCount);
		if (!resized) { return; }
		// Update metadata since we have reallocated.
		meta = DynArrayGetMetadata(*dynArray);
	}

	// If we're not inserting at the very end of the array, we need to shift all of the existing values to make room.
	if (index <= (meta->Size - 1)) {
		const U64 indexCount     = meta->Size - index;                    // Number of indices that need to shift
		const size_t bytesToMove = indexCount * meta->Stride;             // Bytes those indices take up
		void* oldPosition        = (*dynArray) + (index * meta->Stride);  // Pointer to the first index that needs to move
		void* newPosition        = oldPosition + meta->Stride;            // Pointer to the new first index's location
		Memory_Move(newPosition, oldPosition, bytesToMove);  // Memory Move is required as the two blocks overlap
	}

	// Finally, copy in our new value.
	void* ptr = (*dynArray) + (meta->Stride * index);
	Memory_Copy(ptr, element, meta->Stride);

	meta->Size++;
}

void _DynArray_Extract(void** dynArray, U64 index, void* element) {
	Assert(dynArray && *dynArray);
	DynArrayMetadata* meta = DynArrayGetMetadata(*dynArray);

	AssertMsg(meta->Size > 0, "Attempting to extract from empty DynArray!");
	AssertMsg(index < meta->Size, "DynArray extraction index is out of bounds!");

	if (element != NULL) {
		const void* ptr = (*dynArray) + (meta->Stride * index);
		Memory_Copy(element, ptr, meta->Stride);
	}

	// If we didn't pop from the end of the array, we need to shift all the existing values back to fill the gap.
	if (index < (meta->Size - 1)) {
		const U64 firstIndex     = index + 1;                           // First index that needs to move
		const U64 indexCount     = meta->Size - index - 1;              // Number of indices that need to move
		const size_t bytesToMove = indexCount * meta->Stride;           // Bytes those indices take up
		void* oldPosition = (*dynArray) + (firstIndex * meta->Stride);  // Pointer to the first index that needs to move
		void* newPosition = oldPosition - meta->Stride;                 // Pointer to the new first index's location
		Memory_Copy(newPosition,
		            oldPosition,
		            bytesToMove);  // Memory Move is not required here because the destination is located before the source
	}

	meta->Size--;
}
