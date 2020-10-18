#pragma once
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
    struct Material
    {
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 emission;
        unsigned int type;
        float roughness;
        float ior;
        std::vector<cv::Mat> textures;

        Material() : type(2), diffuse(glm::vec3(0.2)), specular(glm::vec3(1)), roughness(9999), ior(0) {}
    };
}