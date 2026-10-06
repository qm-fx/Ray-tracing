#pragma once
#include "Ray.h"

//BVH节点结构
struct __align__(32) BVHNode
{
    float3 _min; 
    unsigned int left = 0; 
    float3 _max;   
    unsigned int right = 0;
};

__device__  bool BVHNodeIntersect(const BVHNode* node,const Ray& ray)
{
    float x_0 = (node->_min.x - ray.origin.x) / ray.direction.x;
    float x_1 = (node->_max.x - ray.origin.x) / ray.direction.x;

    float x_min = fminf(x_0, x_1);
    float x_max = fmaxf(x_0, x_1);

    float y_0 = (node->_min.y - ray.origin.y) / ray.direction.y;
    float y_1 = (node->_max.y - ray.origin.y) / ray.direction.y;
    
    float y_min = fminf(y_0, y_1);
    float y_max = fmaxf(y_0, y_1);

    float z_0 = (node->_min.z - ray.origin.z) / ray.direction.z;
    float z_1 = (node->_max.z - ray.origin.z) / ray.direction.z;
    
    float z_min = fminf(z_0, z_1);
    float z_max = fmaxf(z_0, z_1);

    float t_enter = fmaxf(x_min, y_min);
    t_enter = fmaxf(t_enter, z_min);
    float t_exit = fminf(x_max, y_max);
    t_exit = fminf(t_exit, z_max);

    return (t_exit > t_enter && t_exit > 0);
}

