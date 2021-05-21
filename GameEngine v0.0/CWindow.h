#pragma once
#include <Windows.h>



class CWindow
{


	struct App
	{
		HWND window;
		int width;
		int height;
	};
	int mWidth;
	int mHeight;
	HWND mainWindow = nullptr;
public:
	bool CreateCustomWindow(const wchar_t* className, const wchar_t* windowName, int width, int height, HINSTANCE hInstance);
	bool RegisterOwnClass(HINSTANCE hInstance, const wchar_t* className);

	HWND getWindow() { return mainWindow; }
};

