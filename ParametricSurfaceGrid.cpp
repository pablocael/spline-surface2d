#include "ParametricSurfaceGrid.h"

#include <cmath>
#include <iostream>

#include "rect.h"

vec2d interpolateBetweenU(double t, double y_sp0, double y_sp1, const tk::spline& sp0, const tk::spline& sp1)
{
    vec2d p0(sp0(y_sp0), y_sp0);
    vec2d p1(sp1(y_sp1), y_sp1);
    return  p0 * (1 - t) + p1 * t;
}
vec2d interpolateBetweenV(double t, double x_sp0, double x_sp1, const tk::spline& sp0, const tk::spline& sp1)
{
    vec2d p0(x_sp0, sp0(x_sp0));
    vec2d p1(x_sp1, sp1(x_sp1));
    return p0 * (1 - t) + p1 * t;
}
// corner parameters are numbered according to XY variation, for instance, corner00 is smaller x and smaller y, corner01
// is smaller x and bigger y and so on..
vec2d generateSplinePatch(double nu, double nv, const tk::spline& spU0, const tk::spline& spU1,
                            const tk::spline& spV0, const tk::spline& spV1, const vec2d& corner00,
                            const vec2d& corner01, const vec2d& corner10, const vec2d& corner11)
{
    // interpolate patches using Coon's Patch
    vec2d result;
    vec2d B =
        corner00 * (1 - nv) * (1 - nu) + corner01 * nv * (1 - nu) + corner10 * (1 - nv) * nu + corner11 * nv * nu;

    vec2d Lc = interpolateBetweenU(nu, corner00.y * (1 - nv) + nv * corner01.y,
                                     corner10.y * (1 - nv) + nv * corner11.y, spU0, spU1);
    result = Lc;
    result += interpolateBetweenV(nv, corner00.x * (1 - nu) + nu * corner10.x,
                                  corner01.x * (1 - nu) + nu * corner11.x, spV0, spV1);
    result -= B;
    return result;
}

ParametricSurfaceGrid::ParametricSurfaceGrid(const vec2d& pixelOrigin, double sizeWidth, double sizeHeight,
                                             int gridXControlPointResolution, int gridYControlPointResolution)
{
    _state.rectangle = rect(pixelOrigin, sizeWidth, sizeHeight);
    _gridXControlPointResolution = std::max(5, gridXControlPointResolution);
    _gridYControlPointResolution = std::max(5, gridYControlPointResolution);

    createGridData();
}

void ParametricSurfaceGrid::setPixelWidth(int width)
{

    _state.rectangle.setWidth(std::max(1, width));
    createGridData();
}
void ParametricSurfaceGrid::setPixelHeight(int height)
{
    _state.rectangle.setHeight(std::max(1, height));
    createGridData();
}

void ParametricSurfaceGrid::setPixelSize(int width, int height)
{
    _state.rectangle.setSize(std::max(1, width), std::max(1, height));
    createGridData();
}

void ParametricSurfaceGrid::setControlPointPosition(int row, int col, const vec2d& point)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);
    vec2d oldPosition = controlPointPosition(row, col);
    _splinesAlongY[row].set_point(col, point.x, point.y);
    _splinesAlongX[col].set_point(row, point.y, point.x);
    vec2d newPosition = controlPointPosition(row, col);
}

void ParametricSurfaceGrid::moveControlPoint(int row, int col, const vec2d& delta)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);
    vec2d oldPosition = controlPointPosition(row, col);
    _splinesAlongY[row].move_point(col, delta.x, delta.y);
    _splinesAlongX[col].move_point(row, delta.y, delta.x);
    vec2d newPosition = controlPointPosition(row, col);
}

vec2d ParametricSurfaceGrid::controlPointPosition(int row, int col)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);

    double x, y;
    _splinesAlongY[row].get_point(col, x, y);
    return vec2d(x, y);
}

const std::vector<double>& ParametricSurfaceGrid::generateSurfacePoints()
{
    int width = _state.rectangle.width();
    int height = _state.rectangle.height();

    vec2d gridOrigin = pixelOrigin();
    _state.surfacePoints.resize(width * height * 2);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double u = x / (double)width;
            double v = y / (double)height;
            vec2d surfacepoint = surfacePoint(u, v);
            _state.surfacePoints[2 * y * width + 2 * x + 0] = surfacepoint.x + gridOrigin.x;
            _state.surfacePoints[2 * y * width + 2 * x + 1] = surfacepoint.y + gridOrigin.y;
        }
    }
    return _state.surfacePoints;
}

void ParametricSurfaceGrid::rebuildGridData(int gridXRes, int gridYRes, int gridWidth, int gridHeight)
{
    if (_splinesAlongX.size() == 0 || _splinesAlongY.size() == 0) {
        return createGridData();
    }
    std::vector<tk::spline> splinesAlongX;
    std::vector<tk::spline> splinesAlongY;

    int newWidth = gridWidth > 0 ? gridWidth : _state.rectangle.width();
    int newHeight = gridHeight > 0 ? gridHeight : _state.rectangle.height();
    int newResX = gridXRes > 0 ? gridXRes : _gridXControlPointResolution;
    int newResY = gridYRes > 0 ? gridYRes : _gridYControlPointResolution;
    int numControlPointsX = std::max<int>(3, 1 + std::ceil(newWidth / (float)newResX));
    int numControlPointsY = std::max<int>(3, 1 + std::ceil(newHeight / (float)newResY));

    // create horizontal splines, along y axis
    for (int j = 0; j < numControlPointsY; ++j) {
        int ycoord = std::min(newHeight, j * newResY);
        bool isLinear = rowSpline(j).isLinear();
        tk::spline spline(isLinear);
        double v = ycoord / (double)newHeight;
        std::vector<double> x, y;
        for (int i = 0; i < numControlPointsX; ++i) {
            int xcoord = std::min(newWidth, i * newResX);
            double u = xcoord / (double)newWidth;
            vec2d point = surfacePoint(u, v);
            x.push_back(point.x);
            y.push_back(point.y);
        }
        spline.set_points(x, y);
        splinesAlongY.push_back(spline);
    }

    for (int j = 0; j < numControlPointsX; ++j) {
        int xcoord = std::min(newWidth, j * newResX);
        bool isLinear = colSpline(j).isLinear();
        tk::spline spline(isLinear);
        std::vector<double> x, y;
        double u = xcoord / (double)newWidth;
        for (int i = 0; i < numControlPointsY; ++i) {
            int ycoord = std::min(newHeight, i * newResY);
            double v = ycoord / (double)newHeight;
            vec2d point = surfacePoint(u, v);
            x.push_back(point.y);
            y.push_back(point.x);
        }
        spline.set_points(x, y);
        splinesAlongX.push_back(spline);
    }
    _state.rectangle.setSize(std::max(20, newWidth), std::max(20, newHeight));
    _gridXControlPointResolution = newResX;
    _gridYControlPointResolution = newResY;
    _numControlPointsX = numControlPointsX;
    _numControlPointsY = numControlPointsY;
    _splinesAlongX = splinesAlongX;
    _splinesAlongY = splinesAlongY;
    assert(_splinesAlongY.size() == _numControlPointsY);
    assert(_splinesAlongX.size() == _numControlPointsX);
}

void ParametricSurfaceGrid::createGridData()
{
    _splinesAlongX.clear();
    _splinesAlongY.clear();
    int width = pixelWidth();
    int height = pixelHeight();
    _numControlPointsX = std::max<int>(3, 1 + std::ceil(width / (float)_gridXControlPointResolution));
    _numControlPointsY = std::max<int>(3, 1 + std::ceil(height / (float)_gridYControlPointResolution));

    // create horizontal splines, along y axis
    for (int j = 0; j < _numControlPointsY; ++j) {
        int ycoord = std::min(height, j * _gridYControlPointResolution);
        tk::spline spline(false);
        std::vector<double> x, y;
        for (int i = 0; i < _numControlPointsX; ++i) {
            int xcoord = std::min(width, i * _gridXControlPointResolution);
            x.push_back(xcoord);
            y.push_back(ycoord);
        }
        spline.set_points(x, y);
        _splinesAlongY.push_back(spline);
    }

    for (int j = 0; j < _numControlPointsX; ++j) {
        int xcoord = std::min(width, j * _gridXControlPointResolution);
        tk::spline spline(false);
        std::vector<double> x, y;
        for (int i = 0; i < _numControlPointsY; ++i) {
            int ycoord = std::min(height, i * _gridYControlPointResolution);
            x.push_back(ycoord);
            y.push_back(xcoord);
        }
        spline.set_points(x, y);
        _splinesAlongX.push_back(spline);
    }
    assert(_splinesAlongY.size() == _numControlPointsY);
    assert(_splinesAlongX.size() == _numControlPointsX);
}

vec2d ParametricSurfaceGrid::surfacePoint(double u, double v)
{
    // first step is to know which 4 splines to use, depending on where u,v coordinates are
    int width = pixelWidth();
    int height = pixelHeight();
    double coordRow = (v * height / _gridYControlPointResolution);
    double coordCol = (u * width / _gridXControlPointResolution);
    int row = std::floor(coordRow);
    int col = std::floor(coordCol);
    int col1 = std::ceil(coordCol);
    int row1 = std::ceil(coordRow);

    const tk::spline& spU0 = _splinesAlongX[col];
    const tk::spline& spU1 = _splinesAlongX[col1];
    const tk::spline& spV0 = _splinesAlongY[row];
    const tk::spline& spV1 = _splinesAlongY[row1];
    vec2d p00 = controlPointPosition(row, col);
    vec2d p10 = controlPointPosition(row, col1);
    vec2d p11 = controlPointPosition(row1, col1);
    vec2d p01 = controlPointPosition(row1, col);
    double nv = coordRow - row;
    double nu = coordCol - col;
    if (row1 == _splinesAlongY.size() - 1) {
        double intpart;
        double s = std::modf(height / (double)_gridYControlPointResolution, &intpart);
        if (s > 0) {
            nv /= s;
        }
    }
    if (col1 == _splinesAlongX.size() - 1) {
        double intpart;
        double s = modf(width / (double)_gridXControlPointResolution, &intpart);
        if (s > 0) {
            nu /= s;
        }
    }

    return generateSplinePatch(nu, nv, spU0, spU1, spV0, spV1, p00, p01, p10, p11);
}

vec2d ParametricSurfaceGrid::surfacePoint(const vec2d& point) { return surfacePoint(point.x, point.y); }

void ParametricSurfaceGrid::setGridResolution(int resX, int resY)
{
    rebuildGridData(resX, resY);
}

void ParametricSurfaceGrid::setGridResolutionY(int resY)
{
    setGridResolution(_gridXControlPointResolution, resY);
}

void ParametricSurfaceGrid::setGridResolutionX(int resX)
{
    setGridResolution(resX, _gridYControlPointResolution);
}


