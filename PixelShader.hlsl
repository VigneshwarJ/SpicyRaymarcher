#include "Lighting.hlsli"

// How many lights could we handle?
#define MAX_LIGHTS 128

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
	//return RoughnessTexture.Sample(BasicSampler, input.uv).rgba;

	//return lights[0].Color.xyzz;
	// Always re-normalize interpolated direction vectors
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	//sampling the other textures
	input.normal = NormalMapping(NormalTexture, BasicSampler, input.uv, input.normal, input.tangent);
	float roughness = RoughnessTexture.Sample(BasicSampler, input.uv).r;
	float metal = MetalTexture.Sample(BasicSampler, input.uv).r;

	//sampling the actual albedo
	float4 surfaceColor = AlbedoTexture.Sample(BasicSampler, input.uv);
	surfaceColor.rgb = pow(surfaceColor.rgb, 2.2);// * colorTint.rgb; //apply gamma correction
	//return surfaceColor.xyzz;

		// Specular color - Assuming albedo texture is actually holding specular color if metal == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we want lerp the specular color to match
	float3 specColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metal);

	// Total color for this pixel
	float3 totalColor = float3(0, 0, 0);

	// Loop through all lights this frame
	for (int i = 0; i < lightCount; i++)
	{
		// Which kind of light?
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			totalColor += DirLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;

		case LIGHT_TYPE_POINT:
			totalColor += PointLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;

		case LIGHT_TYPE_SPOT:
			totalColor += SpotLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;
		}
	}



	//return surfaceColor.xyzz;
	return totalColor.xyzz;

}