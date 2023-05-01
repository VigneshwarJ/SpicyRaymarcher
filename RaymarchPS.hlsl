


// How many lights could we handle?
//#define MAX_LIGHTS 128
//struct SDFSphere
//{
//	//....
//	// TODO: Figure out whether this works
//	float  sphereRadius;
//	float3 spherePosition;  // 16 bytes
//
//	float4 sphereColor;
//	//....
//
//
//};

#include "DistanceFunctions.hlsli"

#define MAX_PRIMITIVES 50
#define START_BOXES MAX_PRIMITIVES
#define MAX_COUNT 128

struct Material
{
	float4 color;
};
//struct SDFPrimitive
//{
//
//
//	float3 Position;  // 16 bytes
//    float Size; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)
//	
//	float3 Dimensions;  // 32 bytes
//	int MaterialType;
//
//};

// Alignment matters!!!
cbuffer ExternalData : register(b0)
{
	float3 cameraPosition;
	float3 cameraForward;
	float3 cameraRight;
	float3 cameraUp;
	float3 bgColor; // bg color not working
	int boxCount;
	float3 lightPosition;
	int sphereCount;
	SDFPrimitive spherePrims[MAX_COUNT];
	SDFPrimitive boxPrims[MAX_COUNT];
	Material material[MAX_COUNT];
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v

	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float3 ray           : TEXCOORD1;
};


float3 getRayDirection(float2 screenPosition) {
	float3 direction = screenPosition.x * cameraRight + screenPosition.y * cameraUp + 1.0f * cameraForward;
	return normalize(direction);
}

float calculateLighting(float3 position, float3 normal) {
	//float3 lightPosition = float3(0, 10, 0);
	float3 toLight = normalize(lightPosition - position);
	float diffuse = max(0.0, dot(normal, toLight));
	
	return diffuse ;
	//return  diffuseColor;
}

float2 map(float3 marcherPosition)
{
	//float2 finalDistance = basicUnionWithColor(float2(10000.0,1.0),
	//	float2(plane(marcherPosition - float3(0,-5,0)),30));
	//find the distance of the scene at this pixel

	float2 finalDistance=(10000.0f,1);
	for (int i = 0; i < sphereCount; i++)
	{
		finalDistance = basicUnionWithColor(finalDistance,
			float2(sphere(marcherPosition - spherePrims[i].Position, spherePrims[i].Size), spherePrims[i].MaterialType));
	}

	for (int i = 0; i < boxCount; i++)
	{
		finalDistance = basicUnionWithColor(finalDistance,
			float2(box(marcherPosition - boxPrims[i].Position, boxPrims[i].Dimensions), boxPrims[i].MaterialType));
	
	}
	return finalDistance;
}


// Cast a shadow ray from origin ro (an object surface) in direction rd
// to compute soft shadow in that direction. Returns a lower value
// (darker shadow) when there is more stuff nearby as we step along the shadow ray.
// https://iquilezles.org/articles/rmshadows
float calculateSoftshadow(float3 ro, float3 rd, float mint, float tmax)
{
	float res = 1.0;
	float t = mint;
	for (int i = 0; i < 16; i++)
	{
		float h = map(ro + rd * t).x;
		res = min(res, 8.0 * h / t);
		t += clamp(h, 0.02, 0.10);
		if (h<0.001 || t>tmax) break;
	}
	return clamp(res, 0.0, 1.0);
}

// Compute normal vector to surface at pos, using central differences method
// https://iquilezles.org/articles/normalsSDF
float3 calculateNormals(float3 pos)
{
	float2 e = float2(1.0, -1.0) * 0.5773 * 0.0005;
	return normalize(e.xyy * map(pos + e.xyy).x +
		e.yyx * map(pos + e.yyx).x +
		e.yxy * map(pos + e.yxy).x +
		e.xxx * map(pos + e.xxx).x);
  
}

// compute ambient occlusion value at given position/normal
// https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
float calculateAO(float3 pos, float3 nor)
{
	float occ = 0.0;
	float sca = 1.0;
	for (int i = 0; i < 5; i++)
	{
		float h = 0.01 + 0.12 * float(i) / 4.0;
		float d = map(pos + h * nor).x;
		occ += (h - d) * sca;
		sca *= 0.95;
		if (occ > 0.35) break;
	}
	return clamp(1.0 - 3.0 * occ, 0.0, 1.0) * (0.5 + 0.5 * nor.y);
}

// Cast a ray from origin ro in direction rd until it hits an object.
// Return (t,m) where t is distance traveled along the ray, and m
// is the material of the object hit.
float2 castRay(float3 rayOrigin, float3 rayDirection)
{
	float tmin = .50;
	float tmax = 20.0;

	float finalDistance = tmin;
	float m = -1.0;
	for (int i = 0; i < 150; i++)
	{
		float precis = 0.0005 * finalDistance;
		float2 res = map(rayOrigin + rayDirection * finalDistance);
		if (res.x<precis || finalDistance>tmax) break;
		finalDistance += res.x;
		m = res.y;
	}
	//if (m == 30) return (finalDistance, 30);
	if (finalDistance > tmax) m = -1.0;
	return float2(finalDistance, m);
}


float4 main(VertexToPixel input) : SV_Target
{

	float2 ps = float2(1280, 720);
	float2 ratio = float2(ps.x / ps.y, 1);
	input.uv *= ratio;
	float2 screenPosition = (input.uv) - 0.5;
	screenPosition.y =   0.5 - input.uv.y;
	float3 rayDirection = normalize(input.ray);
	 rayDirection = getRayDirection(screenPosition);
	 //material[30].color = float4(1, 0, .5, 1);

	float3 skyColor = float3(0.7, 0.9, 1.0) + rayDirection.y * 0.8;

	float4 finalcolor = float4(skyColor, 1.0);

	////The cuttoff point at which we say "hit something!"
	float rmHitDistance = 0.1; //this should be made dynamic or a parameter somehow
	int maxSteps = 50;
	//float distances[primitivesCount];
	float3 marcherPosition = cameraPosition; //start marching at the camera position
	
	float2 finalDistance = castRay(marcherPosition, rayDirection);

	marcherPosition = cameraPosition + finalDistance.x * rayDirection;
	if (finalDistance.y > -1)
	{
		float3 normal = calculateNormals(marcherPosition);
		float3 diffuseColor = material[finalDistance.y].color.xyz;

		float3 ref = reflect(rayDirection, normal);

		// lighting        
		float occ = calculateAO(marcherPosition, normal); // ambient occlusion
		float3 lig = normalize(lightPosition- marcherPosition); // sunlight
		//float3 lig = calculateLighting(marcherPosition, normal); // sunlight
		float amb = clamp(0.5 + 0.5 * normal.y, 0.0, 1.0); // ambient light
		float dif = clamp(dot(normal, lig), 0.0, 10.0); // diffuse reflection from sunlight

		// backlight
		float bac = clamp(dot(normal, normalize(float3(lig.x, 0.0, lig.z))), 0.0, 1.0) * clamp(1.0 - marcherPosition.y, 0.0, 1.0);
		float dom = smoothstep(-0.1, 0.1, ref.y); // dome light
		float fre = pow(clamp(1.0 + dot(normal, rayDirection), 0.0, 1.0), 2.0); // fresnel
		float spe = pow(clamp(dot(ref, lig), 0.0, 1.0), 16.0); // specular reflection

		dif *= calculateSoftshadow(marcherPosition, lig, 0.02, 2.5);
		dom *= calculateSoftshadow(marcherPosition, ref, 0.02, 2.5);

		float3 lin = float3(0.0,0.0,0.0);
		lin += 1 * dif * float3(1.00, 1.0, 1.0);
		lin += 0.40 * amb * float3(0.40, 0.60, 1.00) * occ;
		//lin += 0.50 * dom * float3(0.40, 0.60, 1.00) * occ;
		lin += 0.50 * bac * float3(0.25, 0.25, 0.25) * occ;
		//lin += 0.25 * fre * float3(1.00, 1.00, 1.00) * occ;
		diffuseColor = diffuseColor * lin;
		// gamma
		diffuseColor = pow(diffuseColor, float3(0.4545,0.4545,.4545));

		finalcolor = float4(diffuseColor, 1.0);
	}
	//finalcolor = float4(rayDirection, 1.0);
	return finalcolor;
}
