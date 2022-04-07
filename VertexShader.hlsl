cbuffer perFrame : register (b0)
{
	matrix world;
	matrix view;
	matrix projection;
};


// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{ 
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float2 uv				: TEXCOORD;     
	float3 normal			: NORMAL;   
	float3 tangent			: TANGENT;  

};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)

};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;


	// Calculate output position
	matrix worldViewProj = mul(projection, mul(view, world));
	output.screenPosition = mul(worldViewProj, float4(input.localPosition, 1.0f));

	//// Calculate the world position of this vertex (to be used
	//// in the pixel shader when we do point/spot lights)
	//output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;

	//// Make sure the normal is in WORLD space, not "local" space
	//output.normal = normalize(mul((float3x3)worldInverseTranspose, input.normal));
	//output.tangent = normalize(mul((float3x3)worldInverseTranspose, input.tangent));

	//// Pass through the uv
	//output.uv = input.uv * uvScale;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}