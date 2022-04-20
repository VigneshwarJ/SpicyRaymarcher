#include "Material.h"
#include "DX12Helper.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline, DirectX::XMFLOAT4 colorTint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset)
{
	this->pipelineState = pipeline;
	this->colorTint = colorTint;
	this->uvScale = uvScale;
	this->uvOffset = uvOffset;
	finalized = false;
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
	textureSRVsBySlot[slot] = srv;
	highestSRVIndex = slot; //im not sure if this is a good way to set this?
	//should make sure not finalized here? maybe?
}

void Material::FinalizeMaterial()
{
	if (finalized) return; //dont do anything if this is already finalized

	DX12Helper& dx12Helper = DX12Helper::GetInstance();
	dx12Helper.CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle();
}
