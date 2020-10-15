#pragma once
#include <glm/glm.hpp>

namespace SoftwareRasterizer
{
    struct Triangle
    {
        Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
            : v1(v1), v2(v2), v3(v3)
        {}
        glm::vec3 v1, v2, v3;
    };
}