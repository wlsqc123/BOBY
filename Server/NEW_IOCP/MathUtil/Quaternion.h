#pragma once
#include <DirectXMath.h>

class Vector3;
class Matrix4x4;

class Quaternion : private DirectX::XMFLOAT4
{
public:
    Quaternion();
    Quaternion(const XMFLOAT4 &q);
    Quaternion(const Quaternion &q);

    Quaternion operator*(const Quaternion &other) const;
    Matrix4x4 operator*(const Matrix4x4 &other) const;

    Quaternion &operator*=(const Quaternion &other);

    bool operator==(const Quaternion &other) const;
    bool operator!=(const Quaternion &other) const;

    Quaternion inverse() const;
    Vector3 eulerAngles() const;
    Quaternion normalized() const;

    void SetFromToRotation(const Vector3 &from_direction, const Vector3 &to_direction);

public:
    static const Quaternion identity;

    static float Angle(const Quaternion &a, const Quaternion &b);
    static float AngleRad(const Quaternion &a, const Quaternion &b);

    static Quaternion AngleAxis(float angle, const Vector3 &axis);

    static float Dot(const Quaternion &a, const Quaternion &b);

    static Quaternion Euler(float x, float y, float z);

    static Quaternion Slerp(const Quaternion &a, const Quaternion &b, float t);
    static Quaternion SlerpUnclamped(const Quaternion &a, const Quaternion &b, float t);

    static Quaternion FromToRotation(const Vector3 &from_direction, const Vector3 &to_direction);
    static Quaternion RotateTowards(const Quaternion &from, const Quaternion &to, float maxDegreesDelta);

    friend class Matrix4x4;

};
