#pragma once
#include <DirectXMath.h>

class Quaternion;
class Matrix4x4;
class Vector3;

class Vector4 : public DirectX::XMFLOAT4
{
public:
    Vector4();
    Vector4(const XMFLOAT4 &vec);
    Vector4(float x, float y, float z, float w);

    Vector4 operator+(const XMFLOAT4 &other) const;
    Vector4 operator-(const XMFLOAT4 &other) const;
    Vector4 operator*(const XMFLOAT4 &other) const;
    Vector4 operator*(float f) const;
    Vector4 operator*(const Quaternion &q) const; // w = 1
    Vector4 operator*(const Matrix4x4 &matrix) const;
    Vector4 operator/(const XMFLOAT4 &other) const;
    Vector4 operator/(float f) const;

    Vector4 &operator+=(const XMFLOAT4 &other);
    Vector4 &operator-=(const XMFLOAT4 &other);
    Vector4 &operator*=(const XMFLOAT4 &other);
    Vector4 &operator*=(float f);
    Vector4 &operator*=(const Quaternion &q);
    Vector4 &operator*=(const Matrix4x4 &matrix);
    Vector4 &operator/=(const XMFLOAT4 &other);
    Vector4 &operator/=(float f);

    operator Vector3() const;

    bool operator==(const XMFLOAT4 &other) const;
    bool operator!=(const XMFLOAT4 &other) const;

    float length() const;
    float sqr_magnitude() const;
    Vector4 normalized() const;

public:
    static const Vector4 one;
    static const Vector4 zero;

    static Vector4 Max(const Vector4 &a, const Vector4 &b);
    static Vector4 Min(const Vector4 &a, const Vector4 &b);
    static Vector4 Scale(const Vector4 &a, const Vector4 &b);

    static Vector4 MoveTowards(const Vector4 &current, const Vector4 &target, float max_distance_delta);

    static float Distance(const Vector4 &vector1, const Vector4 &vector2);
    static float Dot(const Vector4 &vector1, const Vector4 &vector2);

    static Vector4 Lerp(const Vector4 &current, const Vector4 &target, float t);
    static Vector4 LerpUnclamped(const Vector4 &current, const Vector4 &target, float t);

    static Vector4 Project(const Vector4 &vector, const Vector4 &on_normal);

};
