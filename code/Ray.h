#pragma once
#include <vector_types.h>
#include "helper_math.h"

// 光线结构体
__device__ struct Ray {
    float3 origin;    // 起点
    float3 direction; // 方向（需归一化） 
};
