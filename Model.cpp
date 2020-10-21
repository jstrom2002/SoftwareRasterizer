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
    
    void Model::Draw(cv::Mat& img, cv::Mat& imgZ, glm::mat4 P, glm::mat4 V, 
        int w, int h, int frameCount, bool wireframeOn, bool cullFace, bool frontFaceCCW, 
        bool depthTest, unsigned int& trianglesRendered)
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
                glm::vec3 v2[3] = 
                {
                   glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v[0].position, 1.0f)),
                   glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v[1].position, 1.0f)),
                   glm::vec3(MVP * glm::vec4(this->m_Triangles[i].v[2].position, 1.0f))
                };

                // Normalize by homogenous coordinate to convert from clip space to screen space.
                // Note that we keep the z coordinate unchanged instead of normalizing it, for 
                // use in later writing to the depth buffer.
                for (int r = 0; r < 3; ++r)
                {
                    v2[r].x /= v2[r].z;
                    v2[r].y /= v2[r].z;
                }

                //// Check whether projected points fit view volume in NDC space.
                glm::bvec3 inNDC = glm::bvec3(false);
                for (int i = 0; i < 3; ++i)
                {
                    if (v2[i].x >= -1 &&
                        v2[i].x <= 1 &&
                        v2[i].y >= -1 &&
                        v2[i].y <= 1 &&
                        v2[i].z >= 0 &&
                        v2[i].z <= 1)
                    {
                        inNDC[i] = true;
                    }
                }
                if (!inNDC[0] && !inNDC[1] && !inNDC[2])
                    continue;

                // Make a copy of the triangle, now projected to clip space.
                Triangle clipspaceTri = Triangle(
                    Vertex(v2[0], this->m_Triangles[i].v[0].texcoord, this->m_Triangles[i].v[0].normal),
                    Vertex(v2[1], this->m_Triangles[i].v[1].texcoord, this->m_Triangles[i].v[1].normal),
                    Vertex(v2[2], this->m_Triangles[i].v[2].texcoord, this->m_Triangles[i].v[2].normal),
                    this->m_Triangles[i].materialIndex
                );

                // Cull faces as necessary.
                if (cullFace)                
                    if (clipspaceTri.isCCW() != frontFaceCCW)
                        continue;                

                // Keep copy of NDC bounds check with clip space triangle for testing.
                // Somewhat hacky, should be fixed.
                clipspaceTri.setInNDCbounds(inNDC);

                // Convert clip space coords [-1,1] to integer screen space coords [0,w],[0,h],
                // which correspond to pixel indices on the output frame.
                for (int q = 0; q < 3; ++q)
                {
                    clipspaceTri.v[q].position.x = int((clipspaceTri.v[q].position.x + 1.0) * 0.5 * w);
                    clipspaceTri.v[q].position.y = int((clipspaceTri.v[q].position.y + 1.0) * 0.5 * h);
                }

                // If every point of triangle is at a depth greater than
                // that of the current depths in the z-buffer, skip rendering
                // because the triangle is occluded.
                int vertexOccluded = 0;
                if (depthTest)
                {
                    float minZ = clipspaceTri.getMinZ();
                    for (int p = 0; p < 3; ++p)
                    {
                        if (clipspaceTri.v[p].position.x >= 0 && 
                            clipspaceTri.v[p].position.x < imgZ.rows &&
                            clipspaceTri.v[p].position.y >= 0 && 
                            clipspaceTri.v[p].position.y < imgZ.cols)
                        {
                            // If vertex is occluded, increment occluded vertex counter.
                            if (minZ > imgZ.at<cv::Vec3f>(clipspaceTri.v[p].position.x,
                                clipspaceTri.v[p].position.y)[2])                            
                                    vertexOccluded++;                            
                        }
                    }
                }
                if (vertexOccluded >= 3)
                    continue;

                // Get diffuse color. Remember that OpenCV requires conversion of values from
                // BGR -> RGB.
                Material* material = &this->m_Materials[this->m_Triangles[i].materialIndex];
                glm::vec3 dif = material->diffuse * 255.f;
                float col[3] = { material->diffuse.z, material->diffuse.y, material->diffuse.x };

                // Draw rasterized triangle(s) as necessary.
                clipspaceTri.Draw(img, imgZ, material, col, wireframeOn, depthTest);      
                trianglesRendered++;
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
            char txpath[256];
            char buf[128];
            int res = fscanf(file, "%s", buf);
            if (res == EOF)            
                break;
            
            if (strcmp(buf, "newmtl") == 0)
            {
                char str[80];
                fscanf(file, "%s\n", str);
                m_MaterialNames.push_back(str);
                m_Materials.push_back(Material());
            }

            // Handle loading material properties.
            else if (strcmp(buf, "Kd") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().diffuse.x, &m_Materials.back().diffuse.y, &m_Materials.back().diffuse.z);
            }
            else if (strcmp(buf, "Ks") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().specular.x, &m_Materials.back().specular.y, &m_Materials.back().specular.z);
            }
            else if (strcmp(buf, "Ka") == 0)
            {
                fscanf(file, "%f %f %f\n", &m_Materials.back().ambient.x, &m_Materials.back().ambient.y, &m_Materials.back().ambient.z);
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
            else if (strcmp(buf, "d") == 0)
            {
                fscanf(file, "%f\n", &m_Materials.back().opacity);
            }

            // Handle loading light maps.
            else if (strcmp(buf, "map_Kd") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::DIFFUSE, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_Ks") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::SPECULAR, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_Ka") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::AMBIENT, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_Ke") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::EMISSIVE, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_Kn") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::NORMALS, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_Ns") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::SHININESS, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_d") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::OPACITY, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "map_disp") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::DISPLACEMENT, m_Materials.size() - 1);
            }
            else if (strcmp(buf, "refl") == 0)
            {
                fscanf(file, "%s\n", &txpath);
                m_Materials.back().textures.push_back(MaterialTexture());
                m_Materials.back().textures.back().loadTexture(
                    txpath, TEXTURE_TYPE::REFLECTION, m_Materials.size() - 1);
            }
        }
    }
}