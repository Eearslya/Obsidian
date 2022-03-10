#include <Obsidian/Core/Logger.h>
#include <Obsidian/Platform/Platform.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char* LogLevel_Names[] = {"Fatal", "Error", "Warn", "Info", "Debug", "Trace"};

B8 Logger_Initialize() {
	return TRUE;
}

void Logger_Shutdown() {}

void Logger_ReportAssertion(const char* expr, const char* msg, const char* file, I32 line) {
	Logger_Output(LogLevel_Fatal, "Assertion Failed: %s\n    %s\n    at: %s:%d\n\n", expr, msg, file, line);
	fflush(stdout);
}

void Logger_Output(LogLevel level, const char* fmt, ...) {
	static char msg[16384];

	// Determine how much space the tag is going to take up.
	const I32 tagLen = snprintf(msg, sizeof(msg), "[%s] ", LogLevel_Names[level]);

	// Print out the formatted message, leaving a gap for the tag we just made.
	__builtin_va_list args;
	va_start(args, fmt);
	const I32 msgLen = vsnprintf(msg + tagLen, sizeof(msg) - tagLen, fmt, args);
	va_end(args);

	// Output our final newline at the end
	snprintf(msg + tagLen + msgLen, sizeof(msg) - tagLen - msgLen, "\r\n");

	if (level > LogLevel_Error) {
		Platform_ConsoleOut(msg);
	} else {
		Platform_ConsoleError(msg);
	}
}
