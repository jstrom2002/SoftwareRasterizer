#include "Material.h"

namespace SoftwareRasterizer
{
    MaterialTexture::MaterialTexture() : type(TEXTURE_TYPE::NONE), materialIndex(-1) {}

    MaterialTexture::~MaterialTexture()
    {
        if (!img.empty())
            img.deallocate();
    }

    void MaterialTexture::loadTexture(char* path, TEXTURE_TYPE txtype, unsigned int idx)
    {
        img = cv::imread(path);
        type = txtype;
        materialIndex = idx;
    }

    Material::Material() : type(2), diffuse(glm::vec3(0.2)), specular(glm::vec3(1)), 
        roughness(9999), ior(0), ambient(glm::vec3(0)), metalness(0), opacity(1), 
        emission(glm::vec3(0))
    {}
}