#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>

//#include "RenderCore.h"
class RenderCore;

// We can include the correct library files here
// instead of in Visual Studio settings if we want
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DXCore
{
	//friend class SDFRenderer;
public:
	DXCore(
		HINSTANCE hInstance,		// The application's handle
		const char* titleBarText,	// Text for the window's title bar
		unsigned int windowWidth,	// Width of the window's client area
		unsigned int windowHeight,	// Height of the window's client area
		bool debugTitleBarStats);	// Show extra stats (fps) in title bar?
	~DXCore();

	// Static requirements for OS-level message processing
	static DXCore* DXCoreInstance;
	static LRESULT CALLBACK WindowProc(
		HWND hWnd,		// Window handle
		UINT uMsg,		// Message
		WPARAM wParam,	// Message's first parameter
		LPARAM lParam	// Message's second parameter
	);

	// Internal method for message handling
	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Initialization and game-loop related methods
	HRESULT InitWindow();
	HRESULT InitDirectX();
	void InitializeImGui();
	//void renderImGui();
	HRESULT Run();
	void Quit();
	//virtual void OnResize();

	// Pure virtual methods for setup and game functionality
	virtual void Init() = 0;
	virtual void Update(float deltaTime, float totalTime) = 0;
	virtual void Draw(float deltaTime, float totalTime) = 0;

protected:
	HINSTANCE	hInstance;		// The handle to the application
	HWND		hWnd;			// The handle to the window itself
	std::string titleBarText;	// Custom text in window's title bar
	bool		titleBarStats;	// Show extra stats in title bar?
	ImGuiIO io ;
	// Size of the window's client area
	unsigned int width;
	unsigned int height;

	// Does our window currently have focus?
	// Helpful if we want to pause while not the active window
	bool hasFocus;

	std::shared_ptr<RenderCore> renderer; //this is not ideal for flexibility now that i think about it. but again, time constraints
	// DirectX related objects and variables
	//static const unsigned int numBackBuffers = 3;
	//unsigned int currentSwapBuffer;
	//D3D_FEATURE_LEVEL dxFeatureLevel;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[numBackBuffers];
	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>commandList;
	//unsigned int rtvDescriptorSize;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	//ID3D12DescriptorHeap* srvHeap;
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[numBackBuffers];
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	//Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[numBackBuffers];
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	//D3D12_VIEWPORT viewport;
	//D3D12_RECT scissorRect;

	//breadcrumb: either use just make this a friend of the SDF renderer, or refactor to a "render core" that has these dx things and probably stuff with the helper

	// Helper function for allocating a console window
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);

	//// Helpers for determining the actual path to the executable
	//std::string GetExePath();
	//std::wstring GetExePath_Wide();

	//std::string GetFullPathTo(std::string relativeFilePath);
	//std::wstring GetFullPathTo_Wide(std::wstring relativeFilePath);



private:
	// Timing related data
	double perfCounterSeconds;
	float totalTime;
	float deltaTime;
	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	// FPS calculation
	int fpsFrameCount;
	float fpsTimeElapsed;

	void UpdateTimer();			// Updates the timer for this frame
	void UpdateTitleBarStats();	// Puts debug info in the title bar

};

