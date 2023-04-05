#include "DXCore.h"
#include "Input.h"
#include "DX12Helper.h"

#include <WindowsX.h>
#include <sstream>
#include <imgui.h>
#include <imgui_impl_win32.h>

#include "RenderCore.h" //forward declaration for this is used in DXCore.h

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Define the static instance variable so our OS-level 
// message handling function below can talk to our object
DXCore* DXCore::DXCoreInstance = 0;

// --------------------------------------------------------
// The global callback function for handling windows OS-level messages.
//
// This needs to be a global function (not part of a class), but we want
// to forward the parameters to our class to properly handle them.
// --------------------------------------------------------
LRESULT DXCore::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	return DXCoreInstance->ProcessMessage(hWnd, uMsg, wParam, lParam);
}

// --------------------------------------------------------
// Constructor - Set up fields and timer
//
// hInstance	- The application's OS-level handle (unique ID)
// titleBarText - Text for the window's title bar
// windowWidth	- Width of the window's client (internal) area
// windowHeight - Height of the window's client (internal) area
// debugTitleBarStats - Show debug stats in the title bar, like FPS?
// --------------------------------------------------------
DXCore::DXCore(
	HINSTANCE hInstance,		// The application's handle
	const char* titleBarText,	// Text for the window's title bar
	unsigned int windowWidth,	// Width of the window's client area
	unsigned int windowHeight,	// Height of the window's client area
	bool debugTitleBarStats)
	//srvHeap(nullptr)	// Show extra stats (fps) in title bar? not really necessary with imgui set up now
{
	// Save a static reference to this object.
	//  - Since the OS-level message function must be a non-member (global) function, 
	//    it won't be able to directly interact with our DXCore object otherwise.
	//  - (Yes, a singleton might be a safer choice here).
	DXCoreInstance = this;

	// Save params
	this->hInstance = hInstance;
	this->titleBarText = titleBarText;
	this->width = windowWidth;
	this->height = windowHeight;
	this->titleBarStats = debugTitleBarStats;

	// Initialize fields
	this->hasFocus = true; 
	
	this->fpsFrameCount = 0;
	this->fpsTimeElapsed = 0.0f;
	this->currentTime = 0;
	this->deltaTime = 0;
	this->startTime = 0;
	this->totalTime = 0;

	// Query performance counter for accurate timing information
	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;
}

// --------------------------------------------------------
// Destructor - Clean up (release) all DirectX references
// --------------------------------------------------------
DXCore::~DXCore()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in DXCore

	// Delete input manager singleton
	delete& Input::GetInstance();
	delete& DX12Helper::GetInstance();
}

// --------------------------------------------------------
// Created the actual window for our application
// --------------------------------------------------------
HRESULT DXCore::InitWindow()
{
	// Start window creation by filling out the
	// appropriate window class struct
	WNDCLASS wndClass		= {}; // Zero out the memory
	wndClass.style			= CS_HREDRAW | CS_VREDRAW;	// Redraw on horizontal or vertical movement/adjustment
	wndClass.lpfnWndProc	= DXCore::WindowProc;
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hInstance		= hInstance;						// Our app's handle
	wndClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	// Default icon
	wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);		// Default arrow cursor
	wndClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName	= NULL;
	wndClass.lpszClassName	= "Direct3DWindowClass";

	// Attempt to register the window class we've defined
	if (!RegisterClass(&wndClass))
	{
		// Get the most recent error
		DWORD error = GetLastError();

		// If the class exists, that's actually fine.  Otherwise,
		// we can't proceed with the next step.
		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	// Adjust the width and height so the "client size" matches
	// the width and height given (the inner-area of the window)
	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(
		&clientRect,
		WS_OVERLAPPEDWINDOW,	// Has a title bar, border, min and max buttons, etc.
		false);					// No menu bar

	// Center the window to the screen
	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	// Actually ask Windows to create the window itself
	// using our settings so far.  This will return the
	// handle of the window, which we'll keep around for later
	hWnd = CreateWindow(
		wndClass.lpszClassName,
		titleBarText.c_str(),
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,	// Calculated width
		clientRect.bottom - clientRect.top,	// Calculated height
		0,			// No parent window
		0,			// No menu
		hInstance,	// The app's handle
		0);			// No other windows in our application

	// Ensure the window was created properly
	if (hWnd == NULL)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	// The window exists but is not visible yet
	// We need to tell Windows to show it, and how to show it
	ShowWindow(hWnd, SW_SHOW);

	// Initialize the input manager now that we definitely have a window
	Input::GetInstance().Initialize(hWnd);

	// Return an "everything is ok" HRESULT value
	return S_OK;
}


void DXCore::InitializeImGui()
{
	ID3D12DescriptorHeap* srvHeap = nullptr;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap)) != S_OK)
		return;
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX12_Init(device.Get(), 3,
		DXGI_FORMAT_R8G8B8A8_UNORM, srvHeap,
		srvHeap->GetCPUDescriptorHandleForHeapStart(),
		srvHeap->GetGPUDescriptorHandleForHeapStart());
	renderer->SetSrvHeap(srvHeap);
}

void GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	IDXGIAdapter1* adapter = nullptr;

	IDXGIFactory6* factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter)));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter;
}
HRESULT DXCore::InitDirectX()
{
	IDXGIFactory4* factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	IDXGIAdapter1* hardwareAdapter;
	GetHardwareAdapter(factory, &hardwareAdapter, true);
	HRESULT hr = renderer->InitDirectX(hardwareAdapter, hWnd, width, height);

	//grab the device from the renderer, so that we can use it here and in game
	device = renderer->GetDevice();

	return hr;
}

// --------------------------------------------------------
// This is the main game loop, handling the following:
//  - OS-level messages coming in from Windows itself
//  - Calling update & draw back and forth, forever
// --------------------------------------------------------
HRESULT DXCore::Run()
{
	// Grab the start time now that
	// the game loop is running
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	startTime = now;
	currentTime = now;
	previousTime = now;

	// Give subclass a chance to initialize
	Init();
	Input::GetInstance();
	InitializeImGui();
	// Our overall game and message loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Determine if there is a message waiting
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate and dispatch the message
			// to our custom WindowProc function
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Update timer and title bar (if necessary)
			UpdateTimer();
			if(titleBarStats)
				UpdateTitleBarStats();

			
			// Update the input manager
			Input::GetInstance().Update();

			// The game loop
			Update(deltaTime, totalTime);
			Draw(deltaTime, totalTime);
			

			// Frame is over, notify the input manager
			Input::GetInstance().EndOfFrame();
		}
	}
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	// We'll end up here once we get a WM_QUIT message,
	// which usually comes from the user closing the window
	return (HRESULT)msg.wParam;
}


// --------------------------------------------------------
// Sends an OS-level window close message to our process, which
// will be handled by our message processing function
// --------------------------------------------------------
void DXCore::Quit()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	PostMessage(this->hWnd, WM_CLOSE, NULL, NULL);
}


// --------------------------------------------------------
// Uses high resolution time stamps to get very accurate
// timing information, and calculates useful time stats
// --------------------------------------------------------
void DXCore::UpdateTimer()
{
	// Grab the current time
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	// Calculate delta time and clamp to zero
	//  - Could go negative if CPU goes into power save mode 
	//    or the process itself gets moved to another core
	deltaTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);

	// Calculate the total time from start to now
	totalTime = (float)((currentTime - startTime) * perfCounterSeconds);

	// Save current time for next frame
	previousTime = currentTime;
}


// --------------------------------------------------------
// Updates the window's title bar with several stats once
// per second, including:
//  - The window's width & height
//  - The current FPS and ms/frame
//  - The version of DirectX actually being used (usually 11)
// --------------------------------------------------------
void DXCore::UpdateTitleBarStats()
{
	fpsFrameCount++;

	// Only calc FPS and update title bar once per second
	float timeDiff = totalTime - fpsTimeElapsed;
	if (timeDiff < 1.0f)
		return;

	// How long did each frame take?  (Approx)
	float mspf = 1000.0f / (float)fpsFrameCount;

	// Quick and dirty title bar text (mostly for debugging)
	std::ostringstream output;
	output.precision(6);
	output << titleBarText <<
		"    Width: "		<< width <<
		"    Height: "		<< height <<
		"    FPS: "			<< fpsFrameCount <<
		"    Frame Time: "	<< mspf << "ms";

	// Append the version of DirectX the app is using
	switch (renderer->GetDXFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_12_1: output << " DX 12.1"; break; // New!
	case D3D_FEATURE_LEVEL_12_0: output << " DX 12.0"; break; // New!
	case D3D_FEATURE_LEVEL_11_1: output << "    DX 11.1"; break;
	case D3D_FEATURE_LEVEL_11_0: output << "    DX 11.0"; break;
	case D3D_FEATURE_LEVEL_10_1: output << "    DX 10.1"; break;
	case D3D_FEATURE_LEVEL_10_0: output << "    DX 10.0"; break;
	case D3D_FEATURE_LEVEL_9_3:  output << "    DX 9.3";  break;
	case D3D_FEATURE_LEVEL_9_2:  output << "    DX 9.2";  break;
	case D3D_FEATURE_LEVEL_9_1:  output << "    DX 9.1";  break;
	default:                     output << "    DX ???";  break;
	}

	// Actually update the title bar and reset fps data
	SetWindowText(hWnd, output.str().c_str());
	fpsFrameCount = 0;
	fpsTimeElapsed += 1.0f;
}

// --------------------------------------------------------
// Allocates a console window we can print to for debugging
// 
// bufferLines   - Number of lines in the overall console buffer
// bufferColumns - Numbers of columns in the overall console buffer
// windowLines   - Number of lines visible at once in the window
// windowColumns - Number of columns visible at once in the window
// --------------------------------------------------------
void DXCore::CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns)
{
	// Our temp console info struct
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	// Get the console info and set the number of lines
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = bufferLines;
	coninfo.dwSize.X = bufferColumns;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = windowColumns;
	rect.Bottom = windowLines;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

	// Prevent accidental console window close
	HWND consoleHandle = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(consoleHandle, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
}

// --------------------------------------------------------
// Handles messages that are sent to our window by the
// operating system.  Ignoring these messages would cause
// our program to hang and Windows would think it was
// unresponsive.
// --------------------------------------------------------
LRESULT DXCore::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Check the incoming message and handle any we care about
	switch (uMsg)
	{
	// This is the message that signifies the window closing
	case WM_DESTROY:
		PostQuitMessage(0); // Send a quit message to our own program
		return 0;

	// Prevent beeping when we "alt-enter" into fullscreen
	case WM_MENUCHAR: 
		return MAKELRESULT(0, MNC_CLOSE);

	// Prevent the overall window from becoming too small
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	// Sent when the window size changes
	case WM_SIZE:
		// Don't adjust anything when minimizing,
		// since we end up with a width/height of zero
		// and that doesn't play well with the GPU
		if (wParam == SIZE_MINIMIZED)
			return 0;
		
		// Save the new client area dimensions.
		width = LOWORD(lParam);
		height = HIWORD(lParam);

		// If DX is initialized, resize 
		// our required buffers
		if (device) 
			renderer->OnResize(width, height);

		return 0;

	// Has the mouse wheel been scrolled?
	case WM_MOUSEWHEEL:
		Input::GetInstance().SetWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;
	
	// Is our focus state changing?
	case WM_SETFOCUS:	hasFocus = true;	return 0;
	case WM_KILLFOCUS:	hasFocus = false;	return 0;
	case WM_ACTIVATE:	hasFocus = (LOWORD(wParam) != WA_INACTIVE); return 0;
	}

	// Let Windows handle any messages we're not touching
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

