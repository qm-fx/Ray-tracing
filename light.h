#pragma once

#include "helper_math.h"

__device__  struct  Light
{
	unsigned int _id;
	float3 pos;
	float3 color;
	float intensity = 1.f;	
};