#pragma once
#include "Triangle.h"
#include <vector>

namespace SoftwareRasterizer
{
	class Model
	{
	public:
        std::vector<Triangle> m_Triangles;  
        glm::vec3 bounds[2];

        Model(const char* filename);
        void Draw(int w, int h, glm::vec3 pos, float scaleval, glm::vec3 rot);

    private:
        void LoadTriangles(const char* filename);
        bool inNDCscreen(glm::vec3 v);
	};

}