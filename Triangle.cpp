#include "Triangle.h"
#include "Material.h"
#include "Line.h"

namespace SoftwareRasterizer
{
    float Triangle2D::getMinX()
    {
        int min_x = (pt[0].x < pt[1].x) ? pt[0].x : pt[1].x;
        return (pt[2].x < min_x) ? pt[2].x : min_x;
    }

    float Triangle2D::getMaxX()
    {
        int max_x = (pt[0].x > pt[1].x) ? pt[0].x : pt[1].x;
        return (pt[2].x > max_x) ? pt[2].x : max_x;
    }

	float Triangle2D::getMinY()
	{
		int min_y = (pt[0].y < pt[1].y) ? pt[0].y : pt[1].y;
		return (pt[2].y < min_y) ? pt[2].y : min_y;
	}

	float Triangle2D::getMaxY()
	{
		int max_y = (pt[0].y > pt[1].y) ? pt[0].y : pt[1].y;
		return (pt[2].y > max_y) ? pt[2].y : max_y;
	}

	void Triangle2D::Draw(cv::Mat& img, cv::Mat& imgZ, Material* mat, float* col, float* depth)
	{
        // Get min/max dimensions, edge length of this triangle.
        int minX = getMinX();
        int maxX = getMaxX();
        int extentX = std::abs(maxX - minX)+1;

        int minY = getMinY();
		int maxY = getMaxY();
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
            // Get starting indices.
            int idx = (iter +1) % 3;
            int x1 = pt[iter].x;
            int x2 = pt[idx].x;
            int y1 = pt[iter].y;
            int y2 = pt[idx].y;

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
            Point p1(minMaxXVals[i][0], minY + i);
            Point p2(minMaxXVals[i][1], minY + i);

            //l.draw(img,testcol,1U);
            //l.draw(imgZ,testdepth,1U);

            // Draw horizontal line for pixel color.
            for (int j = minMaxXVals[i][0]; j <= minMaxXVals[i][1]; ++j)
            {                                
                // Set output frame's pixel color.
                if (img.channels() == 3) {
                    img.at<cv::Vec3f>(minY + i,j) =
                        cv::Vec3f(col[0], col[1], col[2]);
                }
                // Set z-buffer depth values.
                if (imgZ.channels() == 3) {
                        imgZ.at<cv::Vec3f>(minY + i,j) =
                            cv::Vec3f(depth[0], depth[1], depth[2]);
                }
            }       
        }
	}
}