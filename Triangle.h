#pragma once
#include "Vertex.h"
#include "Point.h"
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
    class Point;
    class Material;

    /**
    *  \brief Struct for loading 3D triangular faces of model meshes. Vertices are intended to be
    *         loaded in culling order so that v[0],v[1],v[2] are counter-clockwise.
    */
    class Triangle
    {
    public:
        Vertex v[3];
        unsigned int materialIndex;

        Triangle(Vertex v1, Vertex v2, Vertex v3, unsigned int mtlindex);
        Triangle(cv::Point p1, cv::Point p2, cv::Point p3);

        void Draw(cv::Mat& img, cv::Mat& imgZ, Material* mat, float* col,
            bool wireframeOn, bool depthTest);

        inline bool isCCW() {
            return glm::normalize(glm::cross(v[1].position - v[0].position,
                v[2].position - v[0].position)).z <= 0; 
        }

        float getMaxX();
        float getMinX();
        float getMaxY();
        float getMinY();
        float getMaxZ();
        float getMinZ();

         glm::bvec3 checkVertsInNDCbounds();
         void setInNDCbounds(glm::bvec3 inNDC);
    private:
        glm::bvec3 inNDC;
        float getZ(glm::vec2 p);
        glm::vec3 getBarycenterCoords(glm::vec3 p);
    };
}