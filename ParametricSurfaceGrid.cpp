#include "ParametricSurfaceGrid.h"

#include <cmath>
#include <iostream>
QPointF interpolateBetweenU(double t, double y_sp0, double y_sp1, const tk::spline& sp0, const tk::spline& sp1)
{
    QPointF p0(sp0(y_sp0), y_sp0);
    QPointF p1(sp1(y_sp1), y_sp1);
    return (1 - t) * p0 + t * p1;
}
QPointF interpolateBetweenV(double t, double x_sp0, double x_sp1, const tk::spline& sp0, const tk::spline& sp1)
{
    QPointF p0(x_sp0, sp0(x_sp0));
    QPointF p1(x_sp1, sp1(x_sp1));
    return (1 - t) * p0 + t * p1;
}
// corner parameters are numbered according to XY variation, for instance, corner00 is smaller x and smaller y, corner01
// is smaller x and bigger y and so on..
QPointF generateSplinePatch(double nu, double nv, const tk::spline& spU0, const tk::spline& spU1,
                            const tk::spline& spV0, const tk::spline& spV1, const QPointF& corner00,
                            const QPointF& corner01, const QPointF& corner10, const QPointF& corner11)
{
    // interpolate patches using Coon's Patch
    QPointF result;
    QPointF B =
        corner00 * (1 - nv) * (1 - nu) + corner01 * nv * (1 - nu) + corner10 * (1 - nv) * nu + corner11 * nv * nu;

    QPointF Lc = interpolateBetweenU(nu, corner00.y() * (1 - nv) + nv * corner01.y(),
                                     corner10.y() * (1 - nv) + nv * corner11.y(), spU0, spU1);
    result = Lc;
    result += interpolateBetweenV(nv, corner00.x() * (1 - nu) + nu * corner10.x(),
                                  corner01.x() * (1 - nu) + nu * corner11.x(), spV0, spV1);
    result -= B;
    return result;
}

ParametricSurfaceGrid::ParametricSurfaceGrid(const QPointF& pixelOrigin, double sizeWidth, double sizeHeight,
                                             int gridXControlPointResolution, int gridYControlPointResolution)
{
    _state.rectangle = QRectF(pixelOrigin, QSize(sizeWidth, sizeHeight));
    _gridXControlPointResolution = std::max(5, gridXControlPointResolution);
    _gridYControlPointResolution = std::max(5, gridYControlPointResolution);

    createGridData();
}

void ParametricSurfaceGrid::setPixelWidth(int width)
{

    _state.rectangle.setWidth(std::max(20, width));
    createGridData();
}
void ParametricSurfaceGrid::setPixelHeight(int height)
{
    _state.rectangle.setHeight(std::max(20, height));
    createGridData();
}

void ParametricSurfaceGrid::setPixelSize(int width, int height)
{
    _state.rectangle.setSize(QSize(std::max(20, width), std::max(20, height)));
    createGridData();
}

void ParametricSurfaceGrid::setControlPointPosition(int row, int col, const QPointF& point)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);
    QPointF oldPosition = controlPointPosition(row, col);
    _splinesAlongY[row].set_point(col, point.x(), point.y());
    _splinesAlongX[col].set_point(row, point.y(), point.x());
    QPointF newPosition = controlPointPosition(row, col);
}

void ParametricSurfaceGrid::moveControlPoint(int row, int col, const QPointF& delta)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);
    QPointF oldPosition = controlPointPosition(row, col);
    _splinesAlongY[row].move_point(col, delta.x(), delta.y());
    _splinesAlongX[col].move_point(row, delta.y(), delta.x());
    QPointF newPosition = controlPointPosition(row, col);
}

QPointF ParametricSurfaceGrid::controlPointPosition(int row, int col)
{
    assert(row < _splinesAlongY.size() && _splinesAlongY[row].getNumPoints() > col);
    assert(col < _splinesAlongX.size() && _splinesAlongX[col].getNumPoints() > row);

    double x, y;
    _splinesAlongY[row].get_point(col, x, y);
    return QPointF(x, y);
}

const std::vector<double>& ParametricSurfaceGrid::generateSurfacePoints()
{
    int width = _state.rectangle.width();
    int height = _state.rectangle.height();

    QPointF gridOrigin = pixelOrigin();
    _state.surfacePoints.resize(width * height * 2);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double u = x / (double)width;
            double v = y / (double)height;
            QPointF surfacepoint = surfacePoint(u, v);
            _state.surfacePoints[2 * y * width + 2 * x + 0] = surfacepoint.x() + gridOrigin.x();
            _state.surfacePoints[2 * y * width + 2 * x + 1] = surfacepoint.y() + gridOrigin.y();
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
            QPointF point = surfacePoint(u, v);
            x.push_back(point.x());
            y.push_back(point.y());
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
            QPointF point = surfacePoint(u, v);
            x.push_back(point.y());
            y.push_back(point.x());
        }
        spline.set_points(x, y);
        splinesAlongX.push_back(spline);
    }
    _state.rectangle.setSize(QSize(std::max(20, newWidth), std::max(20, newHeight)));
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

QPointF ParametricSurfaceGrid::surfacePoint(double u, double v)
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
    QPointF p00 = controlPointPosition(row, col);
    QPointF p10 = controlPointPosition(row, col1);
    QPointF p11 = controlPointPosition(row1, col1);
    QPointF p01 = controlPointPosition(row1, col);
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

QPointF ParametricSurfaceGrid::surfacePoint(const QPointF& point) { return surfacePoint(point.x(), point.y()); }

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


