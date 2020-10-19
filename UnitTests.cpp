#include "UnitTests.h"
#include "Point.h"
#include "Line.h"
#include "Scene.h"
#include "Model.h"

#include <iostream>
#include <vector>
#include <ctime>

namespace SoftwareRasterizer
{
	bool SoftwareRasterizerUnitTests::LineAlgSpeedTest()
	{
		// Initialize variables and objects for tests.
		srand(clock());
		cv::Mat img1 = cv::Mat::zeros(1000, 1000, CV_32F);
		cv::Mat img2 = cv::Mat::zeros(1000, 1000, CV_32F);
		cv::Mat img3 = cv::Mat::zeros(1000, 1000, CV_32F);
		cv::Mat img4 = cv::Mat::zeros(1000, 1000, CV_32F);
		cv::Mat img5 = cv::Mat::zeros(1000, 1000, CV_32F);
		clock_t clock1, clock2;
		float color[3] = { 255,255,255 };
		int numLines = 50;
		int numTests = 3;
		double test1 = 0;
		double test2 = 0;
		double test3 = 0;
		double test4 = 0;
		double test5 = 0;

		// Generate an array of random lines for tests. 
		std::vector<SoftwareRasterizer::Line> lines;
		for (int i = 0; i < numLines; ++i) {
			lines.push_back(SoftwareRasterizer::Line(
				SoftwareRasterizer::Point(rand() % 1000, rand() % 1000),
				SoftwareRasterizer::Point(rand() % 1000, rand() % 1000)));
		}

		// Run multiple tests each line rasterizing algorithm and average the results.
		for (int i = 0; i < numTests; ++i) {
			clock1 = clock();
			for (int i = 0; i < lines.size(); ++i) { lines[i].draw(img1, color, 7, SoftwareRasterizer::LINE_ALGORITHM::BRESENHAM); }
			clock2 = clock();
			test1 += (double(clock2 - clock1) / CLOCKS_PER_SEC) / numLines;

			clock1 = clock();
			for (int i = 0; i < lines.size(); ++i) { lines[i].draw(img2, color, 7, SoftwareRasterizer::LINE_ALGORITHM::EFLA); }
			clock2 = clock();
			test2 += (double(clock2 - clock1) / CLOCKS_PER_SEC) / numLines;
		
			clock1 = clock();		
			for (int i = 0; i < lines.size(); ++i) { lines[i].draw(img2, color, 7, SoftwareRasterizer::LINE_ALGORITHM::EFLA2); }
			clock2 = clock();
			test2 += (double(clock2 - clock1) / CLOCKS_PER_SEC) / numLines;

			clock1 = clock();
			for (int i = 0; i < lines.size(); ++i) { lines[i].draw(img3, color, 7, SoftwareRasterizer::LINE_ALGORITHM::WU); }
			clock2 = clock();
			test3 += (double(clock2 - clock1) / CLOCKS_PER_SEC) / numLines;

			clock1 = clock();
			for (int i = 0; i < lines.size(); ++i) { lines[i].draw(img4, color, 7, SoftwareRasterizer::LINE_ALGORITHM::DDA); }
			clock2 = clock();
			test4 += (double(clock2 - clock1) / CLOCKS_PER_SEC) / numLines;
		}

		// Display times for completion.
		std::cout << "time Bresenham: " << test1 / numTests << " sec per line\n";
		std::cout << "time EFLA: " << test2 / numTests << " sec per line\n";
		std::cout << "time Wu: " << test3 / numTests << " sec per line\n";
		std::cout << "time DDA: " << test4 / numTests << " sec per line\n";

		// Display all results visually.	
		cv::imshow("Bresenham", img1);
		cv::imshow("EFLA", img2);
		cv::imshow("EFLA2", img3);
		cv::imshow("Wu", img4);
		cv::imshow("DDA", img5);
		cv::waitKey();

		return true;
	}
	

	bool SoftwareRasterizerUnitTests::RenderTest()
	{
		// Initialize vars for rendering.
		Scene scene;
		scene.w = 800;
		scene.h = 600;		
		glm::vec3 rotation = glm::vec3(0.1f, 0.1f, 0.0f);		

		// Add test model 1.
		scene.AddModel("models/face.obj");
		scene.models[0].position = glm::vec3(-0.2f, 0.0f, -1.0f);
		scene.models[0].scale = 1.0f;
		scene.models[0].rotation = rotation;

		// Add test model 2.
		scene.AddModel("models/cube.obj");
		scene.models[1].position = glm::vec3(0.15f, 0.0f, -1.0f);
		scene.models[1].scale = 0.185f;
		scene.models[1].rotation = rotation;

		// Draw all.
		scene.Draw();
		return true;
	}
}
