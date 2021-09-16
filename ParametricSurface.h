#pragma once
#include <QPointF>
class IParametricSurface {
public:
    virtual QPointF surfacePoint(double x, double y) = 0;
    virtual QPointF surfacePoint(const QPointF& point) = 0;
};
