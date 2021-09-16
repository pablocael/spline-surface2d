#pragma once
#include <QPointF>
#include <vector>

#include "IParametricSurface.h"
#include "ParametricSurfaceGridAction.h"
#include "spline.h"

struct State {
    QRectF rectangle;
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
    ParametricSurfaceGrid(const QPointF& pixelOrigin, double pixelWidth, double pixelHeight,
                          int gridXControlPointResolution, int gridYControlPointResolution);
    virtual QPointF surfacePoint(double u, double v);
    virtual QPointF surfacePoint(const QPointF& point);

    // Regenerate the grid in the given DPI resolution. This affects pixelWidth() and pixelHeight()
    int pixelWidth() { return _state.rectangle.width(); }
    int pixelHeight() { return _state.rectangle.height(); }
    void setPixelWidth(int width);
    void setPixelHeight(int height);
    void setPixelSize(int width, int height);
    void setGridResolution(int resX, int resY);
    void setGridResolutionX(int resX);
    void setGridResolutionY(int resY);
    void setControlPointPosition(int row, int col, const QPointF& point);
    void moveControlPoint(int row, int col, const QPointF& delta);
    //
    // Retrieves the control point in local space (relative to pixelOrigin())
    QPointF controlPointPosition(int row, int col);

    QPointF pixelOrigin() { return _state.rectangle.topLeft(); }
    void setPixelOrigin(const QPointF& origin) { _state.rectangle.moveTo(origin); }
    int numControlPointsX() { return _numControlPointsX; }
    int numControlPointsY() { return _numControlPointsY; }
    tk::spline& rowSpline(int row) { return _splinesAlongY[row]; }
    tk::spline& colSpline(int col) { return _splinesAlongX[col]; }
    State& getState() { return _state; }
    // Generates the sample map between the rectangular pixel space and the surface space. Retrieves a vector containing
    // x,y positions for each pixel of the pixelWidth() x pixelHeight() grid.
    const std::vector<double>& generateSurfacePoints();


protected:
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

