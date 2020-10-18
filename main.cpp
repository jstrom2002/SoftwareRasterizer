#include "UnitTests.h"
#include "Scene.h"
#include "Model.h"
#include <glm/glm.hpp>
#include <iostream>
#include <string>

SoftwareRasterizer::SoftwareRasterizerUnitTests tests;
SoftwareRasterizer::Scene scene;

int main(int argc, char** argv) 
{
	// If args are insufficient, run test mode.
	if (1)//argc < 4)
	{
		//tests.LineAlgSpeedTest();
		tests.RenderTest();
	}

	// Use args to load and render given model file. NOTE: Arguments must be 
	// of format 'SoftwareRasterizer.exe [OBJ file path] [window width] [window height]
	// [position x] [position y] [position z] [scale] [rotation x] [rotation y] [rotation z].'"
	else
	{		
		// Set scene frame width/height.
		scene.w = std::stoi(argv[2]);
		scene.h = std::stoi(argv[3]);
		
		// Load model with appropriate transforms.
		glm::vec3 pos(0);
		glm::vec3 rot(0);
		float scaleVal=1;
		if (argc >= 7)
			pos = glm::vec3(std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
		if (argc >= 8)
			scaleVal = std::stof(argv[7]);
		if (argc >= 11)
			rot = glm::vec3(std::stof(argv[8]), std::stof(argv[9]), std::stof(argv[10]));

		std::cout << "Loading object file " << argv[1] << std::endl;
		scene.AddModel(argv[1]);
		scene.models[0].position = pos;
		scene.models[0].rotation = rot;
		scene.models[0].scale = scaleVal;

		// Draw scene.
		scene.Draw();
	}
}