cbuffer perFrame : register (b0)
{
	matrix world;
	matrix worldInverseTranspose;
	matrix view;
	matrix projection;
};
//
//
//// Struct representing a single vertex worth of data
//// - This should match the vertex definition in our C++ code
//// - By "match", I mean the size, order and number of members
//// - The name of the struct itself is unimportant, but should be descriptive
//// - Each variable must have a semantic, which defines its usage
//struct VertexShaderInput
//{
//	// Data type
//	//  |
//	//  |   Name          Semantic
//	//  |    |                |
//	//  v    v                v
//	float3 localPosition	: POSITION;     // XYZ position
//	float2 uv				: TEXCOORD;
//	float3 normal			: NORMAL;
//	float3 tangent			: TANGENT;
//
//};
//
//// Struct representing the data we're sending down the pipeline
//// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
//// - At a minimum, we need a piece of data defined tagged as SV_POSITION
//// - The name of the struct itself is unimportant, but should be descriptive
//// - Each variable must have a semantic, which defines its usage
//struct VertexToPixel
//{
//	// Data type
//	//  |
//	//  |   Name          Semantic
//	//  |    |                |
//	//  v    v                v
//	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
//	float3 worldPosition	: POSITION;
//	float2 uv				: TEXCOORD;
//	float3 normal			: NORMAL;
//	float3 tangent			: TANGENT;
//};
//
//// --------------------------------------------------------
//// The entry point (main method) for our vertex shader
//// 
//// - Input is exactly one vertex worth of data (defined by a struct)
//// - Output is a single struct of data to pass down the pipeline
//// - Named "main" because that's the default the shader compiler looks for
//// --------------------------------------------------------
//VertexToPixel main(VertexShaderInput input)
//{
//	// Set up output struct
//	VertexToPixel output;
//
//
//	// Calculate output position
//	matrix worldViewProj = mul(projection, mul(view, world));
//	output.screenPosition = mul(worldViewProj, float4(input.localPosition, 1.0f));
//
//	//// Calculate the world position of this vertex (to be used
//	//// in the pixel shader when we do point/spot lights)
//	output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;
//
//	//// Make sure the normal is in WORLD space, not "local" space
//	output.normal = normalize(mul((float3x3)worldInverseTranspose, input.normal));
//	output.tangent = normalize(mul((float3x3)world, input.tangent));
//
//	//// Pass through the uv
//	output.uv = input.uv; //* uvScale;
//
//	// Whatever we return will make its way through the pipeline to the
//	// next programmable stage we're using (the pixel shader for now)
//	return output;
//}

// Defines the output data of our vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float4x4 viewMatrix     : VIEW_MAT;
};


// This vertex shader receives one piece of input: an id (ideally from 0 - 2, inclusive)
// 
// From those values (0, 1 or 2), it creates uv coords and a screen position.
// If done correctly, the triangle that arises from those three vertices will
// perfectly fill the screen, like so:
//  ________________
// |- - - - - - - - |- - - - - - - - uv = (2, 0)
// |- uv = (0,0)    |           -
// |-               |        -
// |-               |     -
// |-               | -
// |________________|
//  -           -
//  -       -
//  -   -
//  -
//  uv = (0, 2)
//

VertexToPixel main(uint id : SV_VertexID)
{
	// Set up output
	VertexToPixel output;

	// Calculate the UV (0,0) to (2,2) via the ID
	output.uv = float2(
		(id << 1) & 2,  // id % 2 * 2
		id & 2);

	// Adjust the position based on the UV
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	output.viewMatrix = view;

	return output;
}
