/*
*	Line.h -- header for line drawing/traversal using various line rasterization algorithms.
*	Line raster code adapted from: http://www.edepot.com/algorithm.html
*/

#pragma once
#include "Point.h"
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

namespace SoftwareRasterizer
{
	enum class LINE_ALGORITHM {
		BRESENHAM,
		EFLA,
		EFLA2,
		WU,
		DDA
	};

	class Line {
	public:
		Point p1, p2;

		Line() : p1(0, 0), p2(0, 0) {}
		~Line() {}

		/*!
		* \brief Initializers of a line from integer values.
		*/
		Line(int x1_, int y1_, int x2_, int y2_);
		Line(Point p1, Point p2);
		Line(glm::ivec2 p1, glm::ivec2 p2);

		/*!
		*  \brief Draws rasterized points along a line on an image.
		*
		* \param [in] data An image which the line will color during its traversal.
		* \param [in] color The value for which each pixel along the line will be colored.
		* \param [in] thickness The number of neighbor pixels also colored along the line.
		*/
		void draw(cv::Mat& data, float color[3], unsigned int thickness = 1, 
			LINE_ALGORITHM method = LINE_ALGORITHM::BRESENHAM);
	};
}