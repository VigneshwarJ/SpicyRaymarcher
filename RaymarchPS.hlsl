


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

#define MAX_PRIMITIVES 50
#define START_BOXES MAX_PRIMITIVES
#define MAX_COUNT 128
struct SDFPrimitive
{


	float3 Position;  // 16 bytes
    float Size; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)
	
	float3 Dimensions;  // 32 bytes
	int MaterialType;

};

struct Material
{
	float4 color;
};

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
	SDFPrimitive primitives[MAX_COUNT];
	Material color[MAX_COUNT];
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

// union primitives 1 and 2
// d1 is a vec2 where .x is the distance, and .y is the color/material code.
float2 opU(float2 d1, float2 d2)
{
	return (d1.x < d2.x) ? d1 : d2;
}


float findDistance()
{
	////find the distance of the scene at this pixel
	//for (int i = 0; i < primitiveCount; i++)
	//{
	//	float thisPrimDistance = 0.0f;

	//	switch (primitives[i].Type)
	//	{
	//	case SDF_TYPE_SPHERE:
	//		thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);
	//		break;
	//	case SDF_TYPE_BOX:
	//		thisPrimDistance = box(marcherPosition - primitives[i].Position, primitives[i].Dimensions);
	//		break;
	//	default:
	//		thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);
	//		break;
	//	}

	//	////thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);


	//	finalDistance = basicUnion(finalDistance, thisPrimDistance);
	//	//if the final distance is equal to the primitive distance, then this was prim the closest element to the camera in this pixel path
	//	if (finalDistance = thisPrimDistance)
	//	{
	//		normal = calculateNormal(marcherPosition, primitives[i].Position);
	//		//normal = float3(0.0f, 0.0f, 0.0f);
	//		diffuseColor = primitives[i].Color.xyz;
	//	}
	//}
	
}

float3 getRayDirection(float2 screenPosition) {
	float3 direction = screenPosition.x * cameraRight + screenPosition.y * cameraUp + 1.0f * cameraForward;
	return normalize(direction);
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

//// https://iquilezles.org/articles/rmshadows
//float calcSoftshadow(in float3 ro, in float3 rd, in float mint, in float tmax)
//{
//	// bounding volume
//	float tp = (0.8 - ro.y) / rd.y; if (tp > 0.0) tmax = min(tmax, tp);
//
//	float res = 1.0;
//	float t = mint;
//	for (int i = 0; i < 24; i++)
//	{
//		float h = map(ro + rd * t).x;
//		float s = clamp(8.0 * h / t, 0.0, 1.0);
//		res = min(res, s);
//		t += clamp(h, 0.01, 0.2);
//		if (res<0.004 || t>tmax) break;
//	}
//	res = clamp(res, 0.0, 1.0);
//	return res * res * (3.0 - 2.0 * res);
//}
//
//// https://iquilezles.org/articles/normalsSDF
//float3 calcNormal(in float3 pos)
//{
//#if 0
//	vec2 e = vec2(1.0, -1.0) * 0.5773 * 0.0005;
//	return normalize(e.xyy * map(pos + e.xyy).x +
//		e.yyx * map(pos + e.yyx).x +
//		e.yxy * map(pos + e.yxy).x +
//		e.xxx * map(pos + e.xxx).x);
//#else
//	// inspired by tdhooper and klems - a way to prevent the compiler from inlining map() 4 times
//	float3 n = float3(0.0);
//	for (int i = ZERO; i < 4; i++)
//	{
//		float3 e = 0.5773 * (2.0 * float3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0);
//		n += e * map(pos + 0.0005 * e).x;
//		//if( n.x+n.y+n.z>100.0 ) break;
//	}
//	return normalize(n);
//#endif    
//}
//
//// https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
//float calcAO(in float3 pos, in float3 nor)
//{
//	float occ = 0.0;
//	float sca = 1.0;
//	for (int i = ZERO; i < 5; i++)
//	{
//		float h = 0.01 + 0.12 * float(i) / 4.0;
//		float d = map(pos + h * nor).x;
//		occ += (h - d) * sca;
//		sca *= 0.95;
//		if (occ > 0.35) break;
//	}
//	return clamp(1.0 - 3.0 * occ, 0.0, 1.0) * (0.5 + 0.5 * nor.y);
//}
















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
		for (int i = 0; i < sphereCount; i++)
		{
			float thisPrimDistance = 0.0f;

				thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Size);
				
		       finalDistance = basicUnion(finalDistance, thisPrimDistance);
			   //if the final distance is equal to the primitive distance, then this was prim the closest element to the camera in this pixel path
			   if (finalDistance = thisPrimDistance)
			   {
				   normal = calculateNormal(marcherPosition, primitives[i].Position);
				   //normal = float3(0.0f, 0.0f, 0.0f);
				   diffuseColor = color[primitives[i].MaterialType].color.xyz;
			   }
		}

			////thisPrimDistance = sphere(marcherPosition - primitives[i].Position, primitives[i].Radius);



		for (int i = MAX_PRIMITIVES; i < boxCount; i++)
		{
			float thisPrimDistance = 0.0f;

			thisPrimDistance = box(marcherPosition - primitives[i].Position, primitives[i].Dimensions);
			finalDistance = basicUnion(finalDistance, thisPrimDistance);
			//if the final distance is equal to the primitive distance, then this was prim the closest element to the camera in this pixel path
			if (finalDistance = thisPrimDistance)
			{
				//normal = calculateNormal(marcherPosition, primitives[i].Position);
				normal = float3(0.0f, 0.0f, 0.0f);
				diffuseColor = color[primitives[i].MaterialType].color.xyz;
			}
		}
			//if the final distance is equal to the primitive distance, then this was prim the closest element to the camera in this pixel path
			
		

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
			finalcolor = float4(ambient + diffuseColor * diffuse, 1.0);
			//finalcolor = float4(normal.xyz, 0.0f);
			break;
		}
	}

	return finalcolor;
}
