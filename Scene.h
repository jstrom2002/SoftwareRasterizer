#pragma once
#include "Camera.h"
#include <vector>
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

		Scene() : w(0), h(0), frameCount(0) {}
		void AddModel(std::string filename);
		void Draw();
	};
}