/** @file
 *  @brief Logging functions */
#pragma once

#include <Obsidian/Defines.h>

/** Indicates the severity of a log message. */
typedef enum LogLevel {
	LogLevel_Fatal =
		0, /**< Indicates a fatal error the application cannot recover from. Typically followed by an immediate exit. */
	LogLevel_Error = 1, /**< Indicates a serious problem that affects the functionality of the application. */
	LogLevel_Warn  = 2, /**< Indicates a problem that may affect the performance or usability of the application. */
	LogLevel_Info  = 3, /**< General information. */
	LogLevel_Debug = 4, /**< Debug information. Not output in release builds. */
	LogLevel_Trace = 5  /**< Verbose debug information. Not output in release builds. */
} LogLevel;

/**
 * Initialize the logger system.
 * @return TRUE upon success, FALSE on failure.
 */
B8 Logger_Initialize();

/**
 * Shutdown the logger system.
 */
void Logger_Shutdown();

/**
 * Report a failed assertion to the logs.
 * @param expr The expression that failed.
 * @param msg A message detailing why the expression should not have failed.
 * @param file The source file containing the assertion.
 * @param line The line number containing the assertion.
 */
OAPI void Logger_ReportAssertion(const char* expr, const char* msg, const char* file, I32 line);

/**
 * Output a message to the logs.
 * @param level The severity of the log message.
 * @param fmt A printf-style format string.
 * @param ... A variadic number of arguments with which to format the message.
 */
OAPI void Logger_Output(LogLevel level, const char* fmt, ...);

/**
 * Verify the given expression is TRUE. Otherwise, break and display an error message.
 * @param expr The expression to verify.
 * @param msg An error message shown when the expression is not TRUE.
 */
#define AssertMsg(expr, msg)                                  \
	do {                                                        \
		if (expr) {                                               \
		} else {                                                  \
			Logger_ReportAssertion(#expr, msg, __FILE__, __LINE__); \
			ObsDebugBreak();                                        \
		}                                                         \
	} while (0)

/**
 * Verify the given expression is TRUE. Otherwise, break.
 * @param expr The expression to verify.
 */
#define Assert(expr) AssertMsg(expr, "")

/**
 * Write a Fatal message to the logs.
 */
#define LogF(fmt, ...) Logger_Output(LogLevel_Fatal, fmt, ##__VA_ARGS__)

/**
 * Write an Error message to the logs.
 */
#define LogE(fmt, ...) Logger_Output(LogLevel_Error, fmt, ##__VA_ARGS__)

/**
 * Write a Warning message to the logs.
 */
#define LogW(fmt, ...) Logger_Output(LogLevel_Warn, fmt, ##__VA_ARGS__)

/**
 * Write an Informational message to the logs.
 */
#define LogI(fmt, ...) Logger_Output(LogLevel_Info, fmt, ##__VA_ARGS__)

#if OBSIDIAN_DEBUG == 1
/**
 * Write a Debug message to the logs.
 */
#	define LogD(fmt, ...) Logger_Output(LogLevel_Debug, fmt, ##__VA_ARGS__)

/**
 * Write a Trace message to the logs.
 */
#	define LogT(fmt, ...) Logger_Output(LogLevel_Trace, fmt, ##__VA_ARGS__)

/**
 * Used to perform an AssertMsg() only in debug mode.
 */
#	define DebugAssertMsg(expr, msg) AssertMsg(expr, msg)

/**
 * Used to perform an Assert() only in debug mode.
 */
#	define DebugAssert(expr) Assert(expr)
#else
#	define LogD(fmt, ...)
#	define LogT(fmt, ...)
#	define DebugAssertMsg(expr, msg)
#	define DebugAssert(expr)
#endif
