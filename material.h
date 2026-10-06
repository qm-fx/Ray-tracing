#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "Ray.h"

__device__ struct material {
    float3 diffuse;      //漫反射颜色        半透明物体能量吸收系数
    float ior;           //环境光颜色倍率    透射律          
};

#endif // MATERIAL_H