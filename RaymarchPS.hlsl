


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

#define SDF_TYPE_SPHERE	0
#define SDF_TYPE_BOX	1

struct SDFPrimitive
{

	int Type;
	float3 Position;  // 16 bytes

	float Radius; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)
	float3 Dimensions;  // 32 bytes

	float4 Color;


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
	int primitiveCount;
	SDFPrimitive primitives[128];
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

float sphere(float3 position, float radius)
{
	return length(position) - radius;
}

//this formula worked as is in unreal, its *extremely* unlikely that the math here is wrong
float box(float3 position, float3 bDimensions)
{
	float3 q = abs(position) - bDimensions;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

//by finding the closest distance between the two points, we can join two primitives together
float basicUnion(float distance1, float distance2)
{
	return min(distance1, distance2);
}

float findDistance()
{

}

float3 getRayDirection(float2 screenPosition) {
	float3 direction = screenPosition.x * cameraRight + screenPosition.y * cameraUp + 1.0f * cameraForward;
	return normalize(direction);
}

//wat
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

	//The cuttoff point at which we say "hit something!"
	float rmHitDistance = 1.0;
	int maxSteps = 500;
	//float distances[primitivesCount];
	float finalDistance = 10000.0f;
	float3 marcherPosition = cameraPosition; //start marching at the camera position


	for (int i = 0; i < maxSteps; i++)
	{
		float3 normal;
		float3 diffuseColor;
		//find the distance of the scene at this pixel
		for (int i = 0; i < primitiveCount; i++)
		{
			float thisPrimDistance = 0.0f;

			switch (primitives[i].Type)
			{
			case SDF_TYPE_SPHERE:
				thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);
				break;
			case SDF_TYPE_BOX:
				thisPrimDistance = box(marcherPosition - primitives[i].Position, primitives[i].Dimensions);
				break;
			default:
				thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);
				break;
			}

			////thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);


			finalDistance = basicUnion(finalDistance, thisPrimDistance);
			//if the final distance is equal to the primitive distance, then this was prim the closest element to the camera in this pixel path
			if (finalDistance = thisPrimDistance)
			{
				//normal = calculateNormal(marcherPosition, primitives[i].Position);
				normal = float3(0.0f, 0.0f, 0.0f);
				diffuseColor = primitives[i].Color.xyz;
			}
		}

		//march the ray forward
		marcherPosition += rayDirection * finalDistance;

		//check current distance against the stop distance
		if (finalDistance < rmHitDistance)
		{
			float3 position = marcherPosition;//cameraPosition + finalDistance * rayDirection;
			//float3 normal = calculateNormal(position, primitives[i].Position);
			float diffuse = calculateLighting(position, normal);
			float3 ambient = 0.1;
			//float3 diffuseColor = primitives[i].Color.xyz;
			finalcolor = float4(ambient + diffuseColor * diffuse, primitives[i].Color.w);
			//finalcolor = float4(normal.xyz, 0.0f);
			break;
		}
	}

	//for (int i = 0; i < primitiveCount; i++)
	//{
	//	float t = intersectSphere(cameraPosition, rayDirection, primitives[i].Position, primitives[i].Radius);
	//	//float t = intersectSphere(cameraPosition, rayDirection, primitives[i].Position, primitives[i].Radius);

	//	if (t < 0.0) {
	//		continue;
	//	}
	//	else {
	//		float3 position = cameraPosition + t * rayDirection;
	//		float3 normal = calculateNormal(position, primitives[i].Position);
	//		float diffuse = calculateLighting(position, normal);
	//		float3 ambient = 0.1;
	//		float3 diffuseColor = primitives[i].Color.xyz;
	//		finalcolor = float4(ambient + diffuseColor * diffuse, primitives[i].Color.w);
	//	}
	//}
	return finalcolor;
}
