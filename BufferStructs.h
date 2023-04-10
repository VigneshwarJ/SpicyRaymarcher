#pragma once
#include <DirectXMath.h>
#include "Lights.h"
#include "SDFShapes.h"
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
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 cameraPosition;
	int lightCount;
	Light lights[MAX_LIGHTS];

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
	//UINT id;

};
struct RaymarchPSExternalData
{
	// TODO : To seperate Camera and Light into different c_buffers
	DirectX::XMFLOAT3A cameraPosition;
	DirectX::XMFLOAT3A cameraForward;
	DirectX::XMFLOAT3A cameraRight;
	DirectX::XMFLOAT3A cameraUp;
	DirectX::XMFLOAT3A bgColor;
	DirectX::XMFLOAT3 lightPosition;
	//int sphereCount;
	//SDFSphere spheres[MAX_SDF_COUNT];
	int primitiveCount;
	SDFPrimitive primitives[MAX_SDF_COUNT];

};