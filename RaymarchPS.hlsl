
#include "DistanceFunctions.hlsli"

#define MAX_PRIMITIVES 50
#define START_BOXES MAX_PRIMITIVES
#define MAX_COUNT 128

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
	int torusCount;
	int roundBoxCount;
	int cylinderCount;
	int coneCount;
	SDFPrimitive spherePrims[MAX_COUNT];
	SDFPrimitive boxPrims[MAX_COUNT];
	SDFPrimitive torusPrims[MAX_COUNT];
	Material material[MAX_COUNT];
	float time;
	float anim;
	int materialCount;
	float padding;
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
	float2 finalDistance = float2(plane(marcherPosition - float3(0,-1,0)),0);
	//find the distance of the scene at this pixel

	//float2 finalDistance=(10000.0f,1);
	float t = frac(time);
	t = t * (1.0 - t);
	for (int i = 0; i < sphereCount; i++)
	{
		
		float3 DeltaPosition = spherePrims[i].DeltaPosition * spherePrims[i].speed * t;
		finalDistance = basicUnionWithColor(finalDistance,
			float2(sphere(marcherPosition - spherePrims[i].Position - DeltaPosition, spherePrims[i].Radius), spherePrims[i].MaterialType));
	}

	for (int i = 0; i < boxCount; i++)
	{
		
	
		float3 DeltaPosition = boxPrims[i].DeltaPosition * boxPrims[i].speed * t;
		finalDistance = basicUnionWithColor(finalDistance,
			float2(box(marcherPosition - boxPrims[i].Position - boxPrims[i].DeltaPosition, boxPrims[i].Dimensions), boxPrims[i].MaterialType));
	
	}

	for (int i = 0; i < torusCount; i++)
	{
		float3 DeltaPosition = torusPrims[i].DeltaPosition * torusPrims[i].speed * t;
		finalDistance = basicUnionWithColor(finalDistance,
			float2(
				torus(
					marcherPosition - torusPrims[i].Position - DeltaPosition,
					torusPrims[i].Radius, 
					torusPrims[i].smallRadius
				), 
				torusPrims[i].MaterialType));

	}
	return finalDistance;
}

Surface mapSmooth(float3 marcherPosition)
{
	Surface finalDistance = surface( material[0], plane(marcherPosition - float3(0, -1, 0)));
	//find the distance of the scene at this pixel

	//float2 finalDistance=(10000.0f,1);
	float t = frac(time);
	t = t * (1.0 - t);
	for (int i = 0; i < sphereCount; i++)
	{

		float3 DeltaPosition = spherePrims[i].DeltaPosition * spherePrims[i].speed * t;
		finalDistance = SmoothUnion(finalDistance,
			surface( material[spherePrims[i].MaterialType],
			sphere(marcherPosition - spherePrims[i].Position - DeltaPosition, spherePrims[i].Radius)), spherePrims[i].smoothStep);
	}

	for (int i = 0; i < boxCount; i++)
	{


		float3 DeltaPosition = boxPrims[i].DeltaPosition * boxPrims[i].speed * t;
		finalDistance = SmoothUnion(finalDistance,
			surface( 
				material[boxPrims[i].MaterialType],
			box(marcherPosition - boxPrims[i].Position - boxPrims[i].DeltaPosition, boxPrims[i].Dimensions)), boxPrims[i].smoothStep);

	}

	for (int i = 0; i < torusCount; i++)
	{
		float3 DeltaPosition = torusPrims[i].DeltaPosition * torusPrims[i].speed * t;
		finalDistance = SmoothUnion(finalDistance,
			surface(
				material[torusPrims[i].MaterialType],
			torus(
				marcherPosition - torusPrims[i].Position - DeltaPosition,
				torusPrims[i].Radius,
				torusPrims[i].smallRadius
			)), torusPrims[i].smoothStep);

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
		float h = mapSmooth(ro + rd * t).signedDistance;
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
	return normalize(e.xyy * mapSmooth(pos + e.xyy).signedDistance +
		e.yyx * mapSmooth(pos + e.yyx).signedDistance +
		e.yxy * mapSmooth(pos + e.yxy).signedDistance +
		e.xxx * mapSmooth(pos + e.xxx).signedDistance);
  
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
Surface castRay(float3 rayOrigin, float3 rayDirection)
{
	float tmin = .50;
	float tmax = 20.0;

	float finalDistance = tmin;
	Material m;
	float visib = -1.0;
	for (int i = 0; i < 150; i++)
	{
		float precis = 0.0005 * finalDistance;
		//float2 res = map(rayOrigin + rayDirection * finalDistance);
		Surface res = mapSmooth(rayOrigin + rayDirection * finalDistance);
		//if (res.x<precis || finalDistance>tmax) break;
		if (res.signedDistance<precis || finalDistance>tmax) break;
		finalDistance += res.signedDistance;
		m = res.material;
		visib = 1.0;
	}
	//if (m == 30) return (finalDistance, 30);
	Surface surf = surface(m, finalDistance);
	surf.m = (finalDistance > tmax) ? -1 : visib;
	
	return surf;
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

	float3 skyColor = float3(0.4, 0.75, 1.0) - rayDirection.y * 1.2;
	skyColor = lerp(skyColor, float3(0.6, 0.75, .8), exp(-9.0 * rayDirection.y));
	float4 finalcolor = float4(skyColor, 1.0);

	////The cuttoff point at which we say "hit something!"
	float rmHitDistance = 0.1; //this should be made dynamic or a parameter somehow
	int maxSteps = 50;
	//float distances[primitivesCount];
	float3 marcherPosition = cameraPosition; //start marching at the camera position
	
	Surface finalDistance = castRay(marcherPosition, rayDirection);

	marcherPosition = cameraPosition + finalDistance.signedDistance * rayDirection;
	if (finalDistance.m > -1.0)
	{
		float3 normal = calculateNormals(marcherPosition);
		float3 diffuseColor = finalDistance.material.diffuseColor;

		float3 ref = reflect(rayDirection, normal);

		// lighting        
		float occ = calculateAO(marcherPosition, normal); // ambient occlusion
		float3 lig = normalize(lightPosition- marcherPosition); // sunlight
		//float3 lig = calculateLighting(marcherPosition, normal); // sunlight
		float amb = clamp(0.5 + 0.5 * normal.y, 0.0, 1.0); // ambient light
		float dif = clamp(dot(normal, lig), 0.0, 10.0); // diffuse reflection from sunlight
		float dom = clamp(dot(normal, float3(0,1,0)), 0.0, 10.0);
		// backlight
		float bac = clamp(dot(normal, normalize(float3(lig.x, 0.0, lig.z))), 0.0, 1.0) * clamp(1.0 - marcherPosition.y, 0.0, 1.0);
		//float dom = smoothstep(-0.1, 0.1, ref.y); // dome light
		float fre = pow(clamp(1.0 + dot(normal, rayDirection), 0.0, 1.0), 2.0); // fresnel
		float spe = pow(clamp(dot(ref, lig), 0.0, 1.0), finalDistance.material.shininess); // specular reflection

		dif *= calculateSoftshadow(marcherPosition, lig, 0.02, 2.5);
		dom *= calculateSoftshadow(marcherPosition, ref, 0.02, 2.5);

		float3 lin = float3(0.0,0.0,0.0);
		lin += 1 * dif * float3(1.00, 1.0, 1.0);
		lin += 0.40 * amb * float3(0.40, 0.60, 1.00) * occ;
		lin += 0.50 * dom * float3(0.40, 0.60, 1.00) * occ;
		lin += 0.50 * bac * float3(0.25, 0.25, 0.25) * occ;
		lin += 0.25 * fre * float3(1.00, 1.00, 1.00) * occ;
		diffuseColor = diffuseColor * lin;
		// gamma
		diffuseColor = pow(diffuseColor, float3(0.4545,0.4545,.4545));

		finalcolor = float4(diffuseColor, 1.0);
	}
	return finalcolor;
}
