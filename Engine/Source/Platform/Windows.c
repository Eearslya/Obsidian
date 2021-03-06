#include <Obsidian/Platform/Platform.h>

#if OBSIDIAN_WINDOWS == 1
#	include <Obsidian/Core/Logger.h>
#	include <Obsidian/Core/Input.h>
#	include <Obsidian/Core/Event.h>
#	include <Obsidian/Renderer/Vulkan/Common.h>
#	include <Obsidian/Renderer/Vulkan/VulkanPlatform.h>
#	include <Obsidian/Containers/DynArray.h>
#	include <stdlib.h>
#	include <stdio.h>
#	define NOMINMAX
#	define WIN32_LEAN_AND_LEAN
#	include <malloc.h>
#	include <Windows.h>
#	include <WindowsX.h>
#	include <vulkan/vulkan_win32.h>

struct PlatformStateT {
	HINSTANCE Instance;
	HWND Window;
	U32 FramebufferW;
	U32 FramebufferH;
	U32 WindowW;
	U32 WindowH;
};

static const char* WndClassName = "ObsidianWndClass";
static F64 ClockFrequency       = 0.0;
static LARGE_INTEGER ClockStartTime;

static LRESULT CALLBACK HandleMessage(HWND hwnd, U32 msg, WPARAM wParam, LPARAM lParam);

B8 Platform_Initialize(PlatformState* state, const char* appName, I32 windowX, I32 windowY, I32 windowW, I32 windowH) {
	// Allocate our state object.
	*state = malloc(sizeof(struct PlatformStateT));
	memset(*state, 0, sizeof(struct PlatformStateT));
	(*state)->FramebufferW = windowW;
	(*state)->FramebufferH = windowH;

	// Find our Win32 instance.
	(*state)->Instance = GetModuleHandleA(NULL);

	// Initialize our clock.
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	ClockFrequency = 1.0 / (F64) frequency.QuadPart;
	QueryPerformanceCounter(&ClockStartTime);

	// Register our main window class.
	HICON icon           = LoadIconA((*state)->Instance, IDI_APPLICATION);
	WNDCLASSEXA wndClass = {.cbSize        = sizeof(WNDCLASSEXA),
	                        .style         = 0,
	                        .lpfnWndProc   = HandleMessage,
	                        .cbClsExtra    = 0,
	                        .cbWndExtra    = 0,
	                        .hInstance     = (*state)->Instance,
	                        .hIcon         = icon,
	                        .hCursor       = NULL,
	                        .hbrBackground = NULL,
	                        .lpszMenuName  = NULL,
	                        .lpszClassName = WndClassName,
	                        .hIconSm       = NULL};
	if (!RegisterClassExA(&wndClass)) {
		MessageBoxA(NULL, "Failed to register window class!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	LogT("[Platform] Window class registered.");

	// Determine the size of our initial window.
	const DWORD windowStyle   = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
	const DWORD windowStyleEx = WS_EX_APPWINDOW;
	RECT borderRect           = {0, 0, 0, 0};
	AdjustWindowRectEx(&borderRect, windowStyle, FALSE, windowStyleEx);
	windowW += borderRect.right - borderRect.left;
	windowH += borderRect.bottom - borderRect.top;

	// Center the window position, if applicable.
	if (windowX == -1) {
		const int screenW = GetSystemMetrics(SM_CXSCREEN);
		windowX           = (screenW - windowW) / 2;
	} else {
		windowX += borderRect.left;
	}
	if (windowY == -1) {
		const int screenH = GetSystemMetrics(SM_CYSCREEN);
		windowY           = (screenH - windowH) / 2;
	} else {
		windowY += borderRect.top;
	}
	(*state)->WindowW = windowW;
	(*state)->WindowH = windowH;

	// Create our initial window.
	(*state)->Window = CreateWindowExA(windowStyleEx,
	                                   WndClassName,
	                                   appName,
	                                   windowStyle,
	                                   windowX,
	                                   windowY,
	                                   windowW,
	                                   windowH,
	                                   NULL,
	                                   NULL,
	                                   (*state)->Instance,
	                                   NULL);
	if ((*state)->Window == NULL) {
		MessageBoxA(NULL, "Failed to create initial window!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	// Show the window.
	ShowWindow((*state)->Window, SW_SHOW);

	return TRUE;
}

void Platform_Shutdown(PlatformState state) {
	if (state->Window) {
		DestroyWindow(state->Window);
		state->Window = NULL;
	}

	Logger_Shutdown();
}

B8 Platform_Update(PlatformState state) {
	MSG message;
	while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			EventContext quit = {};
			Event_Fire(EventCode_ApplicationQuit, state, quit);
			return FALSE;
		}

		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	return TRUE;
}

void Platform_GetFramebufferSize(PlatformState state, U32* width, U32* height) {
	*width  = state->FramebufferW;
	*height = state->FramebufferH;
}

void* Platform_Alloc(size_t bytes) {
	return malloc(bytes);
}

void* Platform_AllocAligned(size_t bytes, U8 align) {
	return _aligned_malloc(bytes, align);
}

void* Platform_Realloc(void* ptr, size_t bytes) {
	if (ptr == NULL) { return Platform_Alloc(bytes); }

	return realloc(ptr, bytes);
}

void* Platform_ReallocAligned(void* ptr, U8 align, size_t bytes) {
	if (ptr == NULL) { return Platform_AllocAligned(bytes, align); }

	return _aligned_realloc(ptr, bytes, align);
}

void Platform_Free(void* ptr) {
	free(ptr);
}

void Platform_FreeAligned(void* ptr) {
	_aligned_free(ptr);
}

void Platform_MemCopy(void* dst, const void* src, size_t bytes) {
	memcpy(dst, src, bytes);
}

void Platform_MemMove(void* dst, const void* src, size_t bytes) {
	memmove(dst, src, bytes);
}

void Platform_MemSet(void* ptr, U8 value, size_t bytes) {
	memset(ptr, value, bytes);
}

void Platform_MemZero(void* ptr, size_t bytes) {
	Platform_MemSet(ptr, 0, bytes);
}

void Platform_ConsoleOut(const char* msg) {
#	if OBSIDIAN_DEBUG == 1
	OutputDebugStringA(msg);
#	endif
	fprintf(stdout, "%s", msg);
	fflush(stdout);  // TODO: Find a better solution for log flushing.
}

void Platform_ConsoleError(const char* msg) {
#	if OBSIDIAN_DEBUG == 1
	OutputDebugStringA(msg);
#	endif
	fprintf(stderr, "%s", msg);
	fflush(stderr);
}

F64 Platform_GetAbsoluteTime() {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (F64) now.QuadPart * ClockFrequency;
}

void Platform_Sleep(U64 ms) {
	Sleep(ms);
}

static LRESULT CALLBACK HandleMessage(HWND hwnd, U32 msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_MOUSEMOVE: {
			const I32 x = GET_X_LPARAM(lParam);
			const I32 y = GET_Y_LPARAM(lParam);
			Input_ProcessMouseMove(x, y);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP: {
			const B8 press  = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
			MouseButton btn = MouseButton_Count;
			switch (msg) {
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
					btn = MouseButton_Left;
					break;
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
					btn = MouseButton_Middle;
					break;
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
					btn = MouseButton_Right;
					break;
			}
			if (btn != MouseButton_Count) { Input_ProcessMouseButton(btn, press); }
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP: {
			const B8 press = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
			const Key key  = (U16) wParam;
			Input_ProcessKey(key, press);
			break;
		}
		case WM_MOUSEWHEEL: {
			const I8 delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			Input_ProcessScroll(delta);
			break;
		}
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_ERASEBKGND:
			return 1;
	}
	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void Platform_Vulkan_GetRequiredInstanceExtensions(DynArrayT extensionNames) {
	DynArray_PushValue(extensionNames, &"VK_KHR_surface");
	DynArray_PushValue(extensionNames, &"VK_KHR_win32_surface");
}

B8 Platform_Vulkan_CreateSurface(struct PlatformStateT* platform, struct VulkanContextT* context) {
	PFN_vkCreateWin32SurfaceKHR fn =
		(PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr(context->Instance, "vkCreateWin32SurfaceKHR");
	if (!fn) { return FALSE; }

	const VkWin32SurfaceCreateInfoKHR surfaceCI = {.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
	                                               .pNext     = NULL,
	                                               .flags     = 0,
	                                               .hinstance = platform->Instance,
	                                               .hwnd      = platform->Window};
	VkSurfaceKHR surface                        = VK_NULL_HANDLE;
	const VkResult createResult                 = fn(context->Instance, &surfaceCI, &context->Allocator, &surface);
	if (createResult != VK_SUCCESS) { return FALSE; }

	context->Surface = surface;

	return TRUE;
}
#endif
