#include <Obsidian/Core/Memory.h>
#include <Obsidian/Core/String.h>
#include <string.h>

U64 String_Length(const char* str) {
	return strlen(str);
}

char* String_Duplicate(const char* str) {
	const U64 len = String_Length(str);
	char* copy    = Memory_Allocate(len + 1, MemoryTag_String);
	Memory_Copy(copy, str, len + 1);

	return copy;
}

B8 String_Equal(const char* a, const char* b) {
	return strcmp(a, b) == 0;
}
