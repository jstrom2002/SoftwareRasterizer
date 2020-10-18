#include "UnitTests.h"
#include "Model.h"
#include <iostream>
#include <string>

SoftwareRasterizer::SoftwareRasterizerUnitTests tests;

int main(int argc, char** argv) 
{
	// If args are insufficient, run test mode.
	if (argc < 4)
	{
		//tests.LineAlgSpeedTest();
		tests.RenderTest();
	}

	// Use args to load and render given model file. NOTE: Arguments must be 
	// of format 'SoftwareRasterizer.exe [OBJ file path] [window width] [window height]
	// [position x] [position y] [position z] [scale] [rotation x] [rotation y] [rotation z].'"
	else
	{		
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
		SoftwareRasterizer::Model model(argv[1]);
		model.Draw(std::stoi(argv[2]), std::stoi(argv[3]), pos, scaleVal, rot);
	}
}