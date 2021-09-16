#include "../ParametricSurfaceGrid.h"

#include "../vec2.h"
#include <iostream>


int main() {

    ParametricSurfaceGrid* grid = new ParametricSurfaceGrid(vec2d(0,0), 100, 100, 10, 10);

    vec2d point = grid->surfacePoint(0.5,0.5) ;
    std::cout << point.x << ", " << point.y << std::endl;

    return 0;
}
