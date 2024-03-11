#pragma once
#include <DirectXMath.h>

class Vector2 : public DirectX::XMFLOAT2
{
public:
    Vector2();
    Vector2(const XMFLOAT2 &vec);
    Vector2(float x, float y);

    Vector2 operator+(const XMFLOAT2 &other) const;
    Vector2 operator-(const XMFLOAT2 &other) const;
    Vector2 operator*(const XMFLOAT2 &other) const;
    Vector2 operator*(float f) const;
    Vector2 operator/(const XMFLOAT2 &other) const;
    Vector2 operator/(float f) const;

    Vector2 &operator+=(const XMFLOAT2 &other);
    Vector2 &operator-=(const XMFLOAT2 &other);
    Vector2 &operator*=(const XMFLOAT2 &other);
    Vector2 &operator*=(float f);
    Vector2 &operator/=(const XMFLOAT2 &other);
    Vector2 &operator/=(float f);

    bool operator==(const XMFLOAT2 &other) const;
    bool operator!=(const XMFLOAT2 &other) const;

    float length() const;
    float sqr_magnitude() const;
    Vector2 normalized() const;

public:
    static const Vector2 down;
    static const Vector2 left;
    static const Vector2 one;
    static const Vector2 right;
    static const Vector2 up;
    static const Vector2 zero;

    static const Vector2 win_left;
    static const Vector2 win_right;
    static const Vector2 win_up;
    static const Vector2 win_down;

    static Vector2 Max(const Vector2 &a, const Vector2 &b);
    static Vector2 Min(const Vector2 &a, const Vector2 &b);
    static Vector2 Scale(const Vector2 &a, const Vector2 &b);

    static Vector2 MoveTowards(const Vector2 &current, const Vector2 &target, float max_distance_delta);

    static float Angle(const Vector2 &a, const Vector2 &b);

    static float Distance(const Vector2 &a, const Vector2 &b);
    static Vector2 ClampMagnitude(const Vector2 &vector, float max_length);

    static float Dot(const Vector2 &a, const Vector2 &b);

    static Vector2 Lerp(const Vector2 &current, const Vector2 &target, float t);
    static Vector2 LerpUnclamped(const Vector2 &current, const Vector2 &target, float t);
};
