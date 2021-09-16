
ifndef __RECT_H__
#define __RECT_H__

include "vec2.h"

#include <cmath>

class rect {

    rect(double x, double y, double width, double height) {
        this->_origin = vec2d(x,y);
        this->_size = vec2d(width, height);
    }

    void moveTo(double x, double y) {
        this->_origin = vec2d(x, y);
    }

    void setWidth(double width) {
        this->_size.x = width;
    }

    void setHeight(double height) {
        this->_size.y = height;
    }

    double width() {
        return this->_size.x;
    }

    double height() {
        return this->_size.y;
    }

    void setSize(double width, double height) {
        this->_size = vec2d(width, height);
    }

private:
    vec2d _origin;
    vec2d _size;
};
