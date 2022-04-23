#include "Material.h"
#include "DX12Helper.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline, DirectX::XMFLOAT4 colorTint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset)
{
	this->pipelineState = pipeline;
	this->colorTint = colorTint;
	this->uvScale = uvScale;
	this->uvOffset = uvOffset;
	finalized = false;
	highestSRVIndex = -1;

	finalGPUHandleForSRVs = {};
	ZeroMemory(textureSRVsBySlot, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * 128);
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
	// Valid slot?
	if (finalized || slot < 0 || slot >= 128)
		return;

	// Save and check if this was the highest slot
	textureSRVsBySlot[slot] = srv;
	highestSRVIndex = max(highestSRVIndex, slot);
}

void Material::FinalizeMaterial()
{
	if (finalized) return; //dont do anything if this is already finalized

	DX12Helper& dx12Helper = DX12Helper::GetInstance();

	for (int i = 0; i < highestSRVIndex + 1; i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = dx12Helper.CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[i], 1);
		
		if (i == 0) //we just want the first one, to point to the first descriptor in the final heap
		{
			finalGPUHandleForSRVs = handle;
		}
	}

	finalized = true;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pipelineState;
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForTextures()
{
	return finalGPUHandleForSRVs;
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}
