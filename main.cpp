#include <windows.h>
#include <stdint.h>

typedef unsigned char uint8;
typedef uint8_t uint8;

static bool Running;
static BITMAPINFO bitmapInfo;
static void* bitmapMemory;
static HBITMAP bitmapHandle;
static int bitmapWidth;
static int bitmapHeight;

static void Win32ResizeDIBSection(int width, int height)
{
	if (bitmapMemory)
	{
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	//bitmapInfo.bmiHeader.biSizeImage = 0;
	//bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	//bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	//bitmapInfo.bmiHeader.biClrUsed = 0;
	//bitmapInfo.bmiHeader.biClrImportant = 0;

	//   
	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	int pitch = width * bytesPerPixel;
	uint8* row = (uint8 *)bitmapMemory;
	for (int y = 0; y < bitmapHeight; ++y)
	{
		uint8* pixel = (uint8*)row;
		for (int x = 0; x < bitmapWidth; ++x)
		{
			*pixel = 255;
			++pixel;

			*pixel = 0;
			++pixel;

			*pixel = 0;
			++pixel;

			*pixel = 0;
			++pixel;
		}
		row += pitch;
	}
}

static void Win32UpdateWindow(HDC DeviceContext, RECT *windowRect, int x, int y, int width, int height)
{
	/*

	*/

	int windowWidth = windowRect->right - windowRect->left;
	int windowHeight = windowRect->bottom - windowRect->top;

	StretchDIBits(DeviceContext,
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight,
		bitmapMemory,
		&bitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallBack(
	HWND Window,
	UINT Message,
	WPARAM wParam,
	LPARAM lParam
)
{
	LRESULT Result = 0;
	switch (Message)
	{
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int width = ClientRect.right - ClientRect.left;
			int height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSection(width, height);

			// here we pass in window size? through get client rect
			OutputDebugStringA("WM_SIZE\n");
		} break;
		case WM_DESTROY:
		{
			Running = false;
		} break;
		case WM_CLOSE:
		{
			Running = false;
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int x = Paint.rcPaint.left;
			int y = Paint.rcPaint.top;
			int width = Paint.rcPaint.right - Paint.rcPaint.left;
			int height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);

			Win32UpdateWindow(DeviceContext, &ClientRect, x, y, width, height);
			EndPaint(Window, &Paint);
		}	break;

		default:
			Result = DefWindowProc(Window, Message, wParam, lParam);
			break;
	}
	return Result;
}

// TODO: CALLBACK here is a calling convention
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	WNDCLASS	WindowClass = {};
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallBack;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = L"Handmade Hero Series";

	if (RegisterClass(&WindowClass))
	{
		HWND WindlwHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			L"Handmade hero",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0
		);

		if (WindlwHandle)
		{
			Running = true;
			while (Running)
			{
				MSG msg;
				BOOL bRet = GetMessage(&msg, 0, 0, 0);
				if (bRet != -1)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					break;
					// TODO: handle the error and possibly exit
				}
			}
		}
		else
		{
			// TODO: logging error
		}
	}
	else
	{
		// TODO: logging error
	}

	return 0;
}