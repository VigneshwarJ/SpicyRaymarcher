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
	DirectX::XMFLOAT3A cameraPosition;
	DirectX::XMFLOAT3A cameraForward;
	DirectX::XMFLOAT3A cameraRight;
	DirectX::XMFLOAT3A cameraUp;
	DirectX::XMFLOAT3A bgColor;
	DirectX::XMFLOAT3A lightPosition;
	DirectX::XMFLOAT3A spherePosition;
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