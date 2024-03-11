#include "../stdafx.h"
#include "Vector4.h"
#include "Mathf.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

using namespace DirectX;

const Vector4 Vector4::one{1, 1, 1, 1};
const Vector4 Vector4::zero{0, 0, 0, 0};

Vector4::Vector4() :
    Vector4(0, 0, 0, 0)
{
}

Vector4::Vector4(const XMFLOAT4 &vec) :
    XMFLOAT4(vec)
{
}

Vector4::Vector4(float x, float y, float z, float w) :
    XMFLOAT4(x, y, z, w)
{
}

Vector4 Vector4::operator+(const XMFLOAT4 &other) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) + XMLoadFloat4(&other));
    return result;
}

Vector4 Vector4::operator-(const XMFLOAT4 &other) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) - XMLoadFloat4(&other));
    return result;
}

Vector4 Vector4::operator*(const XMFLOAT4 &other) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) * XMLoadFloat4(&other));
    return result;
}

Vector4 Vector4::operator*(float f) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) * f);
    return result;
}

Vector4 Vector4::operator*(const Quaternion &q) const
{
    return *this * Matrix4x4::Rotate(q);
}

Vector4 Vector4::operator*(const Matrix4x4 &matrix) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMVector4Transform(XMLoadFloat4(this), XMLoadFloat4x4(&matrix)));
    return result;
}

Vector4 Vector4::operator/(const XMFLOAT4 &other) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) / XMLoadFloat4(&other));
    return result;
}

Vector4 Vector4::operator/(float f) const
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMLoadFloat4(this) / f);
    return result;
}

Vector4 &Vector4::operator+=(const XMFLOAT4 &other)
{
    *this = *this + other;
    return *this;
}

Vector4 &Vector4::operator-=(const XMFLOAT4 &other)
{
    *this = *this - other;
    return *this;
}

Vector4 &Vector4::operator*=(const XMFLOAT4 &other)
{
    *this = *this * other;
    return *this;
}

Vector4 &Vector4::operator*=(float f)
{
    *this = *this * f;
    return *this;
}

Vector4 &Vector4::operator*=(const Quaternion &q)
{
    *this = *this * q;
    return *this;
}

Vector4 &Vector4::operator*=(const Matrix4x4 &matrix)
{
    *this = *this * matrix;
    return *this;
}

Vector4 &Vector4::operator/=(const XMFLOAT4 &other)
{
    *this = *this / other;
    return *this;
}

Vector4 &Vector4::operator/=(float f)
{
    *this = *this / f;
    return *this;
}

Vector4::operator Vector3() const
{
    return {x, y, z};
}

bool Vector4::operator==(const XMFLOAT4 &other) const
{
    return Mathf::Approximately(x, other.x)
        && Mathf::Approximately(y, other.y)
        && Mathf::Approximately(z, other.z)
        && Mathf::Approximately(w, other.w);
}

bool Vector4::operator!=(const XMFLOAT4 &other) const
{
    return !operator==(other);
}

float Vector4::length() const
{
    XMFLOAT4 mag;
    XMStoreFloat4(&mag, XMVector4Length(XMLoadFloat4(this)));
    return mag.x;
}

float Vector4::sqr_magnitude() const
{
    return powf(length(), 2.0f);
}

Vector4 Vector4::normalized() const
{
    XMFLOAT4 normal;
    XMStoreFloat4(&normal, XMVector4Normalize(XMLoadFloat4(this)));
    return normal;
}


Vector4 Vector4::Max(const Vector4 &a, const Vector4 &b)
{
    return
    {
        (a.x < b.x) ? b.x : a.x,
        (a.y < b.y) ? b.y : a.y,
        (a.z < b.z) ? b.z : a.z,
        (a.w < b.w) ? b.w : a.w
    };
}

Vector4 Vector4::Min(const Vector4 &a, const Vector4 &b)
{
    return
    {
        (a.x < b.x) ? a.x : b.x,
        (a.y < b.y) ? a.y : b.y,
        (a.z < b.z) ? a.z : b.z,
        (a.w < b.w) ? a.w : b.w
    };
}

Vector4 Vector4::Scale(const Vector4 &a, const Vector4 &b)
{
    return a * b;
}

Vector4 Vector4::MoveTowards(const Vector4 &current, const Vector4 &target, float max_distance_delta)
{
    if ((target - current).length() < max_distance_delta)
        return target;

    return current + (target - current).normalized() * max_distance_delta;

}

float Vector4::Distance(const Vector4 &a, const Vector4 &b)
{
    return (a - b).length();
}

float Vector4::Dot(const Vector4 &a, const Vector4 &b)
{
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMVector4Dot(XMLoadFloat4(&a), XMLoadFloat4(&b)));
    return result.x;
}

Vector4 Vector4::Lerp(const Vector4 &current, const Vector4 &target, float t)
{
    return LerpUnclamped(current, target, Mathf::Clamp01(t));
}

Vector4 Vector4::LerpUnclamped(const Vector4 &current, const Vector4 &target, float t)
{
    return current + (target - current) * t;
}

Vector4 Vector4::Project(const Vector4 &vector, const Vector4 &on_normal)
{
    Vector4 real_on_normal = on_normal.normalized();
    return real_on_normal * Dot(vector, real_on_normal);
}
