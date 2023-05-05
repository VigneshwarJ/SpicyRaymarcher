#pragma once
#include <DirectXMath.h>
#define MAX_SDF_COUNT 128

// Be mindful of padding

// Primitive types
// As with light types in Lights.h, must match definitions in shader
#define SDF_TYPE_SPHERE	0
#define SDF_TYPE_BOX	1


// REMEMBER ALIGNMENT MATTERS HERE TOO, since this ends up in the arrays that go to the shader, it must FILL the last boundary
struct SDFPrimRenderData //todo: low priority right now, but it may be good to have some sort of error checking or assertion of some kind, IE warn user when they try to assign a radius to type box
{
	DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);  // 16 bytes
	float Radius = 1.0f; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)

	DirectX::XMFLOAT3 Dimensions = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);  // 32 bytes
	int MaterialType=0;

	float SmallRadius = 1.0f; //for torus, Radius will be the entire torus radius and smallRadius will be the thickness

	DirectX::XMFLOAT3 DeltaPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	float smoothStep = 0;
	float RotationRadius = 0;
	float speed = 4;
	float buffer;
};

struct SDFMaterial
{
	DirectX::XMFLOAT3 diffuseColor{ 1.0, 1.0, 1.0 };
	float shininess;
};
