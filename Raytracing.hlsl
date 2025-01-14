//----STRUCTS----

struct Vertex //this struct same as VertexShaderInput struct in regular vertex shader
{
	float3 localPosition	: POSITION;     // XYZ position
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;

};
static const uint VertexSizeInBytes = 11 * 4; //alignment thing for above struct. 3+2+3+3 = 11, and a float is 4 bytes

struct RayPayload //the data sent along with each ray
{
	float4 material; //should be as small as possible
};

// Note: We'll be using the built-in BuiltInTriangleIntersectionAttributes struct
// for triangle attributes, so no need to define our own.  It contains a single float2.



//----CONSTANT BUFFERS----

cbuffer SceneData : register(b0)
{
	matrix inverseViewProjection;
	float3 cameraPosition;
	float pad0;
};

cbuffer ObjectData : register(b1)
{
	float3 colorTint;
	float pad1;
};

//----RESOURCES----
// Output UAV 
RWTexture2D<float4> OutputColor				: register(u0);

// The actual scene we want to trace through (Top Level Acceleration Structure)
RaytracingAccelerationStructure SceneTLAS : register(t0);

// Geometry buffers
ByteAddressBuffer IndexBuffer        		: register(t1);
ByteAddressBuffer VertexBuffer				: register(t2);

// Texture-related variables
TextureCube skyTexture		: register(t3);
//Texture2D AlbedoTexture			: register(t3);
//Texture2D MetalTexture			: register(t4);
//Texture2D NormalTexture			: register(t5);
//Texture2D RoughnessTexture		: register(t6);


SamplerState BasicSampler		: register(s0);

//----HELPERS----

// Loads the indices of the specified triangle from the index buffer
uint3 LoadIndices(uint triangleIndex)
{
	// What is the start index of this triangle's indices?
	uint indicesStart = triangleIndex * 3;

	// Adjust by the byte size before loading
	return IndexBuffer.Load3(indicesStart * 4); // 4 bytes per index
}

// Barycentric interpolation of data from the triangle's vertices
//
Vertex InterpolateVertices(uint triangleIndex, float3 barycentricData)
{
	// Grab the indices
	uint3 indices = LoadIndices(triangleIndex);

	// Set up the final vertex
	Vertex vert;
	vert.localPosition = float3(0, 0, 0);
	vert.uv = float2(0, 0);
	vert.normal = float3(0, 0, 0);
	vert.tangent = float3(0, 0, 0);

	// Loop through the barycentric data and interpolate
	for (uint i = 0; i < 3; i++)
	{
		// Get the index of the first piece of data for this vertex
		uint dataIndex = indices[i] * VertexSizeInBytes;

		// Grab the position and offset
		vert.localPosition += asfloat(VertexBuffer.Load3(dataIndex)) * barycentricData[i];
		dataIndex += 3 * 4; // 3 floats * 4 bytes per float

		// UV
		vert.uv += asfloat(VertexBuffer.Load2(dataIndex)) * barycentricData[i];
		dataIndex += 2 * 4; // 2 floats * 4 bytes per float

		// Normal
		vert.normal += asfloat(VertexBuffer.Load3(dataIndex)) * barycentricData[i];
		dataIndex += 3 * 4; // 3 floats * 4 bytes per float

		// Tangent (no offset at the end, since we start over after looping)
		vert.tangent += asfloat(VertexBuffer.Load3(dataIndex)) * barycentricData[i];
	}

	// Final interpolated vertex data is ready
	return vert;
}


//find the direction the ray should go from the camera
void CalcRayFromCamera(uint2 rayIndices, out float3 origin, out float3 direction)
{
	// Offset to the middle of the pixel
	float2 pixel = rayIndices + 0.5f;
	float2 screenPos = pixel / DispatchRaysDimensions().xy * 2.0f - 1.0f;
	screenPos.y = -screenPos.y;

	// Unproject the coords
	float4 worldPos = mul(inverseViewProjection, float4(screenPos, 0, 1));
	worldPos.xyz /= worldPos.w;

	// Set up the outputs
	origin = cameraPosition.xyz;
	direction = normalize(worldPos.xyz - origin);
}


//----SHADERS----

// Ray generation shader - Launched once for each ray we want to generate
// (which is generally once per pixel of our output texture)
[shader("raygeneration")]
void RayGen()
{
	// Get the ray indices
	uint2 rayIndices = DispatchRaysIndex().xy;

	// Calculate the ray data
	float3 rayOrigin;
	float3 rayDirection;
	CalcRayFromCamera(rayIndices, rayOrigin, rayDirection);

	// Set up final ray description
	RayDesc ray;
	ray.Origin = rayOrigin;
	ray.Direction = rayDirection;
	ray.TMin = 0.01f;
	ray.TMax = 1000.0f;

	// Set up the payload for the ray
	RayPayload payload;
	payload.material = float4(0, 0, 0, 1);

	// Perform the ray trace for this ray
	TraceRay(
		SceneTLAS,
		RAY_FLAG_NONE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload);

	// Set the final color of the buffer
	OutputColor[rayIndices] = payload.material;
}


// Miss shader - What happens if the ray doesn't hit anything?
[shader("miss")]
void Miss(inout RayPayload payload)
{
	// Nothing was hit, so return black for now.
	// Ideally this is where we would do skybox stuff!
	payload.material = float4(0, 0, 0, 1);
}


// Closest hit shader - Runs the first time a ray hits anything
[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes hitAttributes)
{
	// Grab the index of the triangle we hit
	uint triangleIndex = PrimitiveIndex();

	// Calculate the barycentric data for vertex interpolation
	float3 barycentricData = float3(
		1.0f - hitAttributes.barycentrics.x - hitAttributes.barycentrics.y,
		hitAttributes.barycentrics.x,
		hitAttributes.barycentrics.y);

	// Get the interpolated vertex data
	Vertex interpolatedVert = InterpolateVertices(triangleIndex, barycentricData);

	//----
	//float4 surfaceColor = AlbedoTexture.Sample(BasicSampler, interpolatedVert.uv);
	//surfaceColor.rgb = pow(surfaceColor.rgb, 2.2);


	//----

	// Use the resulting data to set the final color
	// Note: Here is where we would do actual shading!
	payload.material = float4(interpolatedVert.normal, 1);
}