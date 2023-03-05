#include "Lighting.hlsli"

// How many lights could we handle?
//#define MAX_LIGHTS 128

// Alignment matters!!!
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;
	float3 cameraPosition;
	int lightCount;
	Light lights[MAX_LIGHTS];
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
	float4 screenPosition	: SV_POSITION;
	float3 worldPosition	: POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float3 camLookDir		: POSITION1;
};

// Texture-related variables
Texture2D AlbedoTexture			: register(t0);
Texture2D MetalTexture			: register(t1);
Texture2D NormalTexture			: register(t2);
Texture2D RoughnessTexture		: register(t3);


SamplerState BasicSampler		: register(s0);

//// Raymarching functions - probably should move this to an include file later
//struct SignedDistanceFunctions
//{
//	// primitives sourced from: https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//	// yes, these variable names are the absolute worst. I would fix them but i dont even know what they mean
//	float Sphere(float3 pos, float radius)
//	{
//		return length(pos) - radius;
//	}
//}
	// primitives sourced from: https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
	// yes, these variable names are the absolute worst. I would fix them but i dont even know what they mean
float Sphere(float3 pos, float radius)
{
	return length(pos) - radius;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(0.0f, 0.5f, 0.5f, 1.0f);

	//SignedDistanceFunctions rm;
	// ActionStruct actStruct;
	float4 Col = float4(0.0f, 0.0f, 1.0f, 1.0f);
	float3 pos = input.worldPosition; //WORLD_POSITION;
	float3 normal = 0.0; //default to zero

	//variables that are temporarily going to be hardcoded for now
	int maxSteps = 300;
	float rmStop = 0.1f; //i have absolutely no idea what a good number is here
	float3 OBJECT_POSITION = float3(0.0f, 0.0f, 0.0f); //cameraPosition + input.camLookDir * 10;
	//float EPSILON = 0.0001f;
	//float3 CAMERA_VECTOR = input.camLookDir;

	for (int i = 0; i < maxSteps; i++)
	{
		//int shapesCount = 4;
		//float distances[4];
		//bool shouldSmooth = false;
		//distances[0] = Sphere(pos - OBJECT_POSITION, 100.0);

		float distance = Sphere(pos - OBJECT_POSITION, 100.0);// distances[0]; //start this with the first one


		if (distance < rmStop) //hit!
		{
			Col = float4(1.0f, 0.0f, 0.0f, 1.0f);
			//normal = rm.RMNormal(pos - OBJECT_POSITION);
			break;
		}

		//This where we march the ray forward
		pos += input.camLookDir * distance;

	}

	//return float4(normal, Col.a);
	return Col;

}