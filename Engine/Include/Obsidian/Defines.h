#pragma once

// Integer typedefs
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;
typedef char I8;
typedef short I16;
typedef int I32;
typedef long long I64;
typedef float F32;
typedef double F64;
typedef int B32;
typedef char B8;

// Ensure we have a static assert depending on compiler
#if defined(__clang__) || defined(__gcc__)
#	define STATIC_ASSERT _Static_assert
#else
#	define STATIC_ASSERT static_assert
#endif

// Verify all integer sizes are what we expect
STATIC_ASSERT(sizeof(U8) == 1, "Expected U8 to be 1 byte.");
STATIC_ASSERT(sizeof(U16) == 2, "Expected U16 to be 2 bytes.");
STATIC_ASSERT(sizeof(U32) == 4, "Expected U32 to be 4 bytes.");
STATIC_ASSERT(sizeof(U64) == 8, "Expected U64 to be 8 bytes.");
STATIC_ASSERT(sizeof(I8) == 1, "Expected I8 to be 1 byte.");
STATIC_ASSERT(sizeof(I16) == 2, "Expected I16 to be 2 bytes.");
STATIC_ASSERT(sizeof(I32) == 4, "Expected I32 to be 4 bytes.");
STATIC_ASSERT(sizeof(I64) == 8, "Expected I64 to be 8 bytes.");
STATIC_ASSERT(sizeof(F32) == 4, "Expected F32 to be 4 bytes.");
STATIC_ASSERT(sizeof(F64) == 8, "Expected F64 to be 8 bytes.");

// Boolean values
enum { FALSE = 0, TRUE = 1 };

// Platform detection
#if defined(_WIN64)
#	define OBSIDIAN_WINDOWS 1
#elif defined(__linux__) || defined(__gnu_linux__)
#	error "Linux support is not yet available!"
#	define OBSIDIAN_LINUX 1
#	if defined(__ANDROID__)
#		define OBSIDIAN_ANDROID 1
#	endif
#elif defined(__unix__)
#	error "Unix support is not yet available!"
#	define OBSIDIAN_UNIX 1
#elif defined(_POSIX_VERSION)
#	error "Posix suport is not yet available!"
#	define OBSIDIAN_POSIX 1
#elif defined(__APPLE__)
#	error "Apple support is not yet available!"
#	define OBSIDIAN_APPLE 1
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR
#		define OBSIDIAN_IOS           1
#		define OBSIDIAN_IOS_SIMULATOR 1
#	elif TARGET_OS_IPHONE
#		define OBSIDIAN_IOS 1
#	elif TARGET_OS_MAC
#		define OBSIDIAN_MAC 1
#	else
#		error "Unknown Apple platform!"
#	endif
#endif

// Debug
#if !defined(NDEBUG) && !defined(OBSIDIAN_DEBUG)
#	define OBSIDIAN_DEBUG 1
#	ifdef _MSC_VER
#		include <intrin.h>
#		define DebugBreak() __debugbreak()
#	else
#		define DebugBreak() __builtin_trap()
#	endif
#else
#	define DebugBreak() abort()
#endif

// Shared library exports/imports
#if defined(OBSIDIAN_BUILD)
#	ifdef _MSC_VER
#		define OAPI __declspec(dllexport)
#	else
#		define OAPI __attribute__((visibility("default")))
#	endif
#else
#	ifdef _MSC_VER
#		define OAPI __declspec(dllimport)
#	else
#		define OAPI
#	endif
#endif
