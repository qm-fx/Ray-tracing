#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <vector_functions.h>
#include <cuda_runtime_api.h>
#include <stdio.h>
#include <iostream>
#include <cmath> 
#include <chrono>
#include <vector_types.h>
#include < curand_kernel.h >
#include "bmp_header.h"
#include "Triangle.h"
#include "material.h"
#include "BVH.h"
#include "scene.h"
#include "light.h"

std::vector<BVHNode> BVH_tree;
std::vector<Triangle> triangle_vector;
std::vector<material> material_vector;

__device__ BVHNode* d_BVH_tree;
__device__ Triangle* d_triangle_vector;
__device__ material* d_material_vector;

struct HitTriangleRet
{
    float3 normal;
    unsigned int material_id;
};


__device__ void IntersectPlaneBox(const Ray ray, HitTriangleRet& hit_triangle, float& t_geo)
{
    for (int i = 0; i < 12; i++)
    {
        float3 pos_0 = d_triangle_vector[i].pos[0],
            pos_1 = d_triangle_vector[i].pos[1],
            pos_2 = d_triangle_vector[i].pos[2];

        float tempT = FLT_MAX;
        bool flag = HitTriangle(pos_0, pos_1, pos_2, ray, tempT);
        if (flag && tempT < t_geo)
        {
            t_geo = tempT;
            hit_triangle.material_id = d_triangle_vector[i].material_id;
            hit_triangle.normal = d_triangle_vector[i].normal;
        }
    }
}

__device__ bool IntersectBVH(const Ray ray, HitTriangleRet& hit_triangle, float& t_geo)
{
    bool ret = false;
    unsigned int node_stack[40];              // 显式栈（支持深度≤64）
    unsigned int node_ptr = 0;
    node_stack[node_ptr++] = 0;       // 压入根节点（索引0）             
    int leaf_stack[32];
    int leaf_ptr = 0;

    while (node_ptr > 0) {
        int node_idx = node_stack[node_ptr - 1];
        node_ptr--;
        BVHNode* temp_node = &d_BVH_tree[node_idx];
        bool flag = BVHNodeIntersect(temp_node, ray);
        if (!flag)
            continue;

        //读取后31位掩码的实际值
        unsigned int value = temp_node->left & DATA_MASK;

        //根据最高位掩码判断是否为叶子节点
        flag = temp_node->left & LEAF_MASK;
        if (flag)
        {
            // 叶子节点：与图元求交               
            leaf_stack[leaf_ptr++] = temp_node->right;
            continue;
        }

        // 内部节点：压入子节点（先左后右，实现深度优先）
        else
        {
            node_stack[node_ptr++] = value;
            node_stack[node_ptr++] = temp_node->right;
        }
    }

    while (leaf_ptr > 0)
    {
        int Triangle_idx = leaf_stack[leaf_ptr - 1];
        leaf_ptr--;

        float3 pos_0 = d_triangle_vector[Triangle_idx].pos[0],
            pos_1 = d_triangle_vector[Triangle_idx].pos[1],
            pos_2 = d_triangle_vector[Triangle_idx].pos[2];

        float tempT = FLT_MAX;
        bool flag = HitTriangle(pos_0, pos_1, pos_2, ray, tempT);
        if (flag && tempT < t_geo)
        {
            t_geo = tempT;
            hit_triangle.material_id = d_triangle_vector[Triangle_idx].material_id;
            hit_triangle.normal = d_triangle_vector[Triangle_idx].normal;
            ret = true;
        }

    }
    return ret;

}

__device__ float3 render(const float3 hit_point, const Light light, const material _material, const float3 _normal)
{
    float3 color = { 0.f, 0.f, 0.f };

    float3 lightDir = normalize(light.pos - hit_point);

    //漫反射
    float diffuse = max(0.0f, dot(_normal, lightDir));

    float3 lightContribution = light.color * light.intensity;

    //叠加颜色
    color = color + _material.diffuse * diffuse* lightContribution
                  + _material.diffuse * _material.ior * lightContribution;

    color *= 255;

    return color;
}

__device__ float3 refract() 
{

}

__device__ float3 refract(float3 ray_dir, float3 normal, 
    float air_ior,float model_ior)
{
    float dot_in = dot(ray_dir, normal);
    normal = dot_in > 0 ? - normal: normal;
    float in_ior = dot_in > 0 ? model_ior : air_ior;
    float out_ior = dot_in > 0 ? air_ior : model_ior;
    float eta = in_ior / out_ior;
    float cos_i = abs(dot_in);
    float sin_i_2 = (1.0 - cos_i * cos_i);
    float sin_t_2 = eta * eta * sin_i_2;
    if (sin_t_2 > 1.f)
        return {0.f,0.f,0.f};
    float cos_t = sqrt(1.f - sin_t_2);
    float3 ret = eta * ray_dir + (eta * cos_i - cos_t) * normal;
    ret = normalize(ret);
    return ret;
}

__device__ bool shadow(float3& hit_point, float3 light_pos)
{

}

__device__ float halton(int n, int base) 
{
    float result = 0.f;
    float f = 1.f;
    int i = n;

    while (i > 0) 
{
        f /= base;                  
        result += f * (i % base);    
        i /= base;                 
    }

    return result;
}

// Beer-Lambert计算材质1单位距离能量吸收律
__device__ float3 BeerLambert(float3 material_Tr) 
{
    return { -log(material_Tr.x),-log(material_Tr.y),-log(material_Tr.z) };
}

//Schlick菲涅尔近似
__device__ float fresnelSchlick(float3 ray_dir, float3 light_dir,float dot_in,
                        float air_ior, float model_ior)
{
    float in_ior = dot_in > 0 ? model_ior : air_ior;
    float out_ior = dot_in > 0 ? air_ior : model_ior;
    float3 half = normalize(light_dir + ray_dir);
    float cos_i = dot_in;
    float ratio = in_ior/out_ior;   
    float sin_t_2 = 1.f - cos_i * cos_i;
    sin_t_2 = ratio * ratio * sin_t_2;
    float cos_t = sqrt(1.f - sin_t_2);
    float F_0 = (in_ior-out_ior)/(out_ior+in_ior);
    F_0 = F_0*F_0;
    float ret =  F_0 + (1.f - F_0) * pow(1.f - cos_t, 5);
    ret = sin_t_2 > 1.f  ? 1.f:ret;
    return ret;
}

// 光线追踪核函数
__global__ void renderKernel(unsigned int seed,float3* ray_tracing, int width, int height,int spp_size)
{
    int k_w = 16 / spp_size, k_h = 16 / spp_size;
    int simple_idx = threadIdx.y;
    int x_offest = threadIdx.x % 2;
    int y_offest = threadIdx.x / 2;
    int x = blockIdx.x * k_w + x_offest;
    int y = blockIdx.y * k_h + y_offest;
    bool valid = (x < width&& y < height);
    int tracing_idx = (y * width + x)* spp_size + simple_idx;
    
    float3 color = { 0,0,0 };
    float halton_2 = halton(simple_idx, 2);
    float halton_3 = halton(simple_idx, 3);
    curandState state;
    curand_init(seed, x + y + simple_idx, 0, &state);

    // 生成相机光线 (正交投影)
    //float scale = tan(60.0f * 0.5f * 3.1415926f / 180.0f);
    float u = (2.f * (x + halton_2) / width - 1.f) * 0.6 /* scale*/;
    float v = (1.f - 2.f * (y + halton_3) / height) * 0.6 /** scale*/;

    float3 ray_pos = make_float3(0.f, 0.5f, 4.5f);
    float3 ray_dir = normalize(make_float3(u, v, -1.0f)); // 指向-Z方向
    Ray ray{ ray_pos, ray_dir };
    float3 ray_energy = { 1.f,1.f,1.f };

    Light light = {0, { 0.f, 2.98f, 0.f }, { 1.f,1.f,1.f },1.f };
    int depth = 0;
    float distance = 0.f;
    bool distance_flag = false;

    //循环光线追踪
    while (depth < 8 )
    {
        HitTriangleRet hit_triangle = { {0,0,0},1 };
        float t_geo = FLT_MAX;
        bool hit_flag = IntersectBVH(ray, hit_triangle, t_geo);
        if(!hit_flag)
            IntersectPlaneBox(ray, hit_triangle, t_geo);
        
        float3 hit_point = ray.origin + ray.direction * t_geo;
        material _material = d_material_vector[hit_triangle.material_id];
        float3 r_color;

        if (!hit_flag)
        {
            r_color = render(hit_point, light, _material, hit_triangle.normal);
            r_color.x = clamp(r_color.x, 0.0f, 255.0f);
            r_color.y = clamp(r_color.y, 0.0f, 255.0f);
            r_color.z = clamp(r_color.z, 0.0f, 255.0f);
            //bool _shadow = shadow(hit_point, light.pos);
            //float c_shadow = _shadow ? 0.1f : 1.f;
            r_color *= ray_energy /* * c_shadow*/;
            color += r_color;
            break;
        }

        if(distance_flag)
        {
            float3 d = hit_point - ray.origin;
            distance += sqrtf(d.x*d.x + d.y*d.y + d.z*d.z);
            float3 sigma_a = BeerLambert(_material.diffuse);
            float3 transmittance = { exp(-sigma_a.x * distance),exp(-sigma_a.y * distance),exp(-sigma_a.z * distance) };
            ray_energy *= transmittance;
        }

        float3 light_dir = normalize(light.pos - hit_point);
        float dot_in = dot(ray.direction, hit_triangle.normal);

        float k_reflect = fresnelSchlick(ray.direction, light_dir, dot_in,1.f,_material.ior );
        float k_refract = 1.f - k_reflect;

        float3 offest = hit_triangle.normal * 0.0001f;
        float3 energy_reflect = ray_energy * k_reflect;
        float3 reflect_point = hit_point + offest;
        float3 reflect_dir = ray.direction - 2 * dot_in * hit_triangle.normal;
        reflect_dir = normalize(reflect_dir);
        
        float3 energy_refract = ray_energy* k_refract;
        int channel = simple_idx % 3;
        float current_ior = _material.ior + channel * 0.05f;
        float3 refract_dir = refract(ray.direction, hit_triangle.normal, 
            1.f, current_ior);
        float3 refract_point = dot_in > 0.f ?
            hit_point + offest : hit_point - offest;

        bool reflag = halton_2 < k_reflect;
        ray_dir = reflag ? reflect_dir : refract_dir;
        ray_pos = reflag ? reflect_point : refract_point;
        ray = { ray_pos ,ray_dir };
        ray_energy = reflag ? energy_reflect : energy_refract;  
        distance_flag = reflag ? false : true;

        if(depth >= 4)
        {
            float xi = curand_uniform(&state);
            bool end_flag = xi < 0.5f;
            depth = end_flag ? depth = 20 : depth;
            ray_energy = end_flag ? ray_energy:2.f * ray_energy;
        }
        depth++;
    }
 
    // 写入像素
    
    if (valid == true)
        ray_tracing[tracing_idx] = color; 
}

__global__ void pixelsKernel(uchar3* pixels, float3* ray_tracing, int width, int height, int spp_size)
{
    int x = blockIdx.x;
    int y = blockIdx.y;
    
    int pixels_idx = (height - 1 - y) * width + (width - x - 1);
    int tracing_idx = (y * width + x) * spp_size;
    bool valid = (x < width&& y < height);
    float3 color = {0.f,0.f,0.f};
    float _spp = 1.f / (float)spp_size;
    for(int i = 0;i< spp_size;i++)
    {
        float3 r_color = ray_tracing[tracing_idx + i];
        r_color *= _spp;
        color += r_color;
    }

    pixels[pixels_idx] = make_uchar3(color.x, color.y, color.z);
    
}

int main() {
    auto build_start = std::chrono::high_resolution_clock::now();

    // 图像参数
    const int width = 1024, height = 1024;
    const int spp_size = 8;
    int pixels_size = 32 / spp_size;
    int k_w = 16 / spp_size, k_h = 16 / spp_size;
    dim3 blocks(width/k_w,height/k_h);
    dim3 threads(pixels_size, spp_size);

    // 初始化场景
    InitScence();

    //构建BVH
    buildBVHTree();

    // 分配设备内存
    uchar3* d_pixels;
    cudaMalloc(&d_pixels, width * height * sizeof(uchar3));

    float3* d_ray_tracing;
    cudaMalloc(&d_ray_tracing, width * height *spp_size* sizeof(float3));

    BVHNode* d_temp_BVH = nullptr;
    cudaMalloc(&d_temp_BVH, BVH_tree.size() * sizeof(BVHNode));
    cudaMemcpy(d_temp_BVH, BVH_tree.data(), BVH_tree.size() * sizeof(BVHNode), cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(d_BVH_tree, &d_temp_BVH, sizeof(BVHNode*));

    Triangle* d_temp_triangle = nullptr;
    cudaMalloc(&d_temp_triangle, triangle_vector.size() * sizeof(Triangle));
    cudaMemcpy(d_temp_triangle, triangle_vector.data(), triangle_vector.size() * sizeof(Triangle), cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(d_triangle_vector, &d_temp_triangle, sizeof(Triangle*));

    material* d_temp_material = nullptr;
    cudaMalloc(&d_temp_material, material_vector.size() * sizeof(material));
    cudaMemcpy(d_temp_material, material_vector.data(), material_vector.size() * sizeof(material), cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(d_material_vector, &d_temp_material, sizeof(material*));

    auto build_end = std::chrono::high_resolution_clock::now();
    auto bulid_time = std::chrono::duration_cast<std::chrono::milliseconds>(build_end - build_start).count();
    std::cout << bulid_time << "    ";

    auto render_start = std::chrono::high_resolution_clock::now();
    // 渲染
    renderKernel << <blocks, threads >> > (time(nullptr),d_ray_tracing, width, height,spp_size);

    cudaError_t syncErr = cudaDeviceSynchronize();
    // 验证点1：强制同步并检查错误
    if (syncErr != cudaSuccess) {
        printf("Kernel execution error: %s\n", cudaGetErrorString(syncErr));
        exit(1);
    }
    // 验证点2：检查核函数是否完成
    cudaError_t kernelErr = cudaGetLastError();
    if (kernelErr != cudaSuccess) {
        printf("Kernel launch error: %s\n", cudaGetErrorString(kernelErr));
        exit(1);
    }

    dim3 threads_pixels(1);
    dim3 pixels(width,height);
    pixelsKernel << <pixels, threads_pixels >> > (d_pixels,d_ray_tracing, width, height, spp_size);

    auto render_end = std::chrono::high_resolution_clock::now();
    auto render_time = std::chrono::duration_cast<std::chrono::milliseconds>(render_end - render_start).count();
    std::cout << render_time << "    ";

    uchar3* h_pixels = new uchar3[width * height];
    cudaMemcpy(h_pixels, d_pixels, width * height * sizeof(uchar3), cudaMemcpyDeviceToHost); // 必须拷贝回主机

    saveBMP("out.bmp", h_pixels, width, height);
    // 回传结果 (保存为图像)

    // 清理
    d_BVH_tree = nullptr;
    d_triangle_vector = nullptr;
    d_material_vector = nullptr;
    cudaFree(d_pixels);
    cudaFree(d_ray_tracing);
    cudaFree(d_temp_BVH);
    cudaFree(d_temp_triangle);
    cudaFree(d_temp_material);

    delete[] h_pixels;

    return 0;
}

