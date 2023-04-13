#include "SDFRenderer.h"
#include "Input.h"
#include "WICTextureLoader.h"


#include "BufferStructs.h"

#include "RaytracingHelper.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include "SDFEntity.h"

#include <iostream>

void SDFRenderer::Init(bool vsync, std::shared_ptr<Camera> camera)
{
	this->vsync = vsync;
	this->camera = camera;

	CreateRootSigAndPipelineState();

	createTriangleForScreenQuad();
	// Ensure the command list is closed going into Draw for the first time
	commandList->Close();
}

SDFRenderer::~SDFRenderer()
{
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void SDFRenderer::Render()
{
	DX12Helper& dx12HelperInst = DX12Helper::GetInstance();


	//reset allocator for THIS buffer and set up the command list to use THIS allocator for THIS buffer
	commandAllocators[currentSwapBuffer]->Reset();
	commandList->Reset(commandAllocators[currentSwapBuffer].Get(), 0);
	//dx12HelperInst.GetAllocatorByIndex(currentSwapBuffer)->Reset();
	//commandList->Reset(dx12HelperInst.GetAllocatorByIndex(currentSwapBuffer).Get(), 0);


	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = backBuffers[currentSwapBuffer];

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &rb);

		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };

		// Clear the RTV
		commandList->ClearRenderTargetView(
			rtvHandles[currentSwapBuffer],
			color,
			0, 0); // No scissor rectangles

		// Clear the depth buffer, too
		commandList->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f, // Max depth = 1.0f
			0, // Not clearing stencil, but need a value
			0, 0); // No scissor rects
	}

	//{
	//	//io.WantCaptureMouse = true;
	//	static float f = 0.0f;

	//	ImGui::Begin("Settings", NULL, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.
	//	SDFEntity::GetSDFEntity()->DisplaySDFSettings();

	//	

	//

	//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//	ImGui::End();
	//	//ImGui::End();
	//}

	// Rendering here!
	{
		// Set overall pipeline state
		commandList->SetPipelineState(pipelineState.Get());
		// Root sig (must happen before root descriptor table)
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		// Set up other commands for rendering
		commandList->OMSetRenderTargets(1, &rtvHandles[currentSwapBuffer], true, &dsvHandle);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap =
			dx12HelperInst.GetCBVSRVDescriptorHeap();
		commandList->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());



		{
			RaymarchVSExternalData externalData = {};

			//			//send to a chunk of a constant buffer heap, and grab the GPU handle we need to draw
			D3D12_GPU_DESCRIPTOR_HANDLE handleVS = dx12HelperInst.FillNextConstantBufferAndGetGPUDescriptorHandle((void*)(&externalData), sizeof(externalData));

			commandList->SetGraphicsRootDescriptorTable(0, handleVS);
		}
		// Pixel shader data and cbuffer setup
		{
			 // TODO: should be made as member variable 

			/*
			TODO: Below should be moved inside Camera class

			why?
			*/
			
			auto entity = SDFEntity::GetSDFEntity();
			//std::shared_ptr<RaymarchPSExternalData> psData = entity->GetRayMarchPSData();
			auto psData = entity->GetRayMarchPSData();
			XMFLOAT3 pos = camera->GetTransform()->GetPosition();
			psData->cameraPosition = XMFLOAT3A(pos.x, pos.y, pos.z);
			XMStoreFloat3(&(psData->cameraForward), camera->GetForward());
			XMStoreFloat3(&(psData->cameraRight), camera->GetRight());
			XMStoreFloat3(&(psData->cameraUp), camera->GetUp());
			psData->bgColor = XMFLOAT3A(0.0f, 0.0f, 0.0f);


			//// Send this to a chunk of the constant buffer heap
			//// and grab the GPU handle for it so we can set it for this draw
			D3D12_GPU_DESCRIPTOR_HANDLE cbHandlePS =
				dx12HelperInst.FillNextConstantBufferAndGetGPUDescriptorHandle(
					(void*)(psData), sizeof(RaymarchPSExternalData));
			//// Set this constant buffer handle
			//// Note: This assumes that descriptor table 1 is the
			//// place to put this particular descriptor. This
			//// is based on how we set up our root signature.
			commandList->SetGraphicsRootDescriptorTable(1, cbHandlePS);
		}



		// Set them using IASetVertexBuffers() and IASetIndexBuffer()
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// Call DrawIndexedInstanced() using the index count of this entity's mesh
		commandList->DrawInstanced(3, 1, 0, 0); //first is the PER INSTANCE index count. second is HOW MANY of the INSTANCES themselves
		//commandList->DrawIndexedInstanced(0, 0, 0, 0, 0);//0 may not work at all for the first two but its true so


		RenderImGui();

		//Present
		{

			// Transition back to present
			D3D12_RESOURCE_BARRIER rb = {};
			rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			rb.Transition.pResource = currentBackBuffer.Get();
			rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			commandList->ResourceBarrier(1, &rb);

			// Must occur BEFORE present
			dx12HelperInst.CloseAndExecuteCommandList();
			// Present the current back buffer
			swapChain->Present(vsync ? 1 : 0, 0);
			//// Figure out which buffer is next
			//currentSwapBuffer++;
			//if (currentSwapBuffer >= numBackBuffers)
			//	currentSwapBuffer = 0;
			currentSwapBuffer = dx12HelperInst.SyncSwapChain(currentSwapBuffer);
		}
	}

}

void SDFRenderer::createTriangleForScreenQuad()
{
	float vertex_data_array[] = {
   0.0f,  0.5f,  0.0f, // point at top
   0.5f, -0.5f,  0.0f, // point at bottom-right
  -0.5f, -0.5f,  0.0f, // point at bottom-left
	};
	UINT vertex_stride = 3 * sizeof(float);
	DX12Helper& dx12Helper = DX12Helper::GetInstance();
	vertexBuffer = dx12Helper.CreateStaticBuffer(vertex_stride, 3, &vertex_data_array[0]);
	vbView.StrideInBytes = vertex_stride;
	vbView.SizeInBytes = vertex_stride * 3;
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();

}

void SDFRenderer::CreateRootSigAndPipelineState()
{
	//since we are friend of DXCore just call the path functions from Game
	// Blobs to hold raw shader byte code used in several steps below
	//Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	//Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;
	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(Utils::GetFullPathTo_Wide(L"RaymarchVS.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(Utils::GetFullPathTo_Wide(L"RaymarchPS.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}
	// Input layout
	const unsigned int inputElementCount = 1;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "SV_VertexID"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first semantic of this type

	}
	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 4;		// Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister = 0;	// Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};

		// CBV table param for vertex shader
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;

		// CBV table param for vertex shader
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;

		// SRV table param
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;

		// Create a single static sampler (available to all pixel shaders at the same slot)
		// Note: This is in lieu of having materials have their own samplers for this demo
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0;  // register(s0)
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((char*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}
	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();
		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();
		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;
		// Create the pipe state object
		device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf()));

	}
}