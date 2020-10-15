#pragma once
#include <string>

namespace SoftwareRasterizer
{
	/*!
	*  \brief Simple struct for holding point values to prevent using raw arrays.
	*/
	class Point {
	public:
		int x, y;

		Point() {}
		Point(int x_, int y_);

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