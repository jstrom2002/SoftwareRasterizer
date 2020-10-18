#include "UnitTests.h"
#include "Scene.h"
#include "Model.h"
#include <glm/glm.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) 
{
	// If args are insufficient, run test mode.
	if (argc < 4)
	{
		SoftwareRasterizer::SoftwareRasterizerUnitTests tests;
		//tests.LineAlgSpeedTest();
		tests.RenderTest();
	}

	// Use args to load and render given model file. NOTE: Arguments must be 
	// of format 'SoftwareRasterizer.exe [window width] [window height],' then
	// as many arguments as you like of the format '[OBJ file path] [position x] 
	// [position y] [position z] [scale] [rotation x] [rotation y] [rotation z].'
	else
	{		
		SoftwareRasterizer::Scene scene;

		// Set scene frame width/height.
		scene.w = std::stoi(argv[1]);
		scene.h = std::stoi(argv[2]);
		
		// Load models with appropriate transforms.
		int modelCounter = 0;
		glm::vec3 pos(0);
		glm::vec3 rot(0);
		float scaleVal = 1;
		for (int i = 0; i < argc; i += 10)
		{
			if (argc >= i+7)
				pos = glm::vec3(std::stoi(argv[4+i]), std::stoi(argv[5+i]), std::stoi(argv[6+i]));
			if (argc >= i+8)
				scaleVal = std::stof(argv[7+i]);
			if (argc >= i+11)
				rot = glm::vec3(std::stof(argv[8+i]), std::stof(argv[9+i]), std::stof(argv[10+i]));

			std::cout << "Loading object file " << argv[3+i] << std::endl;
			scene.AddModel(argv[3+i]);
			scene.models[modelCounter].position = pos;
			scene.models[modelCounter].rotation = rot;
			scene.models[modelCounter].scale = scaleVal;
			modelCounter++;
		}

		// Draw scene.
		scene.Draw();
	}
}