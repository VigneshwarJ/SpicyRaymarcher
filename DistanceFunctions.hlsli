struct SDFPrimitive
{


	float3 Position;  // 16 bytes
	float Size; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)

	float3 Dimensions;  // 32 bytes
	int MaterialType;

};

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

float2 basicUnionWithColor(float2 distance1, float2 distance2)
{
	return distance1.x < distance2.x ? distance1 : distance2;
}