#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <memory>
//#include "WICTextureLoader.h"
#include "d3d12.h"

class Material
{
public:
	Material(
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline,
		DirectX::XMFLOAT4 colorTint,
		DirectX::XMFLOAT2 uvScale,
		DirectX::XMFLOAT2 uvOffset
	);

	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot);
	void FinalizeMaterial();

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	D3D12_GPU_DESCRIPTOR_HANDLE GetFinalGPUHandleForTextures();
	//Material(
	//	DirectX::XMFLOAT4 tint,
	//	std::shared_ptr<SimplePixelShader> pixShader,
	//	std::shared_ptr<SimpleVertexShader> vertShader,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, //todo: organize this better
	//	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV

	//);
	//Material(
	//	DirectX::XMFLOAT4 tint,
	//	std::shared_ptr<SimplePixelShader> pixShader,
	//	std::shared_ptr<SimpleVertexShader> vertShader,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV, //todo: organize this better
	//	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normSRV,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV,
	//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV

	//);
	//~Material();

	//DirectX::XMFLOAT4 GetColorTint();
	//std::shared_ptr<SimplePixelShader> GetPixelShader();
	//std::shared_ptr<SimpleVertexShader> GetVertexShader();
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureSRV();
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetNormalSRV();
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetRoughnessSRV();
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetMetalnessSRV();
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState();
	//float GetSpecularExponent();


private:

	//void LoadTexture();

	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	bool finalized;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState; //replaces individual vertex and pixel shaders
	//Microsoft::WRL::ComPtr<D3D12_CPU_DESCRIPTOR_HANDLE> textureSRVsBySlot[4]; //doing 4 slots instead of individual srvs for prb things
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[128]; //using All of Slots
	int highestSRVIndex; //the highest index actually used in the above textureSRVsBySlot array
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture2;
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;


	float specularExponent;
};

