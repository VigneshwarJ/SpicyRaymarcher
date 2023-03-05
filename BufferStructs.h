#pragma once
#include <DirectXMath.h>
#include "Lights.h"

//basic vertex and pixel shader data
struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct PixelShaderExternalData
{
	DirectX::XMFLOAT3 cameraPosition;
	DirectX::XMFLOAT3 cameraForward;
	DirectX::XMFLOAT3 cameraRight;
	DirectX::XMFLOAT3 cameraUp;
	DirectX::XMFLOAT3 bgColor;
	DirectX::XMFLOAT3 lightPosition;
	DirectX::XMFLOAT3 spherePosition;
	DirectX::XMFLOAT4 sphereColor;
	float  sphereRadius;
};

// Overall scene data for raytracing
struct RaytracingSceneData
{
	DirectX::XMFLOAT4X4 inverseViewProjection;
	DirectX::XMFLOAT3 cameraPosition;
	float pad;
};

//raymarching shader data
struct RaymarchVSExternalData
{
	UINT id;
	//DirectX::XMFLOAT4X4 world;
	//DirectX::XMFLOAT4X4 worldInverseTranspose;
	//DirectX::XMFLOAT4X4 view;
	//DirectX::XMFLOAT4X4 projection;
	//DirectX::XMFLOAT4X4 view;
	//DirectX::XMFLOAT4X4 projection;
	//DirectX::XMFLOAT4 position;
	//DirectX::XMFLOAT2 uv;
};
struct RaymarchPSExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 cameraPosition;
	int lightCount;
	Light lights[MAX_LIGHTS];
	//DirectX::XMFLOAT4 colorTint;
};