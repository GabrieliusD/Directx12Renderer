#include "CWindow.h"
#include "EngineFrame.h"
EngineFrame* frame;

ActiveState activeState;
Key key;
bool resizing = false;
int newWidth;
int newHeight;
bool trigger = false;
bool MouseMoved = false;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		frame->MouseInput(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		activeState.LMBPressed = true;
		activeState.lParam = lParam;
		activeState.wParam = wParam;
		break;
	case WM_MOUSEMOVE:
		frame->MouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		MouseMoved = true;
		break;
	case WM_SIZE:
		newWidth = LOWORD(lParam);
		newHeight = HIWORD(lParam);
		if (resizing)
		{

		}
		else
		{
			trigger = true;
		}
		break;
	case WM_ENTERSIZEMOVE:
		resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		resizing = false;
		trigger = true;
		break;
	case WM_KEYDOWN:
		key.pressed = true;
		if(VK_SHIFT)
		key.keyPressed = static_cast<unsigned char>(wParam)+32;
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
int main() {
	WinMain(GetModuleHandle(NULL), NULL, NULL, 1);
	return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	const wchar_t* className = L"d3d";
	CWindow cwindow;

	cwindow.RegisterOwnClass(hInstance, className);
	cwindow.CreateCustomWindow(className, L"d3dWin", 800, 600, hInstance);
	//GET_LPARAM(cwindow.getWindow(), l);
	//GET_WPARAM
	ShowWindow(cwindow.getWindow(), nCmdShow);
	UpdateWindow(cwindow.getWindow());
	Font arial = Utility::LoadFont(L"newarial.fnt",800,600);
	EngineFrame engine(cwindow.getWindow());
	engine.Initialize();
	engine.InitDrawingStuff();
	engine.InitObjectRender();
	engine.InitCamera();
	frame = &engine;

	GameTimer timer;
	timer.Start();
	timer.Reset();
	while (true)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();
			engine.ChangeWorld();
			engine.Update(timer);

			engine.Draw();
			
						
		}
		if (msg.message == WM_QUIT) break;

		if (key.pressed)
		{
			engine.KeyTracking(key);
			key.pressed = false;
		}

		if (trigger)
		{
			trigger = false;
			engine.SetWindowSize(newWidth, newHeight);
			engine.OnResize();
		}
		static int frameCnt = 0;
		static float timeElapsed = 0.0f;

		frameCnt++;

		if ((timer.TotalTime() - timeElapsed) >= 1.0f)
		{
			float fps = (float)frameCnt;
			float mspf = 1000.0f / fps;

			std::wstring fpsStr = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring windowText = L"fps: " + fpsStr + L" mspf: " + mspfStr;

			SetWindowText(cwindow.getWindow(), windowText.c_str());

			frameCnt = 0;
			timeElapsed += 1.0f;
		}
	}

	return 0;
}


bool CWindow::CreateCustomWindow(const wchar_t* className, const wchar_t* windowName, int width, int height, HINSTANCE hInstance)
{
	mWidth = width;
	mHeight = height;
	RECT r = { 0,0,width,height };
	AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, false, WS_EX_CLIENTEDGE);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	mainWindow = CreateWindowEx(WS_EX_CLIENTEDGE, className, windowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, hInstance,NULL);
	return mainWindow ? true: false;
}

bool CWindow::RegisterOwnClass(HINSTANCE hInstance, const wchar_t* className)
{
	WNDCLASSEX wc = { 0 };
	
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.hIconSm = wc.hIcon;
	
	return RegisterClassEx(&wc) ? true:false;
}
