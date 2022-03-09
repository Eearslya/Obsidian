#pragma once

#include <Obsidian/Defines.h>

/** Enumeration for mouse buttons. */
typedef enum MouseButton { MouseButton_Left, MouseButton_Middle, MouseButton_Right, MouseButton_Count } MouseButton;

/** Enumeration for keyboard keys. */
typedef enum Key {
	Key_Backspace = 0x08,
	Key_Tab       = 0x09,
	Key_Enter     = 0x0D,

	Key_Shift    = 0x10,
	Key_Control  = 0x11,
	Key_Pause    = 0x13,
	Key_CapsLock = 0x14,
	Key_Escape   = 0x1B,

	Key_Space       = 0x20,
	Key_PageUp      = 0x21,
	Key_PageDown    = 0x22,
	Key_End         = 0x23,
	Key_Home        = 0x24,
	Key_Left        = 0x25,
	Key_Up          = 0x26,
	Key_Right       = 0x27,
	Key_Down        = 0x28,
	Key_PrintScreen = 0x2C,
	Key_Insert      = 0x2D,
	Key_Delete      = 0x2E,

	Key_LeftSuper  = 0x5B,
	Key_RightSuper = 0x5C,

	Key_Numpad0        = 0x60,
	Key_Numpad1        = 0x61,
	Key_Numpad2        = 0x62,
	Key_Numpad3        = 0x63,
	Key_Numpad4        = 0x64,
	Key_Numpad5        = 0x65,
	Key_Numpad6        = 0x66,
	Key_Numpad7        = 0x67,
	Key_Numpad8        = 0x68,
	Key_Numpad9        = 0x69,
	Key_NumpadMultiply = 0x6A,
	Key_NumpadAdd      = 0x6B,
	Key_NumpadSubtract = 0x6D,
	Key_NumpadDecimal  = 0x6E,
	Key_NumpadDivide   = 0x6F,

	Key_F1  = 0x70,
	Key_F2  = 0x71,
	Key_F3  = 0x72,
	Key_F4  = 0x73,
	Key_F5  = 0x74,
	Key_F6  = 0x75,
	Key_F7  = 0x76,
	Key_F8  = 0x77,
	Key_F9  = 0x78,
	Key_F10 = 0x79,
	Key_F11 = 0x7A,
	Key_F12 = 0x7B,

	Key_NumLock    = 0x90,
	Key_ScrollLock = 0x91,

	Key_LeftShift    = 0xA0,
	Key_RightShift   = 0xA1,
	Key_LeftControl  = 0xA2,
	Key_RightControl = 0xA3,
	Key_LeftAlt      = 0xA4,
	Key_RightAlt     = 0xA5,

	Key_Semicolon = 0xBA,
	Key_Slash     = 0xBF,

	Key_Grave = 0xC0,

	Key_LeftBracket  = 0xDB,
	Key_Backslash    = 0xDC,
	Key_RightBracket = 0xDD,
	Key_Apostrophe   = 0xDE
} Key;

enum {
	EventCode_KeyPressed          = 0x10, /**< A keyboard key has just been pressed. */
	EventCode_KeyReleased         = 0x11, /**< A keyboard key has just been released. */
	EventCode_MouseButtonPressed  = 0x12, /**< A mouse button has just been pressed. */
	EventCode_MouseButtonReleased = 0x13, /**< A mouse button has just been released. */
	EventCode_MouseMoved          = 0x14, /**< The mouse cursor has moved. */
	EventCode_MouseScrolled       = 0x15  /**< The mouse scroll wheel has moved. */
};

/**
 * Initialize the Input subsystem.
 * @return TRUE on success, FALSE on failure.
 */
B8 Input_Initialize();

/**
 * Shutdown the Input subsystem.
 */
void Input_Shutdown();

/**
 * Process an incoming mouse button event.
 * @param btn The mouse button related to the event.
 * @param press Whether the mouse button was pressed.
 */
void Input_ProcessMouseButton(MouseButton btn, B8 press);

/**
 * Process an incoming mouse move event.
 * @param x The x position of the mouse.
 * @param y The y position of the mouse.
 */
void Input_ProcessMouseMove(I16 x, I16 y);

/**
 * Process an incoming mouse scroll event.
 * @param zDelta The number of "lines" scrolled.
 */
void Input_ProcessScroll(I8 zDelta);

/**
 * Process an incoming keyboard key event.
 * @param key The keyboard key related to the event.
 * @param press Whether the keyboard key was pressed.
 */
void Input_ProcessKey(Key key, B8 press);

/**
 * Update the input subsystem. Should be called once per update loop.
 * @param deltaTime The amount of time, in seconds, since the last update.
 */
void Input_Update(F64 deltaTime);

/**
 * Returns whether the specified key is down.
 * @param key The key to check.
 * @return TRUE if the key is down, FALSE if the key is up.
 */
OAPI B8 Input_IsKeyDown(Key key);

/**
 * Returns whether the specified key is up.
 * @param key The key to check.
 * @return TRUE if the key is up, FALSE if the key is down.
 */
OAPI B8 Input_IsKeyUp(Key key);

/**
 * Returns whether the specified key was down in the last update.
 * @param key The key to check.
 * @return TRUE if the key was down, FALSE if the key was up.
 */
OAPI B8 Input_WasKeyDown(Key key);

/**
 * Returns whether the specified key was up in the last update.
 * @param key The key to check.
 * @return TRUE if the key was up, FALSE if the key was down.
 */
OAPI B8 Input_WasKeyUp(Key key);

/**
 * Retrieve the current position of the mouse cursor.
 * @param[out] x The mouse's X position.
 * @param[out] y The mouse's Y position.
 */
OAPI void Input_GetMousePosition(I32* x, I32* y);

/**
 * Retrieve the position of the mouse cursor as of the last update.
 * @param[out] x The mouse's X position.
 * @param[out] y The mouse's Y position.
 */
OAPI void Input_GetLastMousePosition(I32* x, I32* y);

/**
 * Returns whether the specified mouse button is down.
 * @param btn The button to check.
 * @return TRUE if the button is down, FALSE if the button is up.
 */
OAPI B8 Input_IsMouseButtonDown(MouseButton btn);

/**
 * Returns whether the specified mouse button is up.
 * @param btn The button to check.
 * @return TRUE if the button is up, FALSE if the button is down.
 */
OAPI B8 Input_IsMouseButtonUp(MouseButton btn);

/**
 * Returns whether the specified mouse button was down in the last update.
 * @param btn The button to check.
 * @return TRUE if the button was down, FALSE if the button was up.
 */
OAPI B8 Input_WasMouseButtonDown(MouseButton btn);

/**
 * Returns whether the specified mouse button was up in the last update.
 * @param btn The button to check.
 * @return TRUE if the button was up, FALSE if the button was down.
 */
OAPI B8 Input_WasMouseButtonUp(MouseButton btn);
