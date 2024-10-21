#include "platform.h"

#ifdef LPLATFORM_WINDOWS

#include <core/assert.h>
#include <core/logger.h>

#include <windows.h>

// TODO: Events

// Not sure how important this name is honestly.
const char class_name[] = "AppClass";

struct platform_impl {
	HINSTANCE instance; // Windows process instance
	HWND window; // Windows window handle
};

struct mutex_impl {
	HANDLE mutex;
};

// TODO: Use this properly
LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM w_param,
				  LPARAM l_param)
{
	DefWindowProc(window, msg, w_param, l_param);
	return 0;
}

void platform_startup(struct platform *platform,
		      struct event_system *event_system, const char *app_name,
		      i32 start_x, i32 start_y, i32 start_width,
		      i32 start_height)
{
	LASSERT(platform != NULL);
	LASSERT(app_name != NULL);

	(void)event_system;

	LINFO("Initializing the Windows platform state");

	struct platform_impl *windows_impl =
		malloc(sizeof(struct platform_impl));
	platform->impl = windows_impl;

	windows_impl->instance = GetModuleHandle(0);
	if (windows_impl->instance == NULL) {
		LFATAL("Failed to fetch Windows instance");
		return;
	}

	WNDCLASSEX win_info;
	win_info.cbSize = sizeof(WNDCLASSEX);
	win_info.style = CS_HREDRAW | CS_VREDRAW;
	win_info.lpfnWndProc = DefWindowProc;
	win_info.cbClsExtra = 0;
	win_info.cbWndExtra = 0;
	win_info.hInstance = windows_impl->instance;
	win_info.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon
	win_info.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // Default icon
	win_info.hCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
	win_info.hbrBackground =
		CreateSolidBrush(RGB(0, 0, 0)); // Black background
	win_info.lpszMenuName = NULL;
	win_info.lpszClassName = class_name;

	if (!RegisterClassEx(&win_info)) {
		LFATAL("Failed to register window info for Windows, Error code: %lu",
		       GetLastError());
		return;
	}

	windows_impl->window =
		CreateWindowEx(0, class_name, app_name, WS_OVERLAPPEDWINDOW,
			       start_x, start_y, start_width, start_height,
			       NULL, NULL, windows_impl->instance, NULL);

	if (windows_impl->window == NULL) {
		LFATAL("Failed to create window on Windows Error code: %lu",
		       GetLastError());
		return;
	}

	ShowWindow(windows_impl->window, SW_NORMAL);
	UpdateWindow(windows_impl->window);
}

void platform_shutdown(struct platform platform)
{
	LASSERT(platform.impl != NULL);

	LINFO("Shutting down the Windows platform state");

	DestroyWindow(platform.impl->window);

	free(platform.impl);
}

b8 platform_poll_events(struct platform platform)
{
	LASSERT(platform.impl != NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void platform_print_color(FILE *file, const char *string,
			  enum terminal_color color)
{
	LASSERT(file != NULL);
	LASSERT(string != NULL);

	// TODO: Color
	(void)color;
	(void)fprintf(file, "%s", string);
}

LAPI void platform_debug_break(void)
{
#if defined(LCOMPILER_GCC)
	__builtin_trap();
#elif defined(LCOMPILER_CLANG)
	__builtin_debugtrap();
#else // MSVC
	__debugbreak();
#endif
}

// These are just libc wrappers for Windows.

void *platform_allocate(u64 size, b8 aligned)
{
	(void)aligned; // irrelevant, always aligned
	return malloc(size);
}

void platform_free(void *ptr, b8 aligned)
{
	(void)aligned; // irrelevant, this memory is malloced so always aligned
	free(ptr);
}

void *platform_zero_memory(void *ptr, u64 size)
{
	LASSERT(ptr != NULL);

	return memset(ptr, 0, size);
}

void *platform_copy_memory(void *restrict dest, const void *restrict source,
			   u64 size)
{
	LASSERT(dest != NULL);
	LASSERT(source != NULL);

	return memcpy(dest, source, size);
}

void *platform_move_memory(void *dest, const void *source, u64 size)
{
	LASSERT(dest != NULL);
	LASSERT(source != NULL);

	return memmove(dest, source, size);
}

void *platform_set_memory(void *dest, u8 value, u64 size)
{
	LASSERT(dest != NULL);

	return memset(dest, value, size);
}

struct mutex mutex_create(void)
{
	struct mutex mutex;
	mutex.impl = NULL;

	HANDLE mutex_handle = CreateMutex(NULL, false, NULL);
	if (mutex_handle == NULL) {
		LERROR("Failed to create Windows mutex: %lu", GetLastError());
		return mutex;
	}

	struct mutex_impl *windows_impl =
		platform_allocate(sizeof(struct mutex_impl), true);
	windows_impl->mutex = mutex_handle;
	mutex.impl = windows_impl;

	return mutex;
}

void mutex_destroy(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	b8 result = CloseHandle(mutex.impl->mutex);
	if (!result) {
		LERROR("Failed to destroy Windows mutex: %lu", GetLastError());
	}

	platform_free(mutex.impl, true);
}

void mutex_lock(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	const DWORD timeout = INFINITE;
	const DWORD result = WaitForSingleObject(mutex.impl->mutex, timeout);
	switch (result) {
	case WAIT_ABANDONED:
		LERROR("The thread that owned this Windows mutex has terminated");
		break;
	case WAIT_TIMEOUT:
		LERROR("Windows mutex lock has timed out! Amount of time was %lu",
		       timeout);
		break;
	case WAIT_FAILED:
		LERROR("Failed to lock Windows mutex: %lu", GetLastError());
		break;
	default: // success
		break;
	}
}

void mutex_unlock(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	const b8 result = ReleaseMutex(mutex.impl->mutex);
	if (!result) {
		LERROR("Failed to release Windows mutex: %lu", GetLastError());
		return;
	}
}

#endif
