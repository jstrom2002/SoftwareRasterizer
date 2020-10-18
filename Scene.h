#pragma once
#include "Camera.h"
#include <vector>
#include <filesystem>
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
	class Camera;
	class Model;

	class Scene
	{
	public:
		int w, h;
		cv::Mat frame;
		Camera camera;
		std::vector<Model> models;
		unsigned int frameCount;
		unsigned int screenshotCount;

		Scene() : w(0), h(0), frameCount(0), screenshotCount(0)
		{
			// Set screenshot count to last value.
			std::string ssname = "screenshot_" + std::to_string(screenshotCount) + ".png";
			while (std::filesystem::exists(ssname))
				screenshotCount++;
		}
		void AddModel(std::string filename);
		void Draw();
	};
}