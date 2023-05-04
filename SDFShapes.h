#pragma once
#include <DirectXMath.h>
#define MAX_SDF_COUNT 128

// Be mindful of padding

// Primitive types
// As with light types in Lights.h, must match definitions in shader
#define SDF_TYPE_SPHERE	0
#define SDF_TYPE_BOX	1

//struct SDFSphere 
//{
//	//....
//    // TODO: Figure out whether this works
//	float  sphereRadius;
//	DirectX::XMFLOAT3 spherePosition;  // 16 bytes
//
//	DirectX::XMFLOAT4 sphereColor;
//	//....
//
//
//};

// REMEMBER ALIGNMENT MATTERS HERE TOO, since this ends up in the arrays that go to the shader, it must FILL the last boundary
struct SDFPrimRenderData //todo: low priority right now, but it may be good to have some sort of error checking or assertion of some kind, IE warn user when they try to assign a radius to type box
{
	DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);  // 16 bytes
	float Radius = 1.0f; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)

	DirectX::XMFLOAT3 Dimensions = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);  // 32 bytes
	int MaterialType;

	float SmallRadius = 1.0f; //for torus, Radius will be the entire torus radius and smallRadius will be the thickness

	DirectX::XMFLOAT3 padding;
};

struct SDFMaterial
{
	DirectX::XMFLOAT4 color;
};


// TODO: Add more shapes