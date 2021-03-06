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
        glm::vec3 position, rotation;
        float scale;
        std::vector<Triangle> m_Triangles;
        std::vector<Material> m_Materials;  
        glm::vec3 bounds[2];//bounds[0] = minima, bounds[1] = maxima.

        Model(std::string filename);
        void Draw(cv::Mat& img, cv::Mat& imgZ, glm::mat4 P, glm::mat4 V,
            int w, int h, int frameCount, bool wireframeOn, bool cullFace, 
            bool frontFaceCCW, bool depthTest, unsigned int& trianglesRendered);

    private:
        std::vector<std::string> m_MaterialNames;
        void LoadTriangles(std::string  filename);
        void LoadMaterials(std::string  filename);
	};

}