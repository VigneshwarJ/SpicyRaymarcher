struct SDFPrimitive
{


	float3 Position;  // 16 bytes
	float Radius; //this could be used for things other than just spheres, hence the name (but maybe will need to be renamed later if we add something like torus?)

	float3 Dimensions;  // 32 bytes
	int MaterialType;

    float smallRadius; //for torus, Radius will be the entire torus radius and smallRadius will be the thickness


    float3 DeltaPosition;

    float smoothStep;
    float RotationRadius;
    float speed;
    float buffer;

};

struct Material
{
    float3 diffuseColor;
    float shininess;
};

struct Surface
{
    Material material;
    float signedDistance;
    float m;
    //float3 padding;
    
};

Surface surface(Material mat, float d)
{
    Surface surf;
    surf.material = mat;
    surf.signedDistance = d;
    return surf;
}

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

float torus(float3 position, float largeR, float smallR)
{
	float2 q = float2(length(position.xz) - largeR, position.y);
	return length(q) - smallR;
}

float roundBox(float3 p, float3 b, float r)
{
    float3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

float boxFrame(float3 p, float3 b, float e)
{
    p = abs(p) - b;
    float3 q = abs(p + e) - e;
    return min(min(
        length(max(float3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
        length(max(float3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(float3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float cappedTorus(in float3 p, in float2 sc, in float ra, in float rb)
{
    p.x = abs(p.x);
    float k = (sc.y * p.x > sc.x * p.y) ? dot(p.xy, sc) : length(p.xy);
    return sqrt(dot(p, p) + ra * ra - 2.0 * ra * k) - rb;
}
//
//float sdLink(float3 p, float le, float r1, float r2)
//{
//    float3 q = float3(p.x, max(abs(p.y) - le, 0.0), p.z);
//    return length(float2(length(q.xy) - r1, q.z)) - r2;
//}

float cylinder(float3 p, float3 c)
{
    return length(p.xz - c.xy) - c.z;
}

float cone(in float3 p, in float2 c, float h)
{
    // c is the sin/cos of the angle, h is height
    // Alternatively pass q instead of (c,h),
    // which is the point at the base in 2D
    float2 q = h * float2(c.x / c.y, -1.0);

    float2 w = float2(length(p.xz), p.y);
    float2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0, 1.0);
    float2 b = w - q * float2(clamp(w.x / q.x, 0.0, 1.0), 1.0);
    float k = sign(q.y);
    float d = min(dot(a, a), dot(b, b));
    float s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    return sqrt(d) * sign(s);
}

float plane(float3 p)
{
    return p.y;
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

float smoothUnion(float distance1, float distance2, float smoothFactor)
{
    //d1 and d2 are distance function 1 and 2
    float h = clamp(0.5 + 0.5 * (distance2 - distance1) / smoothFactor, 0.0, 1.0);
    return lerp(distance2, distance1, h) - smoothFactor * h * (1.0 - h);
}

Surface SmoothUnion(Surface surface1, Surface surface2, in float smoothness) {
    float interpolation = clamp(0.5 + 0.5 * (surface2.signedDistance - surface1.signedDistance) / smoothness, 0.0, 1.0);
    float3 material = lerp(surface2.material.diffuseColor, surface1.material.diffuseColor, interpolation);
    float shininess = lerp(surface2.material.shininess, surface1.material.shininess, interpolation);
    Material mat;
    mat.diffuseColor = material;
    mat.shininess = shininess;
    return surface(mat,
        lerp(surface2.signedDistance, surface1.signedDistance, interpolation) - smoothness * interpolation * (1.0 - interpolation));
}


// Surface shader (uses the Phong illumination model):
//vec3 shadeSurface(in Surface surface, in Ray ray, in vec3 normal) {
//    vec3 illuminationAmbient = surface.ambientColor * lightColor;
//    float lambertian = max(0.0, dot(normal, lightDirection));
//    vec3 illuminationDiffuse = lambertian * surface.diffuseColor * lightColor;
//    vec3 reflection = reflect(lightDirection, normal);
//    float specularAngle = max(0.0, dot(reflection, ray.direction));
//    vec3 illuminationSpecular = clamp(pow(specularAngle, surface.shininess), 0.0, 1.0) * surface.specularColor * lightColor;
//    return illuminationAmbient + illuminationDiffuse + illuminationSpecular;
//}


