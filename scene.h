#pragma once
#include <vector>
#include "model_obj.h"

extern std::vector<Triangle> triangle_vector;
extern std::vector<material> material_vector;

void TriangleScene()
{
    //×ó ºìÉ«Ç½±Ú
    material red{ make_float3(0.6f, 0.2f, 0.2f),0.2f};
    material_vector.push_back(red);
    Triangle tri = {0,
        {  {-3.f, -3.f,  5.f},
        {-3.f, -3.f, -5.f},
        {-3.f,  3.f, -5.f} },
        { 1,0,0 }};
    triangle_vector.push_back(tri);
    tri = {0,
        { {-3.f, -3.f,  5.f},
        {-3.f,  3.f, -5.f},
        {-3.f,  3.f,  5.f} },
        { 1,0,0 } };
    triangle_vector.push_back(tri);

    //ÓÒ À¶É«Ç½±Ú
    material blue{ make_float3(0.2f, 0.2f, 0.6f),0.2f};
    material_vector.push_back(blue);
    tri = {1,
        { { 3.f, -3.f,  5.f},
        { 3.f, -3.f, -5.f},
        { 3.f,  3.f, -5.f} },
        { -1,0,0 }};
    triangle_vector.push_back(tri);
    tri = {1,
        { { 3.f, -3.f,  5.f},
        { 3.f,  3.f, -5.f},
        { 3.f,  3.f,  5.f} },
        { -1,0,0 }};
    triangle_vector.push_back(tri);

    //»Ò°×É«Ç½±Ú
    material white{ make_float3(0.7f, 0.7f, 0.7f),0.2f};
    material_vector.push_back(white);
    material white_top{ make_float3(0.7f, 0.7f, 0.7f),0.95f };
    material_vector.push_back(white_top);

    //ÏÂ
    tri = {2,
        { {-3.f, -3.f,  5.f},
        { 3.f, -3.f,  5.f},
        { 3.f, -3.f, -5.f} },
        { 0,1,0 }};
    triangle_vector.push_back(tri);
    tri = {2,
        { {-3.f, -3.f,  5.f},
        { 3.f, -3.f, -5.f},
        {-3.f, -3.f, -5.f} },
        { 0,1,0 }};
    triangle_vector.push_back(tri);

    //ÉÏ
    tri = {3,
        { {-3.f, 3.f,  5.f},
        { 3.f, 3.f,  5.f},
        { 3.f, 3.f, -5.f} },
        { 0,-1,0 } };
    triangle_vector.push_back(tri);
    tri = {3,
        {  {-3.f, 3.f,  5.f},
        { 3.f, 3.f, -5.f},
        {-3.f, 3.f, -5.f} },
        { 0,-1,0 }};
    triangle_vector.push_back(tri);

    //Ç°

    tri = {2,
        { {-3.f, -3.f,  5.f},
        { 3.f, -3.f,  5.f},
        { 3.f,  3.f,  5.f} },
        { 0,0,-1 }};
    triangle_vector.push_back(tri);
    tri = {2,
        { {-3.f, -3.f,  5.f},
        { 3.f,  3.f,  5.f},
        {-3.f,  3.f,  5.f} },
        { 0,0,-1 }};
    triangle_vector.push_back(tri);

    //ºó
    tri = {2,
        { { 3.f, -3.f, -5.f},
        {-3.f, -3.f, -5.f},
        {-3.f,  3.f, -5.f} },
        { 0,0,1 }};
    triangle_vector.push_back(tri);
    tri = {2,
        { { 3.f, -3.f, -5.f},
    {-3.f,  3.f, -5.f},
    { 3.f,  3.f, -5.f} },
        { 0,0,1 }};
    triangle_vector.push_back(tri);
}

void InitScence()
{
    TriangleScene();
    std::string str("E:/VS_2017/RayTracing_3/love-poly-1.obj");
    material glass{ make_float3(0.8f, 0.8f,0.8f ),1.5f};
    material_vector.push_back(glass);
    LoadFile(str, 4);
    unsigned int size = triangle_vector.size();

    triangle_vector.shrink_to_fit();
    material_vector.shrink_to_fit();
}

