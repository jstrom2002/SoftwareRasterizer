#pragma once
#include "Camera.h"
#include <vector>
#include <filesystem>
#include <ctime>
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
	class Camera;
	class Model;

	class Scene
	{
	public:
		int w, h;
		cv::Mat frame, frameZ;
		Camera camera;
		std::vector<Model> models;
		unsigned int frameCount;
		unsigned int screenshotCount;
		bool windowClose;
		bool showFPS;
		bool showDepth;
		bool wireframeOn;
		char keyPressed;

		Scene();
		~Scene();
		void AddModel(std::string filename);
		void Draw();

	private:
		clock_t startFrameTime, endFrameTime;
		void ProcessInput(char c);
	};
}