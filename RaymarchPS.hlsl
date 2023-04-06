
// How many lights could we handle?
//#define MAX_LIGHTS 128
struct SDFSphere
{
	//....
	// TODO: Figure out whether this works
	float  sphereRadius;
	float3 spherePosition;  // 16 bytes

	float4 sphereColor;
	//....


};
// Alignment matters!!!
cbuffer ExternalData : register(b0)
{
	float3 cameraPosition;
	float3 cameraForward;
	float3 cameraRight;
	float3 cameraUp;
	float3 bgColor; // bg color not working
	float3 lightPosition;
	int sphereCount;
	SDFSphere spheres[128];
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
};


/*******************************************************************/
//Clean working
//
//
//
float3 getRayDirection(float2 screenPosition) {
	float3 direction = screenPosition.x * cameraRight + screenPosition.y * cameraUp + 1.0f * cameraForward;
	return normalize(direction);
}

float intersectSphere(float3 rayOrigin, float3 rayDirection, float3 spherePosition, float sphereRadius) {
	float3 L = spherePosition - rayOrigin;
	float tca = dot(L, rayDirection);
	if (tca < 0.0) return -1.0;
	float d2 = dot(L, L) - tca * tca;
	if (d2 > sphereRadius * sphereRadius) return -1.0;
	float thc = sqrt(sphereRadius * sphereRadius - d2);
	return tca - thc;
}

float3 calculateNormal(float3 position, float3 spherePosition) {
	return normalize(position - spherePosition);
}

float calculateLighting(float3 position, float3 normal) {
	//float3 lightPosition = float3(0, 10, 0);
	float3 toLight = normalize(lightPosition - position);
	float diffuse = max(0.0, dot(normal, toLight));
	
	return diffuse ;
	//return  diffuseColor;
}

float4 main(VertexToPixel input) : SV_Target{
	//float3 spherePosition = float3(0.0f, 0.0f, 5.0f);
	//float sphereRadius = 5.0f;
	//float2 screenPosition = input.uv ;
	float2 ps = float2(1280, 720);
	float2 ratio = float2(ps.x / ps.y, 1);
	input.uv *= ratio;
	float2 screenPosition = (input.uv) - 0.5;

	float3 rayDirection = getRayDirection(screenPosition);

	float4 finalcolor = float4(bgColor, 1.0);
	for (int i = 0; i < sphereCount; i++)
	{
		float t = intersectSphere(cameraPosition, rayDirection, spheres[i].spherePosition, spheres[i].sphereRadius);
		if (t < 0.0) {
			continue;
		}
		else {
			float3 position = cameraPosition + t * rayDirection;
			float3 normal = calculateNormal(position, spheres[i].spherePosition);
			float diffuse = calculateLighting(position, normal);
			float3 ambient = 0.1;
			float3 diffuseColor = spheres[i].sphereColor.xyz;
			finalcolor = float4(ambient + diffuseColor * diffuse, spheres[i].sphereColor.w);
		}
	}
	return finalcolor;
}
