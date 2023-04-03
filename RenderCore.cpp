#include "RenderCore.h"
#include "Input.h"
#include "DX12Helper.h"

#include <WindowsX.h>
#include <sstream>
#include <imgui.h>
#include <imgui_impl_win32.h>

#include "GameEntity.h" //forward declaration for this is used in RenderCore.h

// --------------------------------------------------------
// Initializes DirectX, which requires a window.  This method
// also creates several DirectX objects we'll need to start
// drawing things to the screen.
// --------------------------------------------------------
HRESULT RenderCore::InitDirectX(IDXGIAdapter1* hardwareAdapter,	HWND windowHandle, int width, int height)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Enable debug layer for runtime debug errors/warnings in visual studio output
	ID3D12Debug* debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif
	// Result variable for below function calls
	HRESULT hr = S_OK;
	// Create the DX 12 device and check the feature level
	{
		//---These are being called in DXCore before it calls this function
		//IDXGIFactory4* factory;
		//CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		//IDXGIAdapter1* hardwareAdapter;
		//GetHardwareAdapter(factory, &hardwareAdapter, true);
		hr = D3D12CreateDevice(
			hardwareAdapter, // Not explicitly specifying which adapter (GPU) (but now we are specifying)
			D3D_FEATURE_LEVEL_11_0, // MINIMUM feature level - NOT the level we'll turn on
			IID_PPV_ARGS(device.GetAddressOf())); // Macro to grab necessary IDs of device
		if (FAILED(hr)) return hr;
		// Now that we have a device, determine the maximum
		// feature level supported by the device
		D3D_FEATURE_LEVEL levelsToCheck[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1
		};
		D3D12_FEATURE_DATA_FEATURE_LEVELS levels = {};
		levels.pFeatureLevelsRequested = levelsToCheck;
		levels.NumFeatureLevels = ARRAYSIZE(levelsToCheck);
		device->CheckFeatureSupport(
			D3D12_FEATURE_FEATURE_LEVELS,
			&levels,
			sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS));
		dxFeatureLevel = levels.MaxSupportedFeatureLevel;
	}
	// Set up DX12 command allocator / queue / list, 
	// which are necessary pieces for issuing standard API calls
	{
		// Set up allocator
		for (unsigned int i = 0; i < numBackBuffers; i++)
		{
			// Set up allocators
			device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(commandAllocators[i].GetAddressOf()));
		}
		// Command queue
		D3D12_COMMAND_QUEUE_DESC qDesc = {};
		qDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		qDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		device->CreateCommandQueue(&qDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
		// Command list
		device->CreateCommandList(
			0, // Which physical GPU will handle these tasks? 0 for single GPU setup
			D3D12_COMMAND_LIST_TYPE_DIRECT, // Type of list - direct is for standard API calls
			commandAllocators[0].Get(), // The allocator for this list
			0, // Initial pipeline state - none for now
			IID_PPV_ARGS(commandList.GetAddressOf()));
	}
	// Now that we have a device and a command list,
	// we can initialize the DX12 helper singleton, which will
	// also create a fence for synchronization
	{
		DX12Helper::GetInstance().Initialize(
			device,
			commandList,
			commandQueue,
			commandAllocators,
			numBackBuffers);
	}
	// Swap chain creation
	{
		// Create a description of how our swap chain should work
		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = numBackBuffers;
		swapDesc.BufferDesc.Width = width;
		swapDesc.BufferDesc.Height = height;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.Flags = 0;
		swapDesc.OutputWindow = windowHandle;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Windowed = true;
		// Create a DXGI factory, which is what we use to create a swap chain
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
		CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
		hr = dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapDesc, swapChain.GetAddressOf());
	}

	// Still inside DXCore::InitDirectX()! Create back buffers
	{
		// What is the increment size between RTV descriptors in a
		// descriptor heap? This differs per GPU so we need to 
		// get it at applications start up
		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		// First create a descriptor heap for RTVs
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = numBackBuffers;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf()));
		// Now create the RTV handles for each buffer (buffers were created by the swap chain)
		for (unsigned int i = 0; i < numBackBuffers; i++)
		{
			// Grab this buffer from the swap chain
			swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffers[i].GetAddressOf()));
			// Make a handle for it
			rtvHandles[i] = rtvHeap->GetCPUDescriptorHandleForHeapStart();
			rtvHandles[i].ptr += rtvDescriptorSize * i;
			// Create the render target view
			device->CreateRenderTargetView(backBuffers[i].Get(), 0, rtvHandles[i]);
		}
	}

	// Create depth/stencil buffer
	{
		// Create a descriptor heap for DSV
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf()));
		// Describe the depth stencil buffer resource
		D3D12_RESOURCE_DESC depthBufferDesc = {};
		depthBufferDesc.Alignment = 0;
		depthBufferDesc.DepthOrArraySize = 1;
		depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.Height = height;
		depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Width = width;
		// Describe the clear value that will most often be used
		// for this buffer (which optimizes the clearing of the buffer)
		D3D12_CLEAR_VALUE clear = {};
		clear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clear.DepthStencil.Depth = 1.0f;
		clear.DepthStencil.Stencil = 0;
		// Describe the memory heap that will house this resource
		D3D12_HEAP_PROPERTIES props = {};
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.CreationNodeMask = 1;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		props.VisibleNodeMask = 1;
		// Actually create the resource, and the heap in which it
		// will reside, and map the resource to that heap
		device->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&depthBufferDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clear,
			IID_PPV_ARGS(depthStencilBuffer.GetAddressOf()));
		// Get the handle to the Depth Stencil View that we'll
		// be using for the depth buffer. The DSV is stored in
		// our DSV-specific descriptor Heap.
		dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		// Actually make the DSV
		device->CreateDepthStencilView(
			depthStencilBuffer.Get(),
			0, // Default view (first mip)
			dsvHandle);
	}

	// Set up the viewport so we render into the correct
// portion of the render target
	viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	// Define a scissor rectangle that defines a portion of
	// the render target for clipping. This is different from
	// a viewport in that it is applied after the pixel shader.
	// We need at least one of these, but we're rendering to 
	// the entire window, so it'll be the same size.
	scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;
	// Wait for the GPU to catch up
	DX12Helper::GetInstance().WaitForGPU();
	// Return the "everything is ok" HRESULT value
	return S_OK;
}

// --------------------------------------------------------
// When the window is resized, the underlying 
// buffers (textures) must also be resized to match.
//
// If we don't do this, the window size and our rendering
// resolution won't match up.  This can result in odd
// stretching/skewing.
// --------------------------------------------------------
void RenderCore::OnResize(int width, int height)
{
	// Wait for the GPU to finish all work, since we'll
// be destroying and recreating resources
	DX12Helper::GetInstance().WaitForGPU();
	// Release the back buffers using ComPtr's Reset()
	for (unsigned int i = 0; i < numBackBuffers; i++)
		backBuffers[i].Reset();
	// Resize the swap chain (assuming a basic color format here)
	swapChain->ResizeBuffers(numBackBuffers, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	// Go through the steps to setup the back buffers again
	// Note: This assumes the descriptor heap already exists
	// and that the rtvDescriptorSize was previously set
	for (unsigned int i = 0; i < numBackBuffers; i++)
	{
		// Grab this buffer from the swap chain
		swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffers[i].GetAddressOf()));
		// Make a handle for it
		rtvHandles[i] = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandles[i].ptr += rtvDescriptorSize * i;
		// Create the render target view
		device->CreateRenderTargetView(backBuffers[i].Get(), 0, rtvHandles[i]);
	}
	// Reset back to the first back buffer
	currentSwapBuffer = 0;

	// Reset the depth buffer and create it again
	{
		depthStencilBuffer.Reset();
		// Describe the depth stencil buffer resource
		D3D12_RESOURCE_DESC depthBufferDesc = {};
		depthBufferDesc.Alignment = 0;
		depthBufferDesc.DepthOrArraySize = 1;
		depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.Height = height;
		depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Width = width;
		// Describe the clear value that will most often be used
		// for this buffer (which optimizes the clearing of the buffer)
		D3D12_CLEAR_VALUE clear = {};
		clear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clear.DepthStencil.Depth = 1.0f;
		clear.DepthStencil.Stencil = 0;
		// Describe the memory heap that will house this resource
		D3D12_HEAP_PROPERTIES props = {};
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.CreationNodeMask = 1;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		props.VisibleNodeMask = 1;
		// Actually create the resource, and the heap in which it
		// will reside, and map the resource to that heap
		device->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&depthBufferDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clear,
			IID_PPV_ARGS(depthStencilBuffer.GetAddressOf()));
		// Now recreate the depth stencil view
		dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		device->CreateDepthStencilView(
			depthStencilBuffer.Get(),
			0, // Default view (first mip)
			dsvHandle);
	}

	// Recreate the viewport and scissor rects, too,
		// since the window size has changed
	{
		// Update the viewport and scissor rect so we render into the correct
		// portion of the render target
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		scissorRect.right = width;
		scissorRect.bottom = height;
	}
	// Wait for the GPU before we proceed
	DX12Helper::GetInstance().WaitForGPU();
}

void RenderCore::RenderImGui()
{
	ImGui::Render();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = backBuffers[currentSwapBuffer].Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->SetDescriptorHeaps(1, &srvHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);
}
