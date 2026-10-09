// =============================================================================
//  MathLib.h  -  our own tiny vector / matrix library (no GLM).
//
//  Everything the project needs for TRANSFORMATIONS lives here:
//    * Vec2 / Vec3 / Vec4 with the usual + - * / dot cross normalize
//    * Mat4 (4x4 matrix, column-major like OpenGL expects)
//    * translate / scale / rotateX / rotateY / rotateZ / rotateAxis
//    * perspective / ortho / lookAt  (camera + projection matrices)
//
//  Column-major means element (row r, column c) is stored at m[c*4 + r].
//  That is exactly the layout glUniformMatrix4fv wants (transpose = GL_FALSE).
// =============================================================================
#pragma once
#include <cmath>

const float PI = 3.14159265358979f;

inline float radians(float deg) { return deg * PI / 180.0f; }
inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float mixf(float a, float b, float t) { return a + (b - a) * t; }
inline float smoothstepf(float e0, float e1, float x) {
    float t = clampf((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// ----------------------------------------------------------------------------
struct Vec2 {
    float x = 0, y = 0;
    Vec2() {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& b) const { return Vec2(x + b.x, y + b.y); }
    Vec2 operator-(const Vec2& b) const { return Vec2(x - b.x, y - b.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
};

// ----------------------------------------------------------------------------
struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() {}
    Vec3(float v) : x(v), y(v), z(v) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& b) const { return Vec3(x + b.x, y + b.y, z + b.z); }
    Vec3 operator-(const Vec3& b) const { return Vec3(x - b.x, y - b.y, z - b.z); }
    Vec3 operator*(const Vec3& b) const { return Vec3(x * b.x, y * b.y, z * b.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3& operator+=(const Vec3& b) { x += b.x; y += b.y; z += b.z; return *this; }
    Vec3& operator-=(const Vec3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
};
inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }
inline Vec3 normalize(const Vec3& v) {
    float l = length(v);
    return l > 1e-8f ? v / l : Vec3(0, 1, 0);
}
inline Vec3 mix(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }

// ----------------------------------------------------------------------------
struct Vec4 {
    float x = 0, y = 0, z = 0, w = 0;
    Vec4() {}
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4(const Vec3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
};

// ----------------------------------------------------------------------------
struct Mat4 {
    float m[16];

    Mat4() { for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f; }  // identity

    float& at(int row, int col) { return m[col * 4 + row]; }
    float  at(int row, int col) const { return m[col * 4 + row]; }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int row = 0; row < 4; row++)
            for (int col = 0; col < 4; col++) {
                float s = 0;
                for (int k = 0; k < 4; k++) s += at(row, k) * b.at(k, col);
                r.at(row, col) = s;
            }
        return r;
    }
    Vec4 operator*(const Vec4& v) const {
        return Vec4(at(0,0)*v.x + at(0,1)*v.y + at(0,2)*v.z + at(0,3)*v.w,
                    at(1,0)*v.x + at(1,1)*v.y + at(1,2)*v.z + at(1,3)*v.w,
                    at(2,0)*v.x + at(2,1)*v.y + at(2,2)*v.z + at(2,3)*v.w,
                    at(3,0)*v.x + at(3,1)*v.y + at(3,2)*v.z + at(3,3)*v.w);
    }
    Vec3 transformPoint(const Vec3& p) const { Vec4 r = (*this) * Vec4(p, 1.0f); return Vec3(r.x, r.y, r.z); }
    Vec3 transformDir(const Vec3& d)   const { Vec4 r = (*this) * Vec4(d, 0.0f); return Vec3(r.x, r.y, r.z); }
};

// ---- basic transformation matrices -----------------------------------------
inline Mat4 translate(const Vec3& t) {
    Mat4 r; r.at(0,3) = t.x; r.at(1,3) = t.y; r.at(2,3) = t.z; return r;
}
inline Mat4 scale(const Vec3& s) {
    Mat4 r; r.at(0,0) = s.x; r.at(1,1) = s.y; r.at(2,2) = s.z; return r;
}
inline Mat4 rotateX(float a) {
    Mat4 r; float c = std::cos(a), s = std::sin(a);
    r.at(1,1) = c; r.at(1,2) = -s; r.at(2,1) = s; r.at(2,2) = c; return r;
}
inline Mat4 rotateY(float a) {
    Mat4 r; float c = std::cos(a), s = std::sin(a);
    r.at(0,0) = c; r.at(0,2) = s; r.at(2,0) = -s; r.at(2,2) = c; return r;
}
inline Mat4 rotateZ(float a) {
    Mat4 r; float c = std::cos(a), s = std::sin(a);
    r.at(0,0) = c; r.at(0,1) = -s; r.at(1,0) = s; r.at(1,1) = c; return r;
}
// Rotation of angle a (radians) around any unit axis (Rodrigues formula).
inline Mat4 rotateAxis(Vec3 axis, float a) {
    axis = normalize(axis);
    float c = std::cos(a), s = std::sin(a), t = 1.0f - c;
    float x = axis.x, y = axis.y, z = axis.z;
    Mat4 r;
    r.at(0,0) = t*x*x + c;   r.at(0,1) = t*x*y - s*z; r.at(0,2) = t*x*z + s*y;
    r.at(1,0) = t*x*y + s*z; r.at(1,1) = t*y*y + c;   r.at(1,2) = t*y*z - s*x;
    r.at(2,0) = t*x*z - s*y; r.at(2,1) = t*y*z + s*x; r.at(2,2) = t*z*z + c;
    return r;
}
// A matrix whose columns are the given axes: turns local (x,y,z) into world.
inline Mat4 basis(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis, const Vec3& origin) {
    Mat4 r;
    r.at(0,0) = xAxis.x; r.at(1,0) = xAxis.y; r.at(2,0) = xAxis.z;
    r.at(0,1) = yAxis.x; r.at(1,1) = yAxis.y; r.at(2,1) = yAxis.z;
    r.at(0,2) = zAxis.x; r.at(1,2) = zAxis.y; r.at(2,2) = zAxis.z;
    r.at(0,3) = origin.x; r.at(1,3) = origin.y; r.at(2,3) = origin.z;
    return r;
}
// Local +Y axis is pointed along 'dir' (handy for cylinders / bars between 2 points).
inline Mat4 alignY(const Vec3& origin, const Vec3& dir) {
    Vec3 y = normalize(dir);
    Vec3 helper = std::fabs(y.y) < 0.95f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 x = normalize(cross(helper, y));
    Vec3 z = cross(x, y);
    return basis(x, y, z, origin);
}

// ---- camera / projection matrices --------------------------------------------
inline Mat4 perspective(float fovyRadians, float aspect, float zNear, float zFar) {
    Mat4 r;
    float f = 1.0f / std::tan(fovyRadians * 0.5f);
    for (int i = 0; i < 16; i++) r.m[i] = 0;
    r.at(0,0) = f / aspect;
    r.at(1,1) = f;
    r.at(2,2) = (zFar + zNear) / (zNear - zFar);
    r.at(2,3) = (2.0f * zFar * zNear) / (zNear - zFar);
    r.at(3,2) = -1.0f;
    return r;
}
inline Mat4 ortho(float l, float r_, float b, float t, float n, float f) {
    Mat4 r;
    r.at(0,0) = 2.0f / (r_ - l);
    r.at(1,1) = 2.0f / (t - b);
    r.at(2,2) = -2.0f / (f - n);
    r.at(0,3) = -(r_ + l) / (r_ - l);
    r.at(1,3) = -(t + b) / (t - b);
    r.at(2,3) = -(f + n) / (f - n);
    return r;
}
inline Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 f = normalize(target - eye);
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);
    Mat4 r;
    r.at(0,0) = s.x;  r.at(0,1) = s.y;  r.at(0,2) = s.z;
    r.at(1,0) = u.x;  r.at(1,1) = u.y;  r.at(1,2) = u.z;
    r.at(2,0) = -f.x; r.at(2,1) = -f.y; r.at(2,2) = -f.z;
    r.at(0,3) = -dot(s, eye);
    r.at(1,3) = -dot(u, eye);
    r.at(2,3) = dot(f, eye);
    return r;
}

// Inverse-transpose of the upper 3x3 part: used to transform NORMALS correctly
// when a model matrix contains non-uniform scaling.
inline Mat4 normalMatrix(const Mat4& a) {
    float a00 = a.at(0,0), a01 = a.at(0,1), a02 = a.at(0,2);
    float a10 = a.at(1,0), a11 = a.at(1,1), a12 = a.at(1,2);
    float a20 = a.at(2,0), a21 = a.at(2,1), a22 = a.at(2,2);
    float det = a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);
    float id = std::fabs(det) > 1e-12f ? 1.0f / det : 0.0f;
    Mat4 r;   // r = transpose(inverse(a)) for the 3x3 block
    r.at(0,0) =  (a11*a22 - a12*a21) * id; r.at(0,1) = -(a10*a22 - a12*a20) * id; r.at(0,2) =  (a10*a21 - a11*a20) * id;
    r.at(1,0) = -(a01*a22 - a02*a21) * id; r.at(1,1) =  (a00*a22 - a02*a20) * id; r.at(1,2) = -(a00*a21 - a01*a20) * id;
    r.at(2,0) =  (a01*a12 - a02*a11) * id; r.at(2,1) = -(a00*a12 - a02*a10) * id; r.at(2,2) =  (a00*a11 - a01*a10) * id;
    return r;
}

// Small deterministic random numbers (same result every run -> repeatable scenes).
inline float hash1(float n) { float s = std::sin(n) * 43758.5453f; return s - std::floor(s); }
struct Rng {
    unsigned int state;
    explicit Rng(unsigned int seed = 1234u) : state(seed) {}
    float next() { state = state * 1664525u + 1013904223u; return (state >> 8) * (1.0f / 16777216.0f); }
    float range(float a, float b) { return a + (b - a) * next(); }
};
