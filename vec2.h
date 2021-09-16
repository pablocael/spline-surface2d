#ifndef __VEC2_H__
#define __VEC2_H__

#include <cmath>


template <class T>
class vec2 {
public:
    T x, y;

    vec2() :x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y) {}
    vec2(const vec2& v) : x(v.x), y(v.y) {}

    vec2& operator=(const vec2& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    vec2 operator+(const vec2& v) const {
        return vec2(x + v.x, y + v.y);
    }
    vec2 operator-(const vec2& v) const {
        return vec2(x - v.x, y - v.y);
    }

    vec2 operator*(const vec2& v) const {
        return vec2(x * v.x, y * v.y);
    }

    vec2& operator+=(const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    vec2& operator-=(const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    vec2& operator*=(const float v) {
        x = x * v,
        y = y * v;
        return *this;
    }

    vec2 operator+(const T& s) const {
        return vec2(x + s, y + s);
    }
    vec2 operator-(const T& s) const {
        return vec2(x - s, y - s);
    }
    vec2 operator*(const T& s) const {
        return vec2(x * s, y * s);
    }
    vec2 operator/(const T& s)  const {
        return vec2(x / s, y / s);
    }


    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    void rotate(double deg) {
        double theta = deg / 180.0 * M_PI;
        double c = cos(theta);
        double s = sin(theta);
        double tx = x * c - y * s;
        double ty = x * s + y * c;
        x = tx;
        y = ty;
    }

    vec2& normalize() {
        if (length() == 0) return *this;
        *this *= (1.0 / length());
        return *this;
    }

    float dist(vec2 v) const {
        vec2 d(v.x - x, v.y - y);
        return d.length();
    }
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    void truncate(double length) {
        double angle = atan2f(y, x);
        x = length * cos(angle);
        y = length * sin(angle);
    }

    vec2 ortho() const {
        return vec2(y, -x);
    }

    static float dot(vec2 v1, vec2 v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }
    static float cross(vec2 v1, vec2 v2) {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }

    const float cross(vec2 v2) const
    {
        return (x * v2.y) - (y * v2.x);
    }

    const float dot(vec2 v2) const
    {
        return dot(*this, v2);
    }

    const vec2 perpCCW() const
    {
        return vec2(-y, x);
    }

    const vec2 perpCW() const
    {
        return vec2(y, -x);
    }

};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;

#define PI 3.14159265359
template <class T>
const T min2(const T val1, const T val2)
{
    return val1 < val2 ? val1 : val2;
}

template <class T>
const T max2(const T val1, const T val2)
{
    return val1 > val2 ? val1 : val2;
}

template <class T>
float sign(const T val) {
    return val > 0 ? 1 : val < 0 ? -1 : 0;
}

template <class T>
const T clamp(const T x, const T min, const T max)
{

    if (x < min)
        return min;
    else if (x > max)
        return max;

    return x;
}

#endif
