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
	char* output = &msg[128];

	__builtin_va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);

	snprintf(output, sizeof(msg) - 128, "[%s] %s\r\n", LogLevel_Names[level], msg);

	if (level > LogLevel_Error) {
		Platform_ConsoleOut(output);
	} else {
		Platform_ConsoleError(output);
	}
}
