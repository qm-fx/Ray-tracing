#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "material.h"
#include "Triangle.h"

extern std::vector<Triangle> triangle_vector;

void LoadFile(std::string str, unsigned int material_id)
{
    std::vector<float3> pos_list;
    std::vector<float3> normal_list;
    std::ifstream in(str);      
    if (in.fail())    
    {     
        std::cout << "obj not found";          
        return;       
    }
    std::string line;
    std::string material_path;

    while (!in.eof())
    {
        char a;
        
        std::getline(in, line);
        std::istringstream ins(line.c_str());
        if (line[0] == 'v' && line[1] == ' ')
        {
            float3 v;
            ins >> a >> v.x >> v.y >> v.z;
            pos_list.push_back(v);
        }
        else if (line[0] == 'v' && line[1] == 'n')
        {
            float3 n;
            ins >> a >> a >> n.x >> n.y >> n.z;
            normal_list.push_back(n);
        }           
        else if (line[0] == 'f')            
        {                
            int tmp[4];
            int tem;
            ins >> a;
            int i = 0;
            while (ins >> tmp[i] >> a>> /*tem >>*/ a >> tmp[3])
            {
                tmp[i] -= 1;//将所有数减一，数学中以1为起始，计算机中以0为起始  
                i++;
            }
            Triangle t{ material_id, { pos_list[tmp[0]], pos_list[tmp[1]], pos_list[tmp[2]] },
                normal_list[tmp[3] - (int)1] };
            triangle_vector.push_back(t);
        } 
    }
}
