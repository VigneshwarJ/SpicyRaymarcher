// Include guard
#ifndef _LIGHTING_HLSL
#define _LIGHTING_HLSL

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
	int		Type;
	float3	Direction;	// 16 bytes

	float	Range;
	float3	Position;	// 32 bytes

	float	Intensity;
	float3	Color;		// 48 bytes

	float	SpotFalloff;
	float3	Padding;	// 64 bytes
};