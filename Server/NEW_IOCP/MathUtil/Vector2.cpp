#include "../stdafx.h"
#include "Vector2.h"
#include "Mathf.h"

using namespace DirectX;

const Vector2 Vector2::down{0, -1};
const Vector2 Vector2::left{-1, 0};
const Vector2 Vector2::one{1, 1};
const Vector2 Vector2::right{1, 0};
const Vector2 Vector2::up{0, 1};
const Vector2 Vector2::zero{0, 0};

const Vector2 Vector2::win_left{Vector2::left};
const Vector2 Vector2::win_right{Vector2::right};
const Vector2 Vector2::win_up{Vector2::down};
const Vector2 Vector2::win_down{Vector2::up};

Vector2::Vector2() :
    Vector2(0, 0)
{
}

Vector2::Vector2(const DirectX::XMFLOAT2 &vec) :
    XMFLOAT2(vec)
{
}

Vector2::Vector2(float x, float y) :
    XMFLOAT2(x, y)
{
}

Vector2 Vector2::operator+(const DirectX::XMFLOAT2 &other) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) + XMLoadFloat2(&other));
    return result;
}

Vector2 Vector2::operator-(const DirectX::XMFLOAT2 &other) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) - XMLoadFloat2(&other));
    return result;
}

Vector2 Vector2::operator*(const DirectX::XMFLOAT2 &other) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) * XMLoadFloat2(&other));
    return result;
}

Vector2 Vector2::operator*(float f) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) * f);
    return result;
}

Vector2 Vector2::operator/(const DirectX::XMFLOAT2 &other) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) / XMLoadFloat2(&other));
    return result;
}

Vector2 Vector2::operator/(float f) const
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMLoadFloat2(this) / f);
    return result;
}

Vector2 &Vector2::operator+=(const DirectX::XMFLOAT2 &other)
{
    *this = *this + other;
    return *this;
}

Vector2 &Vector2::operator-=(const DirectX::XMFLOAT2 &other)
{
    *this = *this - other;
    return *this;
}

Vector2 &Vector2::operator*=(const DirectX::XMFLOAT2 &other)
{
    *this = *this * other;
    return *this;
}

Vector2 &Vector2::operator*=(float f)
{
    *this = *this * f;
    return *this;
}

Vector2 &Vector2::operator/=(const DirectX::XMFLOAT2 &other)
{
    *this = *this / other;
    return *this;
}

Vector2 &Vector2::operator/=(float f)
{
    *this = *this / f;
    return *this;
}

bool Vector2::operator==(const DirectX::XMFLOAT2 &other) const
{
    return Mathf::Approximately(x, other.x)
        && Mathf::Approximately(y, other.y);
}

bool Vector2::operator!=(const DirectX::XMFLOAT2 &other) const
{
    return !operator==(other);
}

float Vector2::length() const
{
    XMFLOAT2 mag;
    XMStoreFloat2(&mag, XMVector2Length(XMLoadFloat2(this)));
    return mag.x;
}

float Vector2::sqr_magnitude() const
{
    return powf(length(), 2.0f);
}

Vector2 Vector2::normalized() const
{
    XMFLOAT2 normal;
    XMStoreFloat2(&normal, XMVector2Normalize(XMLoadFloat2(this)));
    return normal;
}

Vector2 Vector2::Max(const Vector2 &a, const Vector2 &b)
{
    return
    {
        (a.x < b.x) ? b.x : a.x,
        (a.y < b.y) ? b.y : a.y
    };
}

Vector2 Vector2::Min(const Vector2 &a, const Vector2 &b)
{
    return
    {
        (a.x < b.x) ? a.x : b.x,
        (a.y < b.y) ? a.y : b.y
    };
}

Vector2 Vector2::Scale(const Vector2 &a, const Vector2 &b)
{
    return a * b;
}

Vector2 Vector2::MoveTowards(const Vector2 &current, const Vector2 &target, float max_distance_delta)
{
    if ((target - current).length() < max_distance_delta)
        return target;

    return current + (target - current).normalized() * max_distance_delta;
}

float Vector2::Angle(const Vector2 &a, const Vector2 &b)
{
    return Mathf::Rad2Deg(Dot(a.normalized(), b.normalized()));
}

float Vector2::Distance(const Vector2 &a, const Vector2 &b)
{
    return (a - b).length();
}

Vector2 Vector2::ClampMagnitude(const Vector2 &vector, float max_length)
{
    if (vector.length() <= max_length)
        return vector;

    return vector.normalized() * max_length;
}

float Vector2::Dot(const Vector2 &a, const Vector2 &b)
{
    XMFLOAT2 result;
    XMStoreFloat2(&result, XMVector2Dot(XMLoadFloat2(&a), XMLoadFloat2(&b)));
    return result.x;
}

Vector2 Vector2::Lerp(const Vector2 &current, const Vector2 &target, float t)
{
    return LerpUnclamped(current, target, Mathf::Clamp01(t));
}

Vector2 Vector2::LerpUnclamped(const Vector2 &current, const Vector2 &target, float t)
{
    return current + (target - current) * t;
}
