#pragma once
#include "Vertex.h"

namespace SoftwareRasterizer
{
    struct Triangle
    {
        Vertex v1, v2, v3;
        unsigned int materialIndex;

        Triangle(Vertex v1, Vertex v2, Vertex v3, unsigned int mtlindex) : v1(v1), v2(v2), v3(v3), 
            materialIndex(mtlindex) {}
    };
}