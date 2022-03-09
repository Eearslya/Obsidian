#pragma once

#include <Obsidian/Defines.h>

typedef struct EventContextT {
	union {
		I64 I64[2];
		U64 U64[2];
		F64 F64[2];

		I32 I32[4];
		U32 U32[4];
		F32 F32[4];
		B32 B32[4];

		I16 I16[8];
		U16 U16[8];

		I8 I8[16];
		U8 U8[16];

		char C[16];
	} Data;
} EventContext;

enum {
	EventCode_ApplicationQuit = 0x1, /**< Application shutdown is requested. Return TRUE to cancel. */
	EventCode_Resized         = 0x2  /**< Application window has been resized. */
};

typedef B8 (*EventHandlerFn)(U16 code, void* sender, void* listener, EventContext event);

B8 Event_Initialize();
void Event_Shutdown();

OAPI B8 Event_Register(U16 code, void* listener, EventHandlerFn handler);
OAPI B8 Event_Unregister(U16 code, void* listener, EventHandlerFn handler);
OAPI B8 Event_Fire(U16 code, void* sender, EventContext event);
