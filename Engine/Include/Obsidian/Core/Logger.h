#pragma once

#include <Obsidian/Defines.h>

typedef enum LogLevel {
	LogLevel_Fatal = 0,
	LogLevel_Error = 1,
	LogLevel_Warn  = 2,
	LogLevel_Info  = 3,
	LogLevel_Debug = 4,
	LogLevel_Trace = 5
} LogLevel;

B8 Logger_Initialize();
void Logger_Shutdown();

OAPI void Logger_ReportAssertion(const char* expr, const char* msg, const char* file, I32 line);
OAPI void Logger_Output(LogLevel level, const char* fmt, ...);

#define AssertMsg(expr, msg)                                  \
	do {                                                        \
		if (expr) {                                               \
		} else {                                                  \
			Logger_ReportAssertion(#expr, msg, __FILE__, __LINE__); \
			ObsDebugBreak();                                        \
		}                                                         \
	} while (0)
#define Assert(expr) AssertMsg(expr, "")

#define LogF(fmt, ...) Logger_Output(LogLevel_Fatal, fmt, ##__VA_ARGS__)
#define LogE(fmt, ...) Logger_Output(LogLevel_Error, fmt, ##__VA_ARGS__)
#define LogW(fmt, ...) Logger_Output(LogLevel_Warn, fmt, ##__VA_ARGS__)
#define LogI(fmt, ...) Logger_Output(LogLevel_Info, fmt, ##__VA_ARGS__)

#if OBSIDIAN_DEBUG == 1
#	define LogD(fmt, ...)            Logger_Output(LogLevel_Debug, fmt, ##__VA_ARGS__)
#	define LogT(fmt, ...)            Logger_Output(LogLevel_Trace, fmt, ##__VA_ARGS__)
#	define DebugAssertMsg(expr, msg) AssertMsg(expr, msg)
#	define DebugAssert(expr)         Assert(expr)
#else
#	define LogD(fmt, ...)
#	define LogT(fmt, ...)
#	define DebugAssertMsg(expr, msg)
#	define DebugAssert(expr)
#endif
