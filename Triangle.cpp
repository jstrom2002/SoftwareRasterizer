#include "Triangle.h"
#include "Material.h"
#include "Line.h"

namespace SoftwareRasterizer
{
    float Triangle::getZ(glm::vec2 p)
    {//see: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
     //this function assumes the third z coordinate (v[2].position.z) is unknown.
        float lambda = glm::clamp((p.x - v[0].position.x) / (v[1].position.x - v[0].position.x), 0.0f, 1.0f);
        return 1.0/((1.0 - lambda)/v[0].position.z + lambda/v[1].position.z);
    }

    glm::vec3 Triangle::getBarycenterCoords(glm::vec3 p)
    {
        glm::vec3 coefs = glm::vec3(0);
        for (int i = 0; i < 2; ++i)
        {
            int idx_next = (i + 1) % 3;
            int idx_next_next = (i + 2) % 3;

            // Coefficients of barycentric coordinates (u,v,w) are of form P = uA + uB + wC.            
            // These can be calculated by the norm of sides of a parallelogram containing the whole
            // triangle and a parallelogram containing the adjacent side and current point P.            
            coefs[i] = glm::cross(v[i].position - p, 
                v[i].position - v[idx_next_next].position).length() / 
                glm::cross(v[i].position-v[idx_next].position,
                    v[i].position - v[idx_next_next].position).length();
        }

        // Since u + v + w = 1, the third coefficient may be calc'd w.r.t. the other two.
        coefs[2] = 1.0 - coefs[0] - coefs[1];
        return coefs;
    }

    glm::bvec3 Triangle::checkVertsInNDCbounds()
    {
        glm::bvec3 inNDC = glm::bvec3(false);
        for (int i = 0; i < 3; ++i)
        {
            if (v[i].position.x >= -1 && 
                v[i].position.x <= 1 &&
                v[i].position.y >= -1 && 
                v[i].position.y <= 1 &&
                v[i].position.z >= 0 && 
                v[i].position.z <= 1)
            {
                inNDC[i] = true;
            }
        }  
        return inNDC;
    }

    void Triangle::setInNDCbounds(glm::bvec3 inNDC)
    {
        this->inNDC = inNDC;
    }

    Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3, unsigned int mtlindex) :
        materialIndex(mtlindex)
    {
        v[0] = v1;
        v[1] = v2;
        v[2] = v3;
    }

    Triangle::Triangle(cv::Point p1, cv::Point p2, cv::Point p3)
    {
        v[0].position = glm::vec3(p1.x,p1.y,1);
        v[1].position = glm::vec3(p2.x,p2.y,1);
        v[2].position = glm::vec3(p3.x,p3.y,1);
    }

    float Triangle::getMinX()
    {
        int min_x = (v[0].position.x < v[1].position.x) ? v[0].position.x : v[1].position.x;
        min_x = (v[2].position.x < min_x) ? v[2].position.x : min_x;
        return min_x >= 0 ? min_x : 0;
    }

    float Triangle::getMaxX()
    {
        int max_x = (v[0].position.x > v[1].position.x) ? v[0].position.x : v[1].position.x;
        return (v[2].position.x > max_x) ? v[2].position.x : max_x;
    }

    float Triangle::getMinY()
    {
        int min_y = (v[0].position.y < v[1].position.y) ? v[0].position.y : v[1].position.y;
        min_y = (v[2].position.y < min_y) ? v[2].position.y : min_y;
        return min_y >= 0 ? min_y : 0;
    }

    float Triangle::getMaxY()
    {
        int max_y = (v[0].position.y > v[1].position.y) ? v[0].position.y : v[1].position.y;
        return (v[2].position.y > max_y) ? v[2].position.y : max_y;
    }

    float Triangle::getMinZ()
    {
        int min_z = (v[0].position.z < v[1].position.z) ? v[0].position.z : v[1].position.z;
        min_z = (v[2].position.z < min_z) ? v[2].position.z : min_z;
        return min_z >= 0 ? min_z : 0;
    }

    float Triangle::getMaxZ()
    {
        int max_z = (v[0].position.z > v[1].position.z) ? v[0].position.z : v[1].position.z;
        return (v[2].position.z > max_z) ? v[2].position.z : max_z;
    }

	void Triangle::Draw(cv::Mat& img, cv::Mat& imgZ, Material* mat, float* col, bool wireframeOn,
        bool depthTest)
	{
        // Get min/max dimensions, edge length of this triangle.
        int minX = getMinX();
        int maxX = getMaxX();
        maxX = maxX < (img.cols-1) ? maxX : (img.cols-1);//Keep x values in frame bounds.
        int extentX = std::abs(maxX - minX)+1;

        int minY = getMinY();
		int maxY = getMaxY();
        maxY = maxY < (img.rows - 1) ? maxY : (img.rows - 1);//Keep y values in frame bounds.
        int extentY = std::abs(maxY-minY)+1;
        
        // Create an array of values for each horizontal scanline of the image.
        // minMaxXVals[0] = min val, [1] = max val.
        std::vector<std::array<int,2>> minMaxXVals;
		minMaxXVals.resize(extentY);
        for (int i = 0; i < minMaxXVals.size(); ++i)
        {
            minMaxXVals[i][0] = std::numeric_limits<int>::max();
            minMaxXVals[i][1] = -1;
        }

        // For each edge of the triangle, iterate along each scanline using a raster 
        // algorithm and find min/max extrema for each line.
        //// Code adapted from: https://stackoverflow.com/questions/7870533/c-triangle-rasterization
        for (int iter = 0; iter < 3; ++iter)
        {           
            int idx = (iter + 1) % 3;

            // Get starting indices.
            int x1 = v[iter].position.x;
            int x2 = v[idx].position.x;
            int y1 = v[iter].position.y;
            int y2 = v[idx].position.y;

            long sx, sy, dx1, dy1, dx2, dy2, x, y, m, n, k, cnt;

            // Calc edge length to find if direction of travel, dx/dy, is positive or negative.
            sx = x2 - x1;
            sy = y2 - y1;

            if (sx > 0) dx1 = 1;
            else if (sx < 0) dx1 = -1;
            else dx1 = 0;

            if (sy > 0) dy1 = 1;
            else if (sy < 0) dy1 = -1;
            else dy1 = 0;

            // Do edge length comparison for slope.
            m = std::abs(sx);
            n = std::abs(sy);
            dx2 = dx1;
            dy2 = 0;

            // If line is more vertical (ie has slope > 1), increment further in y direction than x.
            if (m < n)
            {
                m = std::abs(sy);
                n = std::abs(sx);
                dx2 = 0;
                dy2 = dy1;
            }

            // Store x1,y1 positions, calculate distance of travel.
            x = x1; y = y1;
            cnt = m + 1;
            k = n / 2;

            while (cnt--)
            {
                // Save all values for extrema of x within triangle y bounds.
                if ((y >= minY) && (y <= maxY))
                {
                    {
                        if (x < minMaxXVals[y - minY][0]) minMaxXVals[y - minY][0] = x;
                        if (x > minMaxXVals[y - minY][1]) minMaxXVals[y - minY][1] = x;
                    }
                }

                // Continue to iterate down line.
                k += n;
                if (k < m)
                {
                    x += dx2;
                    y += dy2;
                }
                else
                {
                    k -= m;
                    x += dx1;
                    y += dy1;
                }
            }
        }

        // With scanline extrema marked, now shade pixels from x-min to x-max.
        for (int i = 0; i < minMaxXVals.size(); ++i)
        {
            // Draw horizontal line for pixel color.
            cv::Vec3f* row = img.ptr<cv::Vec3f>(minY+i);
            cv::Vec3f* rowZ = imgZ.ptr<cv::Vec3f>(minY+i);
            for (int j = minMaxXVals[i][0]; j <= minMaxXVals[i][1]; ++j)
            {            
                // Skip negative indices or interior values if in wireframe mode.
                if (j < 0 || (wireframeOn && j > minMaxXVals[i][0] && j < minMaxXVals[i][1]))                
                    continue;                

                // Compare this depth value to current depth at this pixel in zbuffer.
                float interpDepth = getZ(glm::vec2(j, minY + i));
                if (!depthTest || rowZ[j][2] > interpDepth)
                {
                    // Set output frame's pixel color.
                    if (img.channels() == 3) {
                        row[j] = cv::Vec3f(col[0], col[1], col[2]);
                    }
                    // Set z-buffer depth values.
                    if (imgZ.channels() == 3) {
                        rowZ[j] = cv::Vec3f(0.0f,0.0f,interpDepth);
                    }
                }
            }       
        }
	}
}