#pragma once
#include "Vertex.h"
#include "Point.h"
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
    class Point;
    class Material;

    struct Triangle3D
    {
        Vertex v1, v2, v3;
        unsigned int materialIndex;

        Triangle3D(Vertex v1, Vertex v2, Vertex v3, unsigned int mtlindex) : v1(v1), v2(v2), v3(v3), 
            materialIndex(mtlindex) 
        {}
    };

    struct Triangle2D
    {
        Point pt[3];

        Triangle2D(Point p1, Point p2, Point p3) 
        {
            pt[0] = (p1); 
            pt[1] = (p2); 
            pt[2] = (p3);
        }

        Triangle2D(cv::Point p1, cv::Point p2, cv::Point p3)
        {
            pt[0] = (p1);
            pt[1] = (p2);
            pt[2] = (p3);
        }
        
        void Draw(cv::Mat& img, cv::Mat& imgZ, Material* mat, float* col, float* depth);
    private:
        float getMaxX();
        float getMinX();
        float getMaxY();
        float getMinY();
    };
}