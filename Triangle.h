#pragma once
#include "Ray.h"

struct __align__(64)  Triangle
{
    unsigned int material_id;
    float3 pos[3];
    float3 normal;    
};

__device__ bool HitTriangle(const float3 pos_0, const float3 pos_1, const float3 pos_2, const Ray ray, float& t)
{
    float3 e1 = pos_1 - pos_0;
    float3 e2 = pos_2 - pos_0;
    float3 P = cross(ray.direction, e2);
    float det = dot(e1, P);

    bool flag1 = fabs(det) < 0.0000001f;
    if (flag1)
        return false; // 平行或退化

    float inv_det = 1.0f / det;
    float3 T = ray.origin - pos_0;
    float u = dot(T, P) * inv_det;
    flag1 = u < 0 || u > 1;
    if (flag1)
        return false;

    float3 Q = cross(T, e1);
    float v = dot(ray.direction, Q) * inv_det;
    flag1 = v < 0 || u + v > 1;
    if (flag1)
        return false;

    t = dot(e2, Q) * inv_det;
    return t >= 0;
}
