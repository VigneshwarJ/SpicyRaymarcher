#pragma once
#include <DirectXMath.h>
#define MAX_SDF_COUNT 128

// Be mindful of padding

struct SDFSphere 
{
	//....
    // TODO: Figure out whether this works
	float  sphereRadius;
	DirectX::XMFLOAT3 spherePosition;  // 16 bytes

	DirectX::XMFLOAT4 sphereColor;
	//....


};

// TODO: Add more shapes