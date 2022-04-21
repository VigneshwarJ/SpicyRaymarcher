#include "Lighting.hlsli"

// How many lights could we handle?
#define MAX_LIGHTS 128

// Alignment matters!!!
cbuffer ExternalData : register(b0)
{
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
};

// Texture-related variables
Texture2D AlbedoTexture			: register(t0);
Texture2D MetalTexture			: register(t1);
Texture2D NormalTexture			: register(t2);
Texture2D RoughnessTexture		: register(t3);


SamplerState BasicSampler		: register(s0);

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
	float4 surfaceColor = AlbedoTexture.Sample(BasicSampler, input.uv);
	return surfaceColor;


}