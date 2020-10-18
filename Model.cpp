#include "Model.h"
#include "Triangle.h"
#include "Vertex.h"
#include "Point.h"
#include "Line.h"
#include "Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <limits>
#include <ctime>

namespace SoftwareRasterizer
{
    Model::Model(const char* filename)
    {
        bounds[0] = glm::vec3(std::numeric_limits<float>::max());
        bounds[1] = glm::vec3(-std::numeric_limits<float>::max());
        LoadTriangles(filename);
        wireframeOn = false;
    }

    void Model::LoadTriangles(const char* filename)
    {
        char mtlname[80];
        memset(mtlname, 0, 80);
        strncpy(mtlname, filename, strlen(filename) - 4);
        strcat(mtlname, ".mtl");

        LoadMaterials(mtlname);
            
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
        unsigned int materialIndex = -1;
        Material currentMaterial;

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
            else if (strcmp(lineHeader, "vt") == 0)
            {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                texcoords.push_back(uv);
            }
            else if (strcmp(lineHeader, "vn") == 0)
            {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                normals.push_back(normal);
            }
            else if (strcmp(lineHeader, "usemtl") == 0)
            {
                char str[80];
                fscanf(file, "%s\n", str);
                for (unsigned int i = 0; i < m_MaterialNames.size(); ++i)
                {
                    if (strcmp(str, m_MaterialNames[i].c_str()) == 0)
                    {
                        materialIndex = i;
                        break;
                    }
                }
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
                m_Triangles.push_back(Triangle(
                    Vertex(positions[iv[0] - 1], texcoords[it[0] - 1], normals[in[0] - 1]),
                    Vertex(positions[iv[1] - 1], texcoords[it[1] - 1], normals[in[1] - 1]),
                    Vertex(positions[iv[2] - 1], texcoords[it[2] - 1], normals[in[2] - 1]),
                    materialIndex
                ));
        }
        }

        std::cout << "Model loaded.\nbounds: [" << bounds[0].x << "," << bounds[1].x << "], ["
            << bounds[0].y << "," << bounds[1].y << "], [" << bounds[0].z << "," << bounds[1].z << "]" << std::endl;
    }
    
    void Model::Draw(int w, int h, glm::vec3 pos, float scaleVal, glm::vec3 rot)
    {
        // Setup window and viewport variables. For now, only simple orthographic projection and
        // view matrices are implemented.
        cv::Mat img;
        unsigned int frameCount = 0;
        glm::mat4 P = glm::perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
        glm::mat4 V = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

        // Draw model until user presses 'ESC' key.
        std::cout << "*** Press 'ESC' to quit ****" << std::endl;
        while (cv::waitKey(1) != 27)
        {
            // Start with a cleared image.
            img = cv::Mat::zeros(w, h, CV_32FC3);

            // Apply transforms.
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, pos);
            M = glm::scale(M, glm::vec3(1,-1,1) * scaleVal);//invert y-axis value to flip image.
            M = glm::rotate(M, glm::radians(float(frameCount * rot.length())), glm::normalize(rot));
            glm::mat4 MVP = P* V* M;

            // Iterate over triangles, project to screen space, rasterize lines.
            for (int i = 0; i < this->m_Triangles.size(); ++i)
            {
                // Transform to clip space by projection, dividing out z,w values to get (x,y) coord.
                glm::vec2 v[3] = {
                    glm::vec2(MVP * glm::vec4(this->m_Triangles[i].v1.position, 1.0f)),
                    glm::vec2(MVP * glm::vec4(this->m_Triangles[i].v2.position, 1.0f)),
                    glm::vec2(MVP * glm::vec4(this->m_Triangles[i].v3.position, 1.0f))
                };

                // Get diffuse color. Remember opencv requires conversion RGB -> BGR.
                Material* material = &this->m_Materials[this->m_Triangles[i].materialIndex];
                glm::vec3 dif = material->diffuse * 255.f;
                float col[3] = { material->diffuse.z, material->diffuse.y, material->diffuse.x };
                cv::Scalar colcv = cv::Scalar(col[0], col[1], col[2]);

                // Draw all polygons/shapes on output frame.
                std::vector<cv::Point> contours; 
                std::vector<std::vector<cv::Point>> contourVec;
                for (int i = 0; i < 3; ++i)
                {
                    int j = (i + 1) % 3;
                    // Transform coordinates to screen space for drawing.
                    Point p1(int((v[i].x + 1) * 0.5 * w), int((v[i].y + 1) * 0.5 * h));

                    //// Draw outline of triangle using sub-pixel accurate Wu rasterization algorithm.
                    if (wireframeOn)
                    {
                        Point p2(int((v[j].x + 1) * 0.5 * w), int((v[j].y + 1) * 0.5 * h));
                        Line l(p1, p2);
                        l.draw(img, col, 1U, LINE_ALGORITHM::WU);
                    }
                    //// Else add point to array.
                    else if (p1.x >= 0 && p1.y >= 0)
                        contours.push_back(cv::Point(p1.x, p1.y));                        
                
                    // Draw filled triangle(s) as necessary.
                    if (!wireframeOn && contours.size() >= 3)
                    {
                        contourVec.push_back(contours);
                        cv::drawContours(img, contourVec, -1, colcv, -1, cv::LINE_AA);
                        contourVec.clear();
                        contours.clear();
                    }
                }
            }
            // Finally, display results.
            cv::imshow("Software Rasterizer", img);
            frameCount++;
        }
    }

    void Model::LoadMaterials(const char* filename)
    {
        FILE* file = fopen(filename, "r");
        if (!file)
        {
            throw std::exception("Failed to open material file!");
        }

        while (true)
        {
            char buf[128];
            int res = fscanf(file, "%s", buf);
            if (res == EOF)
            {
                break;
            }
            if (strcmp(buf, "newmtl") == 0)
            {
                char str[80];
                fscanf(file, "%s\n", str);
                m_MaterialNames.push_back(str);
                m_Materials.push_back(Material());
            }
            else if (strcmp(buf, "Kd") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().diffuse.x, &m_Materials.back().diffuse.y, &m_Materials.back().diffuse.z);
            }
            else if (strcmp(buf, "Ks") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().specular.x, &m_Materials.back().specular.y, &m_Materials.back().specular.z);
            }
            else if (strcmp(buf, "Ke") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().emission.x, &m_Materials.back().emission.y, &m_Materials.back().emission.z);
            }
            else if (strcmp(buf, "Ns") == 0)
            {
                fscanf(file, "%f\n", &m_Materials.back().roughness);
            }
            else if (strcmp(buf, "Ni") == 0)
            {
                fscanf(file, "%f\n", &m_Materials.back().ior);
            }
        }
    }
}