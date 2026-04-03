#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "renderer.h"

#define WIDTH 800
#define HEIGHT 600

static bool running = true;
static Buffer *back_buffer = NULL;
static bool use_gpu = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            if (width > 0 && height > 0) {
                if (back_buffer) destroy_buffer(back_buffer);
                back_buffer = create_buffer(width, height);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (back_buffer) {
                BITMAPINFO bmi = {0};
                bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
                bmi.bmiHeader.biWidth = back_buffer->width;
                bmi.bmiHeader.biHeight = -back_buffer->height; // Top-down
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                RECT client_rect;
                GetClientRect(hwnd, &client_rect);

                StretchDIBits(hdc, 0, 0, client_rect.right, client_rect.bottom,
                              0, 0, back_buffer->width, back_buffer->height,
                              back_buffer->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    (void)hPrevInstance;
    
    if (lpCmdLine && strstr(lpCmdLine, "-g")) {
        use_gpu = true;
        MessageBox(NULL, "Using GPU Accelerated Pipeline (Xe2)", "Renderer Info", MB_OK);
    }
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "SubpixelRendererClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Add cursor for resizing feedback

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, "Subpixel Renderer",
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);

    if (!hwnd) return -1;

    ShowWindow(hwnd, nShowCmd);

    // Initial buffer creation will be handled by the first WM_SIZE message
    // but we can ensure it exists here too if needed, though WM_SIZE is sent on creation.

    Color bg = {20, 20, 25, 255};
    Color white = {255, 255, 255, 255};
    Color red = {0, 0, 255, 255};

    float angle = 0.0f;

    LARGE_INTEGER frequency, last_time, current_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last_time);
    
    float fps = 0.0f;
    char fps_text[32];

    while (running) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        QueryPerformanceCounter(&current_time);
        float delta_time = (float)(current_time.QuadPart - last_time.QuadPart) / frequency.QuadPart;
        last_time = current_time;
        
        // Smooth FPS calculation
        if (delta_time > 0) {
            fps = 0.9f * fps + 0.1f * (1.0f / delta_time);
        }

        if (back_buffer) {
            clear_buffer(back_buffer, bg);

            float cx = back_buffer->width / 2.0f;
            float cy = back_buffer->height / 2.0f;

            // Demo drawing
            draw_line_aa(back_buffer, 0.125f * back_buffer->width, 0.166f * back_buffer->height, 
                         0.875f * back_buffer->width, 0.833f * back_buffer->height, white);
            
            draw_circle_aa(back_buffer, cx, cy, 150 + 50 * (float)sin(angle), red);
            
            for(int i=0; i<12; i++) {
                float a = angle + i * 3.14159f / 6.0f;
                draw_line_aa(back_buffer, cx, cy, cx + cosf(a) * 100, cy + sinf(a) * 100, white);
            }

            // Render FPS top-right
            sprintf(fps_text, "FPS: %.2f", fps);
            draw_text(back_buffer, back_buffer->width - 80, 10, fps_text, white);

            angle += 1.0f * delta_time; // Frame-rate independent animation

            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
    }

    destroy_buffer(back_buffer);
    return 0;
}
