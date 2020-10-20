#include "Line.h"
#include <cmath>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

namespace SoftwareRasterizer
{
    Line::Line(int x1_, int y1_, int x2_, int y2_) {
        // Arrange points so that (x1,y1) is always above (ie less than) (x2,y2) 
        if (y1_ <= y2_) {
            this->p1 = Point(x1_, y1_);
            this->p2 = Point(x2_, y2_);
        }
        else {
            this->p1 = Point(x2_, y2_);
            this->p2 = Point(x1_, y1_);
        }
    }

    Line::Line(Point p1, Point p2){
        // Arrange points so that (x1,y1) is always above (x2,y2) 
        if (p1.y <= p2.y) {
            this->p1 = p1;
            this->p2 = p2;
        }
        else {
            this->p1 = p2;
            this->p2 = p1;
        }
    }

    Line::Line(glm::ivec2 p1, glm::ivec2 p2)
    {
        // Arrange points so that (x1,y1) is always above (x2,y2) 
        if (p1.y <= p2.y) {
            this->p1 = Point(p1.x,p1.y);
            this->p2 = Point(p2.x,p2.y);
        }
        else {
            this->p1 = Point(p2.x,p2.y);
            this->p2 = Point(p1.x,p1.y);
        }
    }


    // Helper function to convert color values to grayscale.
    int convertRGBtoGrayscaleLuminance(float color[3]) {
        float gamma = 2.2;
        float luminance = .2126 * pow(color[0], gamma) + .7152 * pow(color[1], gamma) + .0722 * pow(color[2], gamma);
        return 116 * pow(luminance, 1.0 / 3.0) - 16;
    }

    void setPixel(cv::Mat& data, int x, int y, float color[3], unsigned int thickness, bool isVertical) {
        //if at the edge of the image, stop drawing.
        if (
            y > data.rows - 1 ||
            x > data.cols - 1 ||
            y < 0 || x < 0
            ) {
            return;
        }

        //iterate over either x or y values, depending upon if line is more vertical than horizontal
        int startVal = y - thickness;
        int endVal = y + thickness;
        if (isVertical) {
            startVal = x - thickness;
            endVal = x + thickness;
        }
        for (int i = startVal; i < endVal; ++i) {

            //draw non-vertical pixels
            if (!isVertical) {
                //if 'i' is inside boundaries of image, draw line
                if (i >= 0 && i <= data.rows - 1) {
                    //if the image has 3 channels, color the pixel
                    if (data.channels() > 1) {
                        data.at<cv::Vec3f>(i, x) = cv::Vec3f(color[0],color[1],color[2]);
                    }
                    else {//if grayscale, convert to color to grayscale
                        data.at<float>(i, x) = convertRGBtoGrayscaleLuminance(color);
                    }
                }
            }
            // draw vertical pixels
            else {
                //if 'i' is inside boundaries of image, draw line
                if (i >= 0 && i <= data.cols - 1) {
                    //if the image has 3 channels, color the pixel
                    if (data.channels() > 1) {
                        data.at<cv::Vec3f>(y, i) = cv::Vec3f(color[0], color[1], color[2]);
                    }
                    else {//if grayscale, convert to color to grayscale
                        data.at<float>(y, i) = convertRGBtoGrayscaleLuminance(color);
                    }
                }
            }
        }
    }


    void Bresenham(Line this_, cv::Mat& data, float color[3], unsigned int thickness) {
        int dx, dy, i, e;
        int incx, incy, inc1, inc2;
        int x, y;

        // Return if coordinates are invalid, ie endpoints are identical.
        if (this_.p1.x == this_.p2.x && this_.p1.y == this_.p2.y) { return; }

        dx = this_.p2.x - this_.p1.x;
        dy = this_.p2.y - this_.p1.y;

        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        incx = 1;
        if (this_.p2.x < this_.p1.x) incx = -1;
        incy = 1;
        if (this_.p2.y < this_.p1.y) incy = -1;
        x = this_.p1.x; y = this_.p1.y;
        if (dx > dy) {
            setPixel(data, x, y, color, thickness, false);
            e = 2 * dy - dx;
            inc1 = 2 * (dy - dx);
            inc2 = 2 * dy;
            for (i = 0; i < dx; i++) {
                if (e >= 0) {
                    y += incy;
                    e += inc1;
                }
                else
                    e += inc2;
                x += incx;
                setPixel(data, x, y, color, thickness, false);
            }

        }
        else {
            setPixel(data, x, y, color, thickness, true);
            e = 2 * dx - dy;
            inc1 = 2 * (dx - dy);
            inc2 = 2 * dx;
            for (i = 0; i < dy; i++) {
                if (e >= 0) {
                    x += incx;
                    e += inc1;
                }
                else
                    e += inc2;
                y += incy;
                setPixel(data, x, y, color, thickness, true);
            }
        }
    }


    void EFLA(cv::Mat& data, int x, int y, int x2, int y2, float color[3], unsigned int thickness) {
        //// adapted from: http://www.edepot.com/linea.html
        bool yLonger = false;
        int incrementVal;
        int shortLen = y2 - y;
        int longLen = x2 - x;

        if (abs(shortLen) > abs(longLen)) {
            int swap = shortLen;
            shortLen = longLen;
            longLen = swap;
            yLonger = true;
        }

        if (longLen < 0) incrementVal = -1;
        else incrementVal = 1;

        double divDiff;
        if (shortLen == 0) divDiff = longLen;
        else divDiff = (double)longLen / (double)shortLen;
        if (yLonger) {
            for (int i = 0; i != longLen; i += incrementVal) {
                setPixel(data, x + (int)((double)i / divDiff), y + i, color, thickness, yLonger);
            }
        }
        else {
            for (int i = 0; i != longLen; i += incrementVal) {
                setPixel(data, x + i, y + (int)((double)i / divDiff), color, thickness, yLonger);
            }
        }
    }

    void EFLA2(cv::Mat& data, int x, int y, int x2, int y2, float color[3], unsigned int thickness) {
        //// adapted from: http://www.edepot.com/linea.html
        bool yLonger = false;
        int incrementVal;
        int shortLen = y2 - y;
        int longLen = x2 - x;

        if (abs(shortLen) > abs(longLen)) {
            int swap = shortLen;
            shortLen = longLen;
            longLen = swap;
            yLonger = true;
        }

        if (longLen < 0) incrementVal = -1;
        else incrementVal = 1;

        double divDiff;
        if (shortLen == 0) divDiff = longLen;
        else divDiff = (double)longLen / (double)shortLen;
        if (yLonger) {
            for (int i = 0; i != longLen / 2; i += incrementVal) {
                int x_1 = x + (int)((double)i / divDiff);
                int y_1 = y + i;
                int x_2 = x + (int)((double)(longLen - incrementVal - i) / divDiff);
                int y_2 = y + (longLen - incrementVal - i);
                setPixel(data, x_1, y_1, color, thickness, yLonger);
                setPixel(data, x_2, y_2, color, thickness, yLonger);
            }
        }
        else {
            for (int i = 0; i != longLen / 2; i += incrementVal) {
                int x_1 = x + i;
                int y_1 = y + (int)((double)i / divDiff);
                int x_2 = x + (longLen - incrementVal - i);
                int y_2 = y + (int)((double)(longLen - incrementVal - i) / divDiff);
                setPixel(data, x_1, y_1, color, thickness, yLonger);
                setPixel(data, x_2, y_2, color, thickness, yLonger);
            }
        }
    }

    void Wu(cv::Mat& data, int x0, int y0, int x1, int y1, float color[3], unsigned int thickness) {
        // Wu Line Algorithm adapted from: http://www.edepot.com/linewu.html
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;

        if (dy < 0) { dy = -dy;  stepy = -1; }
        else { stepy = 1; }
        if (dx < 0) { dx = -dx;  stepx = -1; }
        else { stepx = 1; }

        setPixel(data, x0, y0, color, thickness, 0);
        setPixel(data, x1, y1, color, thickness, 0);
        if (dx > dy) {
            int length = (dx - 1) >> 2;
            int extras = (dx - 1) & 3;
            int incr2 = (dy << 2) - (dx << 1);
            if (incr2 < 0) {
                int c = dy << 1;
                int incr1 = c << 1;
                int d = incr1 - dx;
                for (int i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d < 0) {                                                              // Pattern:
                        setPixel(data, x0, y0, color, thickness, 0);                          //
                        setPixel(data, x0 += stepx, y0, color, thickness, 0);                 //  x o o
                        setPixel(data, x1, y1, color, thickness, 0);                          //
                        setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                        d += incr1;
                    }
                    else {
                        if (d < c) {                                                          // Pattern:
                            setPixel(data, x0, y0, color, thickness, 0);                      //      o
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);    //  x o
                            setPixel(data, x1, y1, color, thickness, 0);                      //
                            setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0, y0 += stepy, color, thickness, 0);             // Pattern:
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);             //    o o 
                            setPixel(data, x1, y1 -= stepy, color, thickness, 0);             //  x
                            setPixel(data, x1 -= stepx, y1, color, thickness, 0);             //
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        setPixel(data, x0 += stepx, y0, color, thickness, 0);
                        if (extras > 1) setPixel(data, x0 += stepx, y0, color, thickness, 0);
                        if (extras > 2) setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                    }
                    else
                        if (d < c) {
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        }
                }
            }
            else {
                int c = (dy - dx) << 1;
                int incr1 = c << 1;
                int d = incr1 + dx;
                for (int i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d > 0) {
                        setPixel(data, x0, y0 += stepy, color, thickness, 0);                      // Pattern:
                        setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);             //      o
                        setPixel(data, x1, y1 -= stepy, color, thickness, 0);                      //    o
                        setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);	           //  x
                        d += incr1;
                    }
                    else {
                        if (d < c) {
                            setPixel(data, x0, y0, color, thickness, 0);                           // Pattern:
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);         //      o
                            setPixel(data, x1, y1, color, thickness, 0);                           //  x o
                            setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);         //
                        }
                        else {
                            setPixel(data, x0, y0 += stepy, color, thickness, 0);                  // Pattern:
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);                  //    o o
                            setPixel(data, x1, y1 -= stepy, color, thickness, 0);                  //  x
                            setPixel(data, x1 -= stepx, y1, color, thickness, 0);                  //
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                        if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                        if (extras > 2) setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                    }
                    else
                        if (d < c) {
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            if (extras > 2) {
                                if (d > c)
                                    setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                                else
                                    setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                            }
                        }
                }
            }
        }
        else {
            int length = (dy - 1) >> 2;
            int extras = (dy - 1) & 3;
            int incr2 = (dx << 2) - (dy << 1);
            if (incr2 < 0) {
                int c = dx << 1;
                int incr1 = c << 1;
                int d = incr1 - dy;
                for (int i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d < 0) {
                        setPixel(data, x0, y0, color, thickness, 0);
                        setPixel(data, x0, y0 += stepy, color, thickness, 0);
                        setPixel(data, x1, y1, color, thickness, 0);
                        setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                        d += incr1;
                    }
                    else {
                        if (d < c) {
                            setPixel(data, x0, y0, color, thickness, 0);
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            setPixel(data, x1, y1, color, thickness, 0);
                            setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            setPixel(data, x0, y0 += stepy, color, thickness, 0);
                            setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                            setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        setPixel(data, x0, y0 += stepy, color, thickness, 0);
                        if (extras > 1) setPixel(data, x0, y0 += stepy, color, thickness, 0);
                        if (extras > 2) setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                    }
                    else
                        if (d < c) {
                            setPixel(data, stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0, y0 += stepy, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        }
                }
            }
            else {
                int c = (dx - dy) << 1;
                int incr1 = c << 1;
                int d = incr1 + dy;
                for (int i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d > 0) {
                        setPixel(data, x0 += stepx, y0, color, thickness, 0);
                        setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                        setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                        setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        d += incr1;
                    }
                    else {
                        if (d < c) {
                            setPixel(data, x0, y0, color, thickness, 0);
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            setPixel(data, x1, y1, color, thickness, 0);
                            setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0, color, thickness, 0);
                            setPixel(data, x0, y0 += stepy, color, thickness, 0);
                            setPixel(data, x1 -= stepx, y1, color, thickness, 0);
                            setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                        if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                        if (extras > 2) setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                    }
                    else
                        if (d < c) {
                            setPixel(data, x0, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 2) setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                        }
                        else {
                            setPixel(data, x0 += stepx, y0 += stepy, color, thickness, 0);
                            if (extras > 1) setPixel(data, x0, y0 += stepy, color, thickness, 0);
                            if (extras > 2) {
                                if (d > c)
                                    setPixel(data, x1 -= stepx, y1 -= stepy, color, thickness, 0);
                                else
                                    setPixel(data, x1, y1 -= stepy, color, thickness, 0);
                            }
                        }
                }
            }
        }
    }

    // Rounding helper function
    int rd(const float a) { return int(a + 0.5); }

    void DDA(cv::Mat& data, Line l, float color[3], unsigned int thickness) {
        // DDA Line Algorithm
        // adapted from: http://www.edepot.com/linedda.html

        int length = std::abs(l.p2.x - l.p1.x);
        if (std::abs(l.p2.y - l.p1.y) > length) { length = std::abs(l.p2.y - l.p1.y); }
        double xincrement = (double)(l.p2.x - l.p1.x) / (double)length;
        double yincrement = (double)(l.p2.y - l.p1.y) / (double)length;
        double x = l.p1.x + 0.5;
        double y = l.p1.y + 0.5;

        for (int i = 1; i <= length; ++i) {
            setPixel(data, (int)x, (int)y, color, thickness, (x >= y));
            x += xincrement;
            y += yincrement;
        }
    }

    void Line::draw(cv::Mat& data, float color[3], unsigned int thickness, LINE_ALGORITHM method) {
        
        // Case: line is a point.
        if (p1.x == p2.x && p1.y == p2.y)
        {
            setPixel(data,p1.x,p1.y,color,thickness,false);
            return;
        }

        // Case: line is purely vertical. Note: due to constructor, p1.y is always <= p2.y.
        if (p1.x == p2.x && p1.y != p2.y)
        {
            for (int i = p1.y; i < p2.y; ++i)            
                setPixel(data, p1.x, i, color, thickness, false); 
            return;
        }

        // Case: line is purely horizontal.
        if (p1.x != p2.x && p1.y == p2.y)
        {
            int min_x = p1.x < p2.x ? p1.x : p2.x;
            int max_x = p1.x > p2.x ? p1.x : p2.x;
            for (int i = min_x; i < max_x; ++i)
                setPixel(data, i, p1.y, color, thickness, false);
            return;
        }
        
        // Else, use rasterization algorithm.
        switch (method) {
        case LINE_ALGORITHM::BRESENHAM:
            Bresenham(*this, data, color, thickness);
            break;
        case  LINE_ALGORITHM::EFLA:
            EFLA(data, this->p1.x, this->p1.y, this->p2.x, this->p2.y, color, thickness);
            break;
        case  LINE_ALGORITHM::EFLA2:
            EFLA2(data, this->p1.x, this->p1.y, this->p2.x, this->p2.y, color, thickness);
            break;
        case  LINE_ALGORITHM::WU:
            Wu(data, this->p1.x, this->p1.y, this->p2.x, this->p2.y, color, thickness);
            break;
        case  LINE_ALGORITHM::DDA:
            DDA(data, *this, color, thickness);
            break;
        }
    }
}