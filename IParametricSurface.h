#pragma once

#include "vec2.h" 

class IParametricSurface {
public:
    virtual vec2d surfacePoint(double u, double v) = 0;
    virtual vec2d surfacePoint(const vec2d& point) = 0;
};
