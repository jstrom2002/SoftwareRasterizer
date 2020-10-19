#include "Point.h"
#include <utility>
#include <sstream>
#include <limits>

namespace SoftwareRasterizer
{
    Point::Point(int x_, int y_) : x(x_), y(y_) {}

    Point::Point(cv::Point p) : x(p.x), y(p.y) {}


    void Point::operator=(Point p) {
        x = p.x;
        y = p.y;
    }

    bool Point::operator==(const Point& p) {
        if (this->x == p.x && this->y == p.y) {
            return true;
        }
        else {
            return false;
        }
    }

    std::string Point::to_string() {
        std::stringstream ss;
        ss << "(" << x << "," << y << ")";
        std::string str = ss.str();
        ss.clear();
        return str;
    }
}