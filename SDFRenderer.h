#pragma once

#include "DXCore.h"
#include "Game.h"
#include "DX12Helper.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>

#include "Camera.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Material.h"
#include "BufferStructs.h"
#include "Lights.h"
class SDFRenderer
	//: public DXCore //i dont know if this is a good idea yet but sure lets go with this for now
{ //Probably *not* going to work to inherit from dxcore. remember how the asset manager was a pain this way? Probably going to have to do something like that with shaders being initialized in game and then sent over here. ugh
	//other option is to pull out stuff that this needs into its own singleton? that sounds like it could break a lot of stuff though
	friend class DXCore; //this may be the most effective option available right now
	/*
	* Need to:
	*	load shaders
	*	--i think just load the shaders in game and pass them here tbh.
	*	set up input layout, the variables that the sdf shaders will need
	*	
	*	set up root signature - which describes the pipeline (buffers, etc)
	*	
	*	set up pipeline state!!!
	* 
	*	set up PostResize here?
	* 
	* I think it makes sense to have the shaders load here, at least for now until we decide it makes sense to implement an asset manager class (a big task...)
	*	
	*/

public:
	SDFRenderer(
		bool vsync,
		unsigned int numBackBuffers,
		unsigned int currentSwapBuffer,
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocators,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>commandList,
		unsigned int rtvDescriptorSize,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap,
		ID3D12DescriptorHeap* srvHeap,
		D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandles,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		Microsoft::WRL::ComPtr<ID3D12Resource>& backBuffers,
		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
		D3D12_VIEWPORT viewport,
		D3D12_RECT scissorRect,
		std::shared_ptr<Camera> camera,
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode,
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode
	);

	//this will be the only ctor if refactoring this as "friend" works as intended
	/*SDFRenderer(
		Game* game,

	);*/

	~SDFRenderer();

	void Render();

private:
	//Microsoft::WRL::ComPtr<ID3D12Device> device;

#pragma region Stuff from DXCore/Game
	//stuff that is at least for now is just going to be passed in the constructor here from game
	//bool vsync;
	//unsigned int numBackBuffers;
	//unsigned int currentSwapBuffer;
	//Microsoft::WRL::ComPtr<ID3D12Device> device;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators;// [numBackBuffers] ; //cant init here with an undefined variable. shouldnt be necessary though since this will just be a copy of whatever is already defined
	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>commandList;
	//unsigned int rtvDescriptorSize;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	////ID3D12DescriptorHeap* srvHeap;
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles;// [numBackBuffers] ;
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	//Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers;// [numBackBuffers] ;
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	//D3D12_VIEWPORT viewport;
	//D3D12_RECT scissorRect;


	std::shared_ptr<Camera> camera;

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;
	
	//definitely would not use traditional entities, but maybe would add another entity type later
	//same with materials, lights, etc

	//DXCore dxCore;
	Game* game; //uuuh heck apparently dxcore is abstract or something??
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;



	void CreateRootSigAndPipelineState();
	//I would like to split up the above function into these somehow, but there may not be a way to do that without it being a pain. Let's just get this up and running for now
	//void LoadShaders();
	//D3D12_INPUT_ELEMENT_DESC DefineInputLayout();
	//void CreateRootSignature();
	//void CreatePipelineState();

};

