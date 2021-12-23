#include "utils.cpp"
#include <windows.h>
#include <time.h>


global_variable bool running = true;


struct Render_State {
	int height, width;
	void* memory;

	BITMAPINFO bitmap_info;
};
global_variable Render_State render_state;
global_variable float common_constant = 1;
global_variable float window_start_width = 720.f * common_constant;
global_variable float window_start_height = 720.f * common_constant;

#include "renderer.cpp"
#include "sim.cpp"


LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (uMsg) {
		case WM_CLOSE: 
		case WM_DESTROY: {
			running = false;
		} break;
		
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);
			render_state.width = rect.right - rect.left;
			render_state.height = rect.bottom - rect.top;
			
			int size = render_state.width * render_state.height * sizeof(unsigned int);

			if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
			render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
			render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
			render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
			render_state.bitmap_info.bmiHeader.biPlanes = 1;
			render_state.bitmap_info.bmiHeader.biBitCount = 32;
			render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

		} break;

		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	srand(time(0));
	// Create window class 
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = TEXT("Game Window Class");
	window_class.lpfnWndProc = window_callback;

	// Register class
	RegisterClass(&window_class);

	// Create window
	HWND window = CreateWindow(window_class.lpszClassName, TEXT("Simulation"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, int(window_start_width), int(window_start_height), 0, 0, hInstance, 0);
	HDC hdc = GetDC(window);

	// Creating objects
	simulation_startup();

	//test

	while (running) {

		// Input
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		// Simulate
		simulation();

		// Render
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);
	}
}

