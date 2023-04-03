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

#include "Camera.h"
//#include "GameEntity.h"
class GameEntity;
#include "Mesh.h"
#include "Material.h"
#include "BufferStructs.h"
#include "Lights.h"

/// <summary>
/// The base class for rendering - here we are setting up the different things that DX12 needs to draw
/// </summary>

class RenderCore
{
public:
	//RenderCore(
	//	
	//);

	HRESULT InitDirectX(
		IDXGIAdapter1* hardwareAdapter,
		HWND windowHandle,
		int width,
		int height
	);

	virtual void OnResize(int width, int height);


	//this is for the SDFRenderer Init, which i would like to get rid of but im big tired tbh
	virtual void Init(bool vsync, std::shared_ptr<Camera> camera) = 0;

	virtual void Render(
		std::vector<std::shared_ptr<GameEntity>> entities,
		float color[4],
		float sphereSize,
		float lightPos[3],
		float spherePos[3]
	) = 0;

	virtual Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipeState() = 0;

	//make the frame-by-frame rendering functions pure virtual?
	
	//getter for the feature level? that feels like a lazy fix to still needing it in core but eh

	//getter for IsInitialized - check if device exists
	//that was the plan but enough stuff outside this class (in both game and dxcore) that its most practical right now to just have a getter for the device
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device; };
	ID3D12DescriptorHeap* GetSrvHeap() { return srvHeap; }; //similar case here
	D3D_FEATURE_LEVEL GetDXFeatureLevel() { return dxFeatureLevel; };//aaand here too
protected:

	void RenderImGui(); //maybe this isnt the best place to put this? but its not terrible. probably

	//originally from DXCore
	static const unsigned int numBackBuffers = 3;
	unsigned int currentSwapBuffer;
	D3D_FEATURE_LEVEL dxFeatureLevel;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[numBackBuffers];
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>commandList;
	unsigned int rtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	ID3D12DescriptorHeap* srvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[numBackBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[numBackBuffers];
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

private:
};
