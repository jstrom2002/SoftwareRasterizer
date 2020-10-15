#include "Model.h"
#include "Triangle.h"
#include "Point.h"
#include "Line.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/opencv.hpp>
#include <limits>
#include <ctime>

namespace SoftwareRasterizer
{
    Model::Model(const char* filename)
    {
        bounds[0] = glm::vec3(std::numeric_limits<float>::max());
        bounds[1] = glm::vec3(-std::numeric_limits<float>::max());
        LoadTriangles(filename);
    }

    void Model::LoadTriangles(const char* filename)
    {
        std::vector<glm::vec3> positions;
        FILE* file = fopen(filename, "r");
        if (!file)
        {
            throw std::exception("Failed to open .OBJ file!");
        }

        while (true)
        {
            char lineHeader[128];
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
            {
                break;
            }
            if (strcmp(lineHeader, "v") == 0)
            {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                
                // Update bounding box dimensions.
                if (vertex.x < bounds[0].x)
                    bounds[0].x = vertex.x;
                if (vertex.x > bounds[1].x)
                    bounds[1].x = vertex.x;
                if (vertex.y < bounds[0].y)
                    bounds[0].y = vertex.y;
                if (vertex.y > bounds[1].y)
                    bounds[1].y = vertex.y;
                if (vertex.z < bounds[0].z)
                    bounds[0].z = vertex.z;
                if (vertex.z > bounds[1].z)
                    bounds[1].z = vertex.z;

                positions.push_back(vertex);
            }

            else if (strcmp(lineHeader, "f") == 0)
            {
                // For now, faces must be triangularized (ie 3 indices per face maximum).
                unsigned int iv[3], it[3], in[3];
                int count = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &iv[0], &it[0], &in[0], &iv[1], &it[1], &in[1], &iv[2], &it[2], &in[2]);
                if (count != 9)
                {
                    throw std::exception("Failed to load face!");
                }
                m_Triangles.push_back(Triangle(positions[iv[0] - 1],positions[iv[1] - 1],positions[iv[2] - 1]));
            }
        }

        std::cout << "Model loaded.\nbounds: [" << bounds[0].x << "," << bounds[1].x << "], ["
            << bounds[0].y << "," << bounds[1].y << "], [" << bounds[0].z << "," << bounds[1].z << "]" << std::endl;
    }
    
    bool Model::inNDCscreen(glm::vec3 v)
    {
        if (v.x >= -1 && v.x <= 1 &&
            v.y >= -1 && v.y <= 1 &&
            v.z <= 0)
        {
            return true;
        }
        else
            return false;
    }

    void Model::Draw(int w, int h, glm::vec3 pos, float scaleVal, glm::vec3 rot)
    {
        // Setup window and viewport variables. For now, only simple orthographic projection 
        // view matrices are implemented.
        cv::Mat img;
        float col[3] = { 255,255,255 };
        unsigned int frameCount = 0;
        glm::mat4 P = glm::ortho(0.0f, float(w), float(h), 0.0f, 0.0f, 2.0f * scaleVal * this->bounds[1].z);
        glm::mat4 V = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

        // Draw model until user presses 'ESC' key.
        std::cout << "*** Press 'ESC' to quit ****" << std::endl;
        while (cv::waitKey(1) != 27)
        {
            // Start with a cleared image.
            img = cv::Mat::zeros(w, h, CV_32F);

            // Apply transforms.
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, pos);
            M = glm::scale(M, glm::vec3(scaleVal));
            M = glm::rotate(M, glm::radians(float(frameCount * rot.length())), glm::normalize(rot));
            glm::mat4 MVP = P * V * M;

            // Iterate over triangles, project to screen space, rasterize lines.
            for (int i = 0; i < this->m_Triangles.size(); ++i)
            {
                // Transform to clip space by projection.
                glm::vec3 v[3] = {
                    glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v1, 1.0f)),
                    glm::vec(MVP * glm::vec4(this->m_Triangles[i].v2, 1.0f)),
                    glm::vec(MVP * glm::vec4(this->m_Triangles[i].v3, 1.0f))
                };

                // Check whether projected points fit view volume in NDC space.
                bool chk[3] = { inNDCscreen(v[0]), inNDCscreen(v[1]), inNDCscreen(v[2]) };

                // Draw all valid triangle edge line combinations on screen.
                for (int i = 0; i < 3; ++i)
                    for (int j = i; j < 3; ++j)
                        if (i != j && chk[i] && chk[j])
                        {
                            // Transform coordinates to screen space for drawing.
                            Point p1(int((v[i].x + 1) * 0.5 * w), int((v[i].y + 1) * 0.5 * h));
                            Point p2(int((v[j].x + 1) * 0.5 * w), int((v[j].y + 1) * 0.5 * h));
                            Line l(p1, p2);
                            l.draw(img, col, 1U);
                        }
            }
            // Finally, display results.
            cv::imshow("Software Rasterizer", img);
            frameCount++;
        }
    }
}