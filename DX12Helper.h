#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

#include "WICTextureLoader.h"
#include "ResourceUploadBatch.h"
using namespace DirectX;

class DX12Helper
{
#pragma region Singleton
public:
	// Gets the one and only instance of this class
	static DX12Helper& GetInstance()
	{
		if (!instance)
		{
			instance = new DX12Helper();
		}

		return *instance;
	}

	// Remove these functions (C++ 11 version)
	DX12Helper(DX12Helper const&) = delete;
	void operator=(DX12Helper const&) = delete;

private:
	static DX12Helper* instance;
	DX12Helper() {};
#pragma endregion

public:
	~DX12Helper();
	// Initialization for singleton
	void Initialize(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>* commandAllocators,
		unsigned int numBackBuffers);

	//added from raymarching demo
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(UINT64 size,
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		UINT64 alignment = 0);
	void ReserveSrvUavDescriptorHeapSlot(D3D12_CPU_DESCRIPTOR_HANDLE* reservedCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE* reservedGPUHandle);

	// Resource creation
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateStaticBuffer(
		unsigned int dataStride,
		unsigned int dataCount,
		void* data);
	// Command list & synchronization
	void CloseAndExecuteCommandList();
	void WaitForGPU();
	unsigned int SyncSwapChain(unsigned int currentSwapBufferIndex);


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCBVSRVDescriptorHeap();
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetDefaultAllocator();
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetAllocatorByIndex(unsigned int index);

	D3D12_GPU_DESCRIPTOR_HANDLE FillNextConstantBufferAndGetGPUDescriptorHandle(
		void* data,
		unsigned int dataSizeInBytes);

	D3D12_CPU_DESCRIPTOR_HANDLE LoadTexture(const wchar_t* file, bool generateMips = true);
	D3D12_GPU_DESCRIPTOR_HANDLE CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(
		D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptorToCopy,
		unsigned int numDescriptorsToCopy);
private:


	// Maximum number of constant buffers, assuming each buffer
	// is 256 bytes or less. Larger buffers are fine, but will
	// result in fewer buffers in use at any time
	const unsigned int maxConstantBuffers = 1000;
	// GPU-side contant buffer upload heap
	Microsoft::WRL::ComPtr<ID3D12Resource> cbUploadHeap;
	UINT64 cbUploadHeapSizeInBytes;
	UINT64 cbUploadHeapOffsetInBytes;
	void* cbUploadHeapStartAddress;
	// GPU-side CBV/SRV descriptor heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvSrvDescriptorHeap;
	SIZE_T cbvSrvDescriptorHeapIncrementSize;
	unsigned int cbvDescriptorOffset;
	void CreateConstantBufferUploadHeap();
	void CreateCBVSRVDescriptorHeap();

	// Overall device
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// Command list related
	// Note: We're assuming a single command list for the entire
	// engine at this point. That's not always true for more
	// complex engines but should be fine for now
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>* commandAllocators;
	// Basic CPU/GPU synchronization
	Microsoft::WRL::ComPtr<ID3D12Fence> waitFence;
	HANDLE waitFenceEvent;
	unsigned long waitFenceCounter;

	//frame sync
	unsigned int numBackBuffers;
	Microsoft::WRL::ComPtr<ID3D12Fence> frameSyncFence;
	HANDLE	frameSyncFenceEvent;
	UINT64* frameSyncFenceCounters;

	// Maximum number of texture descriptors (SRVs) we can have.
	// Each material will have a chunk of this,
	// Note: If we delayed the creation of this heap until
	// after all textures and materials were created,
	// we could come up with an exact amount. The following
	// constant ensures we (hopefully) never run out of room.
	const unsigned int maxTextureDescriptors = 1000;
	unsigned int srvDescriptorOffset;
	// Texture resources we need to keep alive
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textures;
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> cpuSideTextureDescriptorHeaps;


	//// DirectX related objects and variables (originally located in DXCore)
	//static const unsigned int numBackBuffers = 3;
	//unsigned int currentSwapBuffer;
	//D3D_FEATURE_LEVEL dxFeatureLevel;
	//Microsoft::WRL::ComPtr<ID3D12Device> device;
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
};

