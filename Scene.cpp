#include "Scene.h"
#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SoftwareRasterizer
{
    Scene::Scene() : w(0), h(0), frameCount(0), screenshotCount(0), windowClose(false), keyPressed(0),
        showFPS(false), showDepth(false), wireframeOn(false), cullFace(false), frontFaceCCW(true),
        depthTest(true), showRenderedTriangleCount(false)
    {
        // Set screenshot count to last value.
        std::string ssname = "screenshot_" + std::to_string(screenshotCount) + ".png";
        while (std::filesystem::exists(ssname))
        {
            screenshotCount++;
            ssname = "screenshot_" + std::to_string(screenshotCount) + ".png";
        }
    }

    Scene::~Scene()
    {
        if (!frame.empty())
            frame.deallocate();
        if (!frameZ.empty())
            frameZ.deallocate();
    }

    void Scene::AddModel(std::string filename)
    {
        Model m(filename);
        models.push_back(m);
    }

    unsigned int Scene::TotalTriangles()
    {
        unsigned int total = 0;
        for (int i = 0; i < models.size(); ++i)
            total += models[i].m_Triangles.size();
        return total;
    }

	void Scene::Draw()
	{
        // Setup window and viewport variables.
        glm::mat4 P = glm::perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
        frameCount = 0;

        // Draw model until user presses 'ESC' key.
        std::cout << "*** USER CONTROLS ****" << std::endl;
        std::cout << "'ESC' - quit" << std::endl;
        std::cout << "'wsad' - move camera" << std::endl;
        std::cout << "'qe' - rotate camera vertically" << std::endl;
        std::cout << "'zc' - rotate camera horizontally" << std::endl;
        std::cout << "'p' - screenshot" << std::endl;
        std::cout << "'o' - show FPS" << std::endl;
        std::cout << "'i' - render depth" << std::endl;
        std::cout << "'u' - wireframe mode" << std::endl;
        std::cout << "'j' - toggle face culling" << std::endl;       
        std::cout << "'k' - toggle front face CCW or CW" << std::endl;       
        std::cout << "'l' - toggle depth test" << std::endl;       
        std::cout << "';' - display rendered triangle count" << std::endl;       
        std::cout << "***********************" << std::endl;        
        while (!windowClose)
        {
            // Update per-frame logic and user input.
            camera.Update();
            keyPressed = (char)cv::waitKey(1);
            ProcessInput(keyPressed);

            // Start with a cleared image and z-buffer. Z-buffer cleared value = 1,
            // farthest depth of view volume in clip space.
            frame = cv::Mat(h, w, CV_32FC3, cv::Scalar(0,0,0));
            frameZ = cv::Mat(h, w, CV_32FC3, cv::Scalar(1,1,1));

            // Render all models.
            unsigned int trianglesRendered = 0;
            startFrameTime = clock();
            glm::mat4 V = camera.getViewMatrix();
#pragma omp parallel for
            for (int i = 0; i < models.size(); ++i)            
                models[i].Draw(frame, frameZ, P, V, w, h, frameCount, wireframeOn, cullFace, frontFaceCCW, depthTest, trianglesRendered);
            endFrameTime = clock();

            // Draw text info if necessary.
            if (showFPS)
            {
                std::string FPStext = "FPS: " + std::to_string(
                    CLOCKS_PER_SEC/(float(endFrameTime) - float(startFrameTime)));
                cv::putText(frame, FPStext, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                    0.75, cv::Scalar(255, 255, 255, 255), 2, cv::LINE_AA);
            }
            if (showRenderedTriangleCount)
            {
                std::string FPStext = "% triangles rendered: " + std::to_string(
                    double(trianglesRendered)/double(this->TotalTriangles()));
                cv::putText(frame, FPStext, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX,
                    0.75, cv::Scalar(255, 255, 255, 255), 2, cv::LINE_AA);
            }

            // Finally, display results.
            if(showDepth)
                cv::imshow("Software Rasterizer", frameZ);
            else
                cv::imshow("Software Rasterizer", frame);
            frameCount++;
        }
	}

    void Scene::ProcessInput(char c)
    {
        if (c == 27)//'ESC' key.
            windowClose = true;

        // Handle camera movement.
        else if (c == 'w')
            camera.position += camera.front * camera.movementSpeed;
        else if (c == 's')
            camera.position -= camera.front * camera.movementSpeed;
        else if (c == 'a')
            camera.position -= camera.right * camera.movementSpeed;
        else if (c == 'd')
            camera.position += camera.right * camera.movementSpeed;

        // Handle camera rotation.
        else if (c == 'q')
            camera.front = glm::normalize(camera.front + camera.up * camera.movementSpeed);
        else if (c == 'e')
            camera.front = glm::normalize(camera.front - camera.up * camera.movementSpeed);
        else if (c == 'z')
            camera.front = glm::normalize(camera.front - camera.right * camera.movementSpeed);
        else if (c == 'c')
            camera.front = glm::normalize(camera.front + camera.right * camera.movementSpeed);

        // Handle settings buttons.
        else if (c == 'o')
            this->showFPS = !this->showFPS;
        else if (c == 'i')
            this->showDepth = !this->showDepth;
        else if (c == 'u')
            this->wireframeOn = !this->wireframeOn;
        else if (c == 'j')
            this->cullFace = !this->cullFace;
        else if (c == 'k')
            this->frontFaceCCW = !this->frontFaceCCW;
        else if (c == 'l')
            this->depthTest = !this->depthTest;
        else if (c == ';')
            this->showRenderedTriangleCount = !this->showRenderedTriangleCount;
        else if (c == 'p')
        {
            // Ensure screenshots are not being overwritten.
            std::string ssname = "screenshot_" + std::to_string(screenshotCount) + ".png";
            while (std::filesystem::exists(ssname))
            {
                screenshotCount++;
                ssname = "screenshot_" + std::to_string(screenshotCount) + ".png";
            }

            //Save screenshot.
            cv::Mat img = frame.clone();
            img.convertTo(img, CV_8UC3, 255.0);
            cv::imwrite(ssname, img);
            img.deallocate();
            std::cout << "screenshot saved." << std::endl;
            screenshotCount++;
        }
    }

}