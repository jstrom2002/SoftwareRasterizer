#pragma once
#include "Triangle.h"
#include "Material.h"
#include <vector>
#include <string>

namespace SoftwareRasterizer
{
	class Model
	{
	public:
        std::vector<Triangle> m_Triangles;  
        std::vector<Material> m_Materials;  
        glm::vec3 bounds[2];//bounds[0] = minima, bounds[1] = maxima.
        bool wireframeOn;

        Model(const char* filename);
        void Draw(int w, int h, glm::vec3 pos, float scaleval, glm::vec3 rot);

    private:
        std::vector<std::string> m_MaterialNames;
        void LoadTriangles(const char* filename);
        void LoadMaterials(const char* filename);
	};

}