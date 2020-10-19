/**
*  Point.h -- This class is intended to represent 2D points for drawing on a planar image using 
*			  the 'Line' class. If intending to represent 3D points, use the 'Vertex' class.
*/

#pragma once
#include <string>
#include <opencv2/opencv.hpp>

namespace SoftwareRasterizer
{
	class Point {
	public:
		int x, y;

		Point() : x(0), y(0) {}
		Point(int x_, int y_);
		Point(cv::Point p);

		Point operator+(const Point& p) {
			return Point(this->x + p.x, this->y + p.y);
		}
		template <class T>
		Point operator+(const T n) {
			return Point(this->x + n, this->y + n);
		}
		Point operator-(const Point& p) {
			return Point(this->x - p.x, this->y - p.y);
		}
		template <class T>
		Point operator-(const T n) {
			return Point(this->x - n, this->y - n);
		}
		Point operator/(const Point& p) {
			return Point(this->x / p.x, this->y / p.y);
		}
		template <class T>
		Point operator/(const T n) {
			return Point(this->x / n, this->y / n);
		}
		Point operator*(const Point& p) {
			return Point(this->x * p.x, this->y * p.y);
		}
		template <class T>
		Point operator*(const T n) {
			return Point(this->x * n, this->y * n);
		}

		void operator=(Point p);

		bool operator==(const Point& p);

		/*!
		*  \brief Prints the point's fields to a string in the format "(x,y)".
		*/
		std::string to_string();
	};

}