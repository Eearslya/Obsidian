#include <Obsidian/Core/Event.h>
#include <Obsidian/Core/Input.h>
#include <Obsidian/Core/Memory.h>

typedef struct KeyboardStateT {
	B8 Keys[256];
} KeyboardState;

typedef struct MouseStateT {
	I16 X;
	I16 Y;
	B8 Buttons[MouseButton_Count];
} MouseState;

typedef struct InputStateT {
	KeyboardState Keyboard;
	KeyboardState LastKeyboard;
	MouseState Mouse;
	MouseState LastMouse;
} InputState;

static InputState Input = {};

B8 Input_Initialize() {
	Memory_Zero(&Input, sizeof(InputState));

	return TRUE;
}

void Input_Shutdown() {}

void Input_ProcessMouseButton(MouseButton btn, B8 press) {
	if (Input.Mouse.Buttons[btn] != press) {
		Input.Mouse.Buttons[btn] = press;

		EventContext evt = {};
		evt.Data.U16[0]  = btn;
		Event_Fire(press ? EventCode_MouseButtonPressed : EventCode_MouseButtonReleased, NULL, evt);
	}
}

void Input_ProcessMouseMove(I16 x, I16 y) {
	if (Input.Mouse.X != x || Input.Mouse.Y != y) {
		Input.Mouse.X = x;
		Input.Mouse.Y = y;

		EventContext evt = {};
		evt.Data.I16[0]  = x;
		evt.Data.I16[1]  = y;
		Event_Fire(EventCode_MouseMoved, NULL, evt);
	}
}

void Input_ProcessScroll(I8 zDelta) {
	EventContext evt = {};
	evt.Data.I8[0]   = zDelta;
	Event_Fire(EventCode_MouseScrolled, NULL, evt);
}

void Input_ProcessKey(Key key, B8 press) {
	if (Input.Keyboard.Keys[key] != press) {
		Input.Keyboard.Keys[key] = press;

		EventContext onKey = {};
		onKey.Data.U16[0]  = key;
		Event_Fire(press ? EventCode_KeyPressed : EventCode_KeyReleased, NULL, onKey);
	}
}

void Input_Update(F64 deltaTime) {
	Memory_Copy(&Input.LastKeyboard, &Input.Keyboard, sizeof(KeyboardState));
	Memory_Copy(&Input.LastMouse, &Input.Mouse, sizeof(MouseState));
}

B8 Input_IsKeyDown(Key key) {
	return Input.Keyboard.Keys[key];
}

B8 Input_IsKeyUp(Key key) {
	return Input.Keyboard.Keys[key] == FALSE;
}

B8 Input_WasKeyDown(Key key) {
	return Input.LastKeyboard.Keys[key];
}

B8 Input_WasKeyUp(Key key) {
	return Input.LastKeyboard.Keys[key] == FALSE;
}

void Input_GetMousePosition(I32* x, I32* y) {
	*x = Input.Mouse.X;
	*y = Input.Mouse.Y;
}

void Input_GetLastMousePosition(I32* x, I32* y) {
	*x = Input.LastMouse.X;
	*y = Input.LastMouse.Y;
}

B8 Input_IsMouseButtonDown(MouseButton btn) {
	return Input.Mouse.Buttons[btn];
}

B8 Input_IsMouseButtonUp(MouseButton btn) {
	return Input.Mouse.Buttons[btn] == FALSE;
}

B8 Input_WasMouseButtonDown(MouseButton btn) {
	return Input.LastMouse.Buttons[btn];
}

B8 Input_WasMouseButtonUp(MouseButton btn) {
	return Input.LastMouse.Buttons[btn] == FALSE;
}
