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
#include <stdlib.h>  

namespace SoftwareRasterizer
{
    Model::Model(std::string  filename)
    {
        position = rotation = glm::vec3(0);
        scale = 1;
        bounds[0] = glm::vec3(std::numeric_limits<float>::max());
        bounds[1] = glm::vec3(-std::numeric_limits<float>::max());
        LoadTriangles(filename);
    }

    void Model::LoadTriangles(std::string  filename)
    {
        char mtlname[80];
        memset(mtlname, 0, 80);
        strncpy(mtlname, filename.c_str(), strlen(filename.c_str()) - 4);
        strcat(mtlname, ".mtl");

        LoadMaterials(mtlname);
            
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
        unsigned int materialIndex = -1;
        Material currentMaterial;

        FILE* file = fopen(filename.c_str(), "r");
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
                // Tokenize line and parse index values.
                char linestr[128];
                fgets(linestr, sizeof(linestr), file);
                char* tokens = strtok(linestr, " ");
                std::vector<unsigned int> iv, it, in;
                while (tokens != NULL)
                {
                    if (strlen(tokens) > 1)
                    {
                        iv.push_back(0);
                        it.push_back(0);
                        in.push_back(0);
                        sscanf(tokens, "%d/%d/%d", &iv[iv.size() - 1], &it[it.size() - 1], &in[in.size() - 1]);
                    }
                    tokens = strtok(NULL, " ");
                }

                // Add all triangles to the array.
                for (int i = 0; i < iv.size() - 2; ++i)
                {
                    int idx1 = i + 0;
                    int idx2 = i + 1;
                    int idx3 = i + 2;
                    m_Triangles.push_back(Triangle(
                        Vertex(positions[iv[idx1] - 1], texcoords[it[idx1] - 1], normals[in[idx1] - 1]),
                        Vertex(positions[iv[idx2] - 1], texcoords[it[idx2] - 1], normals[in[idx2] - 1]),
                        Vertex(positions[iv[idx3] - 1], texcoords[it[idx3] - 1], normals[in[idx3] - 1]),
                        materialIndex
                    ));
                }
                // Add final triangle, connecting face back to the first vertex.
                if (iv.size() > 1)
                {
                    int idx1 = iv.size() - 2;
                    int idx2 = iv.size() - 1;
                    int idx3 = 0;
                    m_Triangles.push_back(Triangle(
                        Vertex(positions[iv[idx1] - 1], texcoords[it[idx1] - 1], normals[in[idx1] - 1]),
                        Vertex(positions[iv[idx2] - 1], texcoords[it[idx2] - 1], normals[in[idx2] - 1]),
                        Vertex(positions[iv[idx3] - 1], texcoords[it[idx3] - 1], normals[in[idx3] - 1]),
                        materialIndex
                    ));
                }
            }
        }

        std::cout << "Model " + filename + " loaded.\nbounds: [" << bounds[0].x << "," << bounds[1].x << "], ["
            << bounds[0].y << "," << bounds[1].y << "], [" << bounds[0].z << "," << bounds[1].z << "]" << std::endl;
        std::cout << "# faces: " << m_Triangles.size() << std::endl;
    }

    bool Model::inNDCscreen(glm::vec3 v)
    {
        if (v.x >= -1 && v.x <= 1 &&
            v.y >= -1 && v.y <= 1 &&
            v.z >= -1 && v.z <= 1)
        {
            return true;
        }
        else
            return false;
    }
    
    void Model::Draw(cv::Mat& img, cv::Mat& imgZ, glm::mat4 P, glm::mat4 V, int w, int h, 
        int frameCount, bool wireframeOn)
    {
            // Apply transforms.
            glm::mat4 M = glm::mat4(1);
            M = glm::translate(M, this->position);
            M = glm::scale(M, glm::vec3(1,-1,1) * this->scale);//invert y-axis value to flip image.
            M = glm::rotate(M, glm::radians(float(frameCount * this->rotation.length())), 
                glm::normalize(this->rotation));
            glm::mat4 MVP = P* V* M;

            // Iterate over triangles, project to screen space, rasterize lines.
            for (int i = 0; i < this->m_Triangles.size(); ++i)
            {
                // Transform to clip space by projection, dividing out z,w values to get (x,y) coord.
                glm::vec3 v[3] = {
                    glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v1.position, 1.0f)),
                    glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v2.position, 1.0f)),
                    glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v3.position, 1.0f))
                };

                // Get least depth value (temporary fix until per-pixel depth can be rendered).
                // Remember opencv requires conversion RGB -> BGR.
                float leastDepth = (v[0].z < v[1].z) ? v[0].z : v[1].z;
                leastDepth = (v[2].z < leastDepth) ? v[2].z : leastDepth;
                float coldepth[3] = { 0,0,leastDepth };
                cv::Scalar depthcv = cv::Scalar(coldepth[0], coldepth[1], coldepth[2]);

                // Normalize by homogenous coordinate to convert from clip space to screen space.
                if(v[0].z != 0)
                    v[0] /= v[0].z;
                if (v[1].z != 0)
                    v[1] /= v[1].z;
                if (v[2].z != 0)
                    v[2] /= v[2].z;

                // Check whether projected points fit view volume in NDC space.
                // Without this check, the 'Line' class cannot draw properly.
                bool chk[3] = { inNDCscreen(v[0]), inNDCscreen(v[1]), inNDCscreen(v[2]) };

                // Get diffuse color. Remember opencv requires conversion RGB -> BGR.
                Material* material = &this->m_Materials[this->m_Triangles[i].materialIndex];
                glm::vec3 dif = material->diffuse * 255.f;
                float col[3] = { material->diffuse.z, material->diffuse.y, material->diffuse.x };
                cv::Scalar colcv = cv::Scalar(col[0], col[1], col[2]);

                // Draw all polygons/shapes on output frame.
                std::vector<cv::Point> contours; 
                std::vector<std::vector<cv::Point>> contourVec;
                for (int n = 0; n < 3; ++n)
                {
                    int j = (n + 1) % 3;

                    // Transform coordinates to screen space for drawing.
                    Point p1(int((v[n].x + 1) * 0.5 * w), int((v[n].y + 1) * 0.5 * h));

                    //// If in wireframe mode, draw lines of triangle edges.
                    if (wireframeOn && chk[n] && chk[j])
                    {
                        Point p2(int((v[j].x + 1) * 0.5 * w), int((v[j].y + 1) * 0.5 * h));
                        Line l(p1, p2);
                        l.draw(img, col, 1U);
                        l.draw(imgZ, coldepth, 1U);
                    }
                    //// Else add point to array for drawing.
                    else if (chk[n])
                    {
                        contours.push_back(cv::Point(p1.x,p1.y));
                    }
                
                    // Draw filled triangle(s) as necessary.
                    if (!wireframeOn && contours.size() >= 3)
                    {
                        contourVec.push_back(contours);
                        cv::drawContours(img, contourVec, 0, colcv, -1, cv::LINE_AA);
                        cv::drawContours(imgZ, contourVec, 0, depthcv, -1, cv::LINE_AA);
                        contourVec.clear();
                        contours.clear();
                    }
                }
            }
    }

    void Model::LoadMaterials(std::string  filename)
    {
        FILE* file = fopen(filename.c_str(), "r");
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