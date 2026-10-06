#pragma once
#include "BVHNode.h"
#include "Triangle.h"
#include <algorithm>
#include <vector>
#include "helper_math.h"

constexpr unsigned int LEAF_MASK = 0x80000000u; // 1 << 31
constexpr unsigned int DATA_MASK = 0x7FFFFFFFu; // 低 31 位全 1

extern std::vector<BVHNode> BVH_tree;
extern std::vector<Triangle> triangle_vector;

struct TriBarycenter
{
    float3 _max, _min;
    float3 _barycenter;
    unsigned int _id;
};

void bounds(float3& max, float3& min,const float3 pos[3])
{
    min = pos[0];
    max = pos[1];

    // 比较三个顶点
    min.x = std::min({ pos[0].x, pos[1].x, pos[2].x });
    min.y = std::min({ pos[0].y, pos[1].y, pos[2].y });
    min.z = std::min({ pos[0].z, pos[1].z, pos[2].z });

    max.x = std::max({ pos[0].x, pos[1].x, pos[2].x });
    max.y = std::max({ pos[0].y, pos[1].y, pos[2].y });
    max.z = std::max({ pos[0].z, pos[1].z, pos[2].z });

    //坐标向外扩展一小段，避免退化为面
    min -= 1e-4f;
    max += 1e-4f;
}

unsigned int buildBVH(int start, int end,std::vector<TriBarycenter> &barycenter_list)
{
    float3 max = barycenter_list[start]._max, 
           min = barycenter_list[start]._min;
    for (int i = start + 1; i < end; i++)
    {
        float3 other_max = barycenter_list[i]._max, 
               other_min = barycenter_list[i]._min;

        min.x = std::min(min.x, other_min.x);
        min.y = std::min(min.y, other_min.y);
        min.z = std::min(min.z, other_min.z);
        max.x = std::max(max.x, other_max.x);
        max.y = std::max(max.y, other_max.y);
        max.z = std::max(max.z, other_max.z);
    }

    if (end - start == 1)
    {
        BVHNode leaf;
        leaf._min = min;
        leaf._max = max;
        leaf.left = 1;
        leaf.left |= LEAF_MASK;
        leaf.right = barycenter_list[start]._id;
        BVH_tree.push_back(leaf);
        return BVH_tree.size() - 1;
    }

    int axis = 0;

    float3 extent = max - min;  
    if (extent.x > extent.y && extent.x > extent.z)
        axis = 0;

    if (extent.y > extent.z)
        axis = 1;
    else
        axis = 2;

    auto comparator = [axis](TriBarycenter a, TriBarycenter b)
    {
        if (axis == 0) return a._barycenter.x < b._barycenter.x;
        if (axis == 1) return a._barycenter.y < b._barycenter.y;
        if (axis == 2) return a._barycenter.z < b._barycenter.z;
    };
    std::sort(barycenter_list.begin() + start, barycenter_list.begin() + end, comparator);

    int mid = start + (end - start) / 2;
    BVHNode node;
    node._min = min;
    node._max = max;
    unsigned int ret = BVH_tree.size();
    BVH_tree.push_back(node);
    node.left = buildBVH(start, mid, barycenter_list);
    node.right = buildBVH(mid, end, barycenter_list);    
    BVH_tree[ret] = node;
    return ret;
}

void buildBVHTree()
{
    std::vector<TriBarycenter> barycenter_list;
    for(auto it = triangle_vector.begin() + 12;it!= triangle_vector.end();it++)
    {
        float3 max, min;
        bounds(max,min, it->pos);
        float3 _bary = (it->pos[0] + it->pos[1] + it->pos[2]) / 3;
        TriBarycenter tri_bary{ max,min,_bary, it- triangle_vector .begin()};
        barycenter_list.push_back(tri_bary);
    }

    buildBVH(0, barycenter_list.size(), barycenter_list);
    BVH_tree.shrink_to_fit();
}





