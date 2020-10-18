#include "Scene.h"
#include "Model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SoftwareRasterizer
{
    void Scene::AddModel(std::string filename)
    {
        Model m(filename);
        models.push_back(m);
    }

	void Scene::Draw()
	{
        // Setup window and viewport variables.
        glm::mat4 P = glm::perspective(45.0f, float(w) / float(h), 0.1f, 100.0f);
        frameCount = 0;

        // Draw model until user presses 'ESC' key.
        std::cout << "*** Press 'ESC' to quit ****" << std::endl;
        std::cout << "*** Press 'wsad' keys to move camera ****" << std::endl;
        char pressed = 0;
        bool windowClose = false;
        while (!windowClose)
        {
            // Update per-frame logic.
            camera.Update();

            // Handle user input.
            pressed = (char)cv::waitKey(1);
            if (pressed == 27)
                windowClose = true;
            if (pressed == 'w')
                camera.position += camera.front * camera.movementSpeed;            
            if (pressed == 's')
                camera.position -= camera.front * camera.movementSpeed;
            if (pressed == 'a')
                camera.position -= camera.right * camera.movementSpeed;
            if (pressed == 'd')
                camera.position += camera.right * camera.movementSpeed;

            // Start with a cleared image.
            frame = cv::Mat::zeros(w, h, CV_32FC3);

            // Render all models.
            glm::mat4 V = camera.getViewMatrix();
            for (int i = 0; i < models.size(); ++i)
            {
                models[i].Draw(frame, P, V, w, h, frameCount);
            }

            // Finally, display results.
            cv::imshow("Software Rasterizer", frame);
            frameCount++;
        }
	}
}