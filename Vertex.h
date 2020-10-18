#pragma once
#include <glm/glm.hpp>

namespace SoftwareRasterizer
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent_s;
        glm::vec3 tangent_t;

        Vertex() {}
        Vertex(const glm::vec3& position, const glm::vec2& texcoord, const glm::vec3& normal)
            : position(position), texcoord(texcoord.x, texcoord.y, 0),
            normal(normal)
        {}
    };
}