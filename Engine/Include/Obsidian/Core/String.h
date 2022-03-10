/** @file
 *  @brief String manipulation functions */
#pragma once

#include <Obsidian/Core/Memory.h>
#include <Obsidian/Defines.h>
#include <string.h>

/**
 * Retrieve the length of a string, not including the null-terminating character.
 * @param str A pointer to the string.
 * @return The length of the string in bytes, not including the null-terminating character.
 */
OAPI U64 String_Length(const char* str);

/**
 * Duplicates a string.
 * @param str The string to duplicate.
 * @return A new copy of the given string. This string must later be freed with Memory_Free().
 */
OAPI char* String_Duplicate(const char* str);

/**
 * Compares the equality of two strings.
 * @param a The first string to compare.
 * @param b The second string to compare.
 * @return TRUE if the strings are equal, FALSE otherwise.
 */
OAPI B8 String_Equal(const char* a, const char* b);
