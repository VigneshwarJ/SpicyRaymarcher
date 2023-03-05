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

	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float4x4 viewMatrix     : VIEW_MAT;
};

// Texture-related variables
Texture2D AlbedoTexture			: register(t0);
Texture2D MetalTexture			: register(t1);
Texture2D NormalTexture			: register(t2);
Texture2D RoughnessTexture		: register(t3);


SamplerState BasicSampler		: register(s0);
float4x4 inverse(float4x4 m) {
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

float3 getRayDirection(float2 screenPosition, float4x4 view) {
    float4 position = float4(screenPosition, 1.0, 1.0);
    position = position * float4(2.0, -2.0, 1.0, 1.0);
    position = mul(inverse(view), position);
    return normalize(position.xyz);
}

//float intersectSphere(float3 rayOrigin, float3 rayDirection, float3 spherePosition, float sphereRadius) {
//    float3 L = spherePosition - rayOrigin;
//    float tca = dot(L, rayDirection);
//    if (tca < 0.0) return -1.0;
//    float d2 = dot(L, L) - tca * tca;
//    if (d2 > sphereRadius * sphereRadius) return -1.0;
//    float thc = sqrt(sphereRadius * sphereRadius - d2);
//    return tca - thc;
//}
//
//float3 calculateNormal(float3 position, float3 spherePosition) {
//    return normalize(position - spherePosition);
//}
//
//float3 calculateLighting(float3 position, float3 normal, float3 lightPosition) {
//    float3 toLight = normalize(lightPosition - position);
//    float diffuse = max(0.0, dot(normal, toLight));
//    float3 ambient = 0.1;
//    float3 diffuseColor = 1.0;
//    return diffuseColor;
//}
//
//float4 main(VertexToPixel input) : SV_Target{
//    float3 lightPosition = float3(0.0f, 4.0f, 4.0f);
//    float3 OBJECT_POSITION = float3(0.0f, 0.0f, -10.0f);
//    float2 screenPosition = input.position.xy;
//    float3 rayDirection = getRayDirection(screenPosition,input.viewMatrix);
//    float3 rayOrigin = input.viewMatrix[3].xyz;
//    float sphereRadius = 0.0050f;
//    float t = intersectSphere(rayOrigin, rayDirection, OBJECT_POSITION, sphereRadius);
//    if (t < 0.0) {
//        return float4(0.0, 0.0, 0.0, 1.0);
//    }
// else {
//  float3 position = rayOrigin + t * rayDirection;
//  float3 normal = calculateNormal(position, OBJECT_POSITION);
//  float3 lighting = calculateLighting(position, normal, lightPosition);
//  return float4(lighting, 1.0);
//}
//}


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
    float3 pos = input.position; //WORLD_POSITION;
    float3 normal = 0.0; //default to zero

    //variables that are temporarily going to be hardcoded for now
    int maxSteps = 300;
    float rmStop = 0.1f; //i have absolutely no idea what a good number is here
    float3 OBJECT_POSITION = float3(5.0f, 1.0f, 0.0f); //cameraPosition + input.camLookDir * 10;
    //float EPSILON = 0.0001f;
    //float3 CAMERA_VECTOR = input.camLookDir;
    //    float2 screenposition = input.position.xy;
//    float3 raydirection = getraydirection(screenposition,input.viewmatrix);
//    float3 rayorigin = input.viewmatrix[3].xyz;
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
        pos += input.viewMatrix[3].xyz * distance;

    }

    //return float4(normal, Col.a);
    return Col;

}