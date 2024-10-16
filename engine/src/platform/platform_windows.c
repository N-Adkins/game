#include "platform.h"

#ifdef LPLATFORM_WINDOWS

#include <core/assert.h>
#include <core/logger.h>

#include <windows.h>

// Not sure how important this name is honestly.
const char class_name[] = "AppClass";

struct windows_platform_state {
    HINSTANCE instance; // Windows process instance
    HWND window; // Windows window handle
};

// TODO: Use this properly
LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) 
{
    DefWindowProc(window, msg, w_param, l_param);
    return 0;
}

void platform_startup(
    struct platform_state *state,
    const char *app_name,
    i32 start_x,
    i32 start_y,
    i32 start_width,
    i32 start_height
)
{
    LINFO("Initializing the Windows platform state");

    struct windows_platform_state *windows_state = malloc(sizeof(struct windows_platform_state));
    state->inner_state = windows_state;
    
    windows_state->instance = GetModuleHandle(0);
    if (windows_state->instance == NULL) {
        LFATAL("Failed to fetch Windows instance");
        return;
    }

    WNDCLASSEX win_info;
    win_info.cbSize = sizeof(WNDCLASSEX);
    win_info.style = CS_HREDRAW | CS_VREDRAW;
    win_info.lpfnWndProc = DefWindowProc;
    win_info.cbClsExtra = 0;
    win_info.cbWndExtra = 0;
    win_info.hInstance = windows_state->instance;
    win_info.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon
    win_info.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // Default icon
    win_info.hCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
    win_info.hbrBackground = CreateSolidBrush(RGB(0, 0, 0)); // Black background
    win_info.lpszMenuName = NULL;
    win_info.lpszClassName = class_name;

    if (!RegisterClassEx(&win_info)) {
        LFATAL("Failed to register window info for Windows, Error code: %lu", GetLastError());
        return;
    }

    windows_state->window = CreateWindowEx(
        0,
        class_name,
        app_name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        start_width,
        start_height,
        NULL,
        NULL,
        windows_state->instance,
        NULL
    );

    if (windows_state->window == NULL) {
        LFATAL("Failed to create window on Windows Error code: %lu", GetLastError());
        return;
    }

    ShowWindow(windows_state->window, SW_NORMAL);
    UpdateWindow(windows_state->window);
}

void platform_shutdown(struct platform_state *state)
{
    LINFO("Shutting down the Windows platform state");

    struct windows_platform_state *windows_state = state->inner_state;
    DestroyWindow(windows_state->window);
    
    free(state->inner_state);
}

b8 platform_poll_events(struct platform_state *state)
{ 
    struct windows_platform_state *windows_state = state->inner_state;
    (void)windows_state;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void platform_print_color(
    FILE *file,
    const char *string,
    enum terminal_color color
)
{
    // TODO: Color
    (void)color;
    fprintf(file, "%s", string);
}

void platform_debug_break(void)
{
#if defined (LCOMPILER_GCC)
    __builtin_trap();
#elif defined (LCOMPILER_CLANG)
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
    return memset(ptr, 0, size);
}

void *platform_copy_memory(void *restrict dest, const void *restrict source, u64 size)
{
    return memcpy(dest, source, size);
}

void *platform_set_memory(void *dest, i32 value, u64 size)
{
    return memset(dest, value, size); 
}

#endif
