#include <Obsidian/Containers/DynArray.h>
#include <Obsidian/Core/Event.h>
#include <Obsidian/Core/Logger.h>
#include <Obsidian/Core/Memory.h>

#define Event_MaxEventCodes 16384

typedef struct EventListener {
	void* Listener;
	EventHandlerFn Handler;
	void* UserData;
} EventListener;

typedef struct EventCode {
	EventListener* Listeners;
} EventCode;

typedef struct EventSystemT {
	EventCode Codes[Event_MaxEventCodes];
} EventSystemData;

static EventSystemData EventSystem;

B8 Event_Initialize() {
	Memory_Zero(&EventSystem, sizeof(EventSystemData));

	return TRUE;
}

void Event_Shutdown() {
	for (U64 i = 0; i < Event_MaxEventCodes; ++i) {
		EventListener* listeners = EventSystem.Codes[i].Listeners;
		if (listeners) { DynArray_Destroy(listeners); }
		EventSystem.Codes[i].Listeners = NULL;
	}
}

B8 Event_Register(U16 code, void* listener, EventHandlerFn handler, void* userData) {
	if (EventSystem.Codes[code].Listeners == NULL) { EventSystem.Codes[code].Listeners = DynArray_Create(EventListener); }

	EventListener* listeners = EventSystem.Codes[code].Listeners;

	const U64 listenerCount = DynArray_Size(listeners);
	for (U64 i = 0; i < listenerCount; ++i) {
		// Duplicate registration, return failure.
		if (listeners[i].Listener == listener) { return FALSE; }
	}

	EventListener newListener = {.Listener = listener, .Handler = handler, .UserData = userData};
	DynArray_Push(listeners, newListener);

	return TRUE;
}

B8 Event_Unregister(U16 code, void* listener, EventHandlerFn handler) {
	EventListener* listeners = EventSystem.Codes[code].Listeners;

	if (listeners == NULL) { return FALSE; }

	const U64 listenerCount = DynArray_Size(listeners);
	if (listenerCount == 0) { return FALSE; }

	for (U64 i = 0; i < listenerCount; ++i) {
		if (listeners[i].Listener == listener && listeners[i].Handler == handler) {
			DynArray_Extract(listeners, i, NULL);
			return TRUE;
		}
	}

	return FALSE;
}

B8 Event_Fire(U16 code, void* sender, EventContext event) {
	EventListener* listeners = EventSystem.Codes[code].Listeners;

	if (listeners == NULL) { return FALSE; }

	const U64 listenerCount = DynArray_Size(listeners);
	for (U64 i = 0; i < listenerCount; ++i) {
		if (listeners[i].Handler(code, sender, listeners[i].Listener, event, listeners[i].UserData)) { return TRUE; }
	}

	return FALSE;
}
