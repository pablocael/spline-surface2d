#pragma once

#include <vector>

#include <IParametricSurface.h>

#include "vec2.h"
#include "rect.h"
#include "spline.h"

struct State {
    rect rectangle;
    std::vector<double> surfacePoints;
};

// Represents a parametric surface that is composed by a grid of splines of which controlpoints are coincident
class ParametricSurfaceGrid : public IParametricSurface {
public:
    // Intialized the parametric surface using numControlPointsX * numControlPointsY control points.
    // Control points are initialize uniformily spaced, but can be manipulated using setControlPointPosition().
    // \param pixelWidth: the width of the grid, in pixels!
    // \param pixelHeight: the height of the grid, in pixels!
    // \param gridXControlPointResolution: the pixel resolution of controlpoints in between spacing in X axis
    // \param gridYControlPointResolution: the pixel resolution of controlpoints in between spacing in Y axis
    ParametricSurfaceGrid(const vec2d& pixelOrigin, double pixelWidth, double pixelHeight,
                          int gridXControlPointResolution, int gridYControlPointResolution);
    virtual vec2d surfacePoint(double u, double v);
    virtual vec2d surfacePoint(const vec2d& point);

    // Regenerate the grid in the given DPI resolution. This affects pixelWidth() and pixelHeight()
    int pixelWidth() { return _state.rectangle.width(); }
    int pixelHeight() { return _state.rectangle.height(); }
    void setPixelWidth(int width);
    void setPixelHeight(int height);
    void setPixelSize(int width, int height);
    void setGridResolution(int resX, int resY);
    void setGridResolutionX(int resX);
    void setGridResolutionY(int resY);
    void setControlPointPosition(int row, int col, const vec2d& point);
    void moveControlPoint(int row, int col, const vec2d& delta);
    //
    // Retrieves the control point in local space (relative to pixelOrigin())
    vec2d controlPointPosition(int row, int col);

    vec2d pixelOrigin() { return _state.rectangle.getOrigin(); }
    void setPixelOrigin(const vec2d& origin) { _state.rectangle.moveTo(origin); }
    int numControlPointsX() { return _numControlPointsX; }
    int numControlPointsY() { return _numControlPointsY; }
    tk::spline& rowSpline(int row) { return _splinesAlongY[row]; }
    tk::spline& colSpline(int col) { return _splinesAlongX[col]; }
    State& getState() { return _state; }
    // Generates the sample map between the rectangular pixel space and the surface space. Retrieves a vector containing
    // x,y positions for each pixel of the pixelWidth() x pixelHeight() grid.
    const std::vector<double>& generateSurfacePoints();


protected:
    void createGridData();
    void rebuildGridData(int gridXRes = 0, int gridYRes = 0, int gridWidth = 0, int gridHeight = 0);

protected:
    State _state;
    int _gridXControlPointResolution;
    int _gridYControlPointResolution;
    int _numControlPointsX;
    int _numControlPointsY;
    std::vector<tk::spline> _splinesAlongX;
    std::vector<tk::spline> _splinesAlongY;
};

