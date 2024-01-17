#include "stdafx.h"
#include "Vector3.h"
#include "Mathf.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

using namespace DirectX;

const Vector3 Vector3::back{ 0,0,-1 };
const Vector3 Vector3::down{ 0, -1, 0 };
const Vector3 Vector3::forward{ 0, 0, 1 };
const Vector3 Vector3::left{ -1, 0, 0 };
const Vector3 Vector3::one{ 1, 1, 1 };
const Vector3 Vector3::right{ 1, 0, 0 };
const Vector3 Vector3::up{ 0,1,0 };
const Vector3 Vector3::zero{ 0,0,0 };

Vector3::Vector3() : Vector3(0.f, 0.f, 0.f)
{

}

Vector3::Vector3(const XMFLOAT3& vec) : XMFLOAT3(vec)
{
}

Vector3::Vector3(float x, float y, float z) : XMFLOAT3({ x,y,z })
{	
}

Vector3 Vector3::operator+(const XMFLOAT3& other) const
{	
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) + XMLoadFloat3(&other));
	return result;
}

Vector3 Vector3::operator-(const XMFLOAT3& other) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) - XMLoadFloat3(&other));
	return result;
}

Vector3 Vector3::operator*(const XMFLOAT3& other) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) * XMLoadFloat3(&other));
	return result;
}

Vector3 Vector3::operator*(float f) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) * f);
	return result;
}

Vector3 Vector3::operator*(const Quaternion& q) const
{
	return *this * Matrix4x4::Rotate(q);
}

Vector3 Vector3::operator*(const Matrix4x4& matrix) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3TransformCoord(XMLoadFloat3(this), XMLoadFloat4x4(&matrix)));
	return result;
}

Vector3 Vector3::operator/(const DirectX::XMFLOAT3& other) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) / XMLoadFloat3(&other));
	return result;
}

Vector3 Vector3::operator/(float f) const
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMLoadFloat3(this) / f);
	return result;
}

Vector3& Vector3::operator+=(const XMFLOAT3& other)
{
	*this = *this + other;
	return *this;
}

Vector3& Vector3::operator-=(const XMFLOAT3& other)
{
	*this = *this - other;
	return *this;
}

Vector3& Vector3::operator*=(const XMFLOAT3& other)
{
	*this = *this * other;
	return *this;
}

Vector3& Vector3::operator*=(float f)
{
	*this = *this * f;
	return *this;
}

Vector3& Vector3::operator*=(const Quaternion& q)
{
	*this = *this * q;
	return *this;
}

Vector3& Vector3::operator*=(const Matrix4x4& matrix)
{
	*this = *this * matrix;
	return *this;
}

Vector3& Vector3::operator/=(const DirectX::XMFLOAT3& other)
{
	*this = *this / other;
	return *this;
}

Vector3& Vector3::operator/=(float f)
{
	*this = *this / f;
	return *this;
}

bool Vector3::operator==(const DirectX::XMFLOAT3& other) const
{	
	return Mathf::Approximately(x, other.x)
		&& Mathf::Approximately(y, other.y)
		&& Mathf::Approximately(z, other.z);
}

bool Vector3::operator!=(const DirectX::XMFLOAT3& other) const
{
	return !operator==(other);
}

float Vector3::length() const
{
	XMFLOAT3 mag;
	XMStoreFloat3(&mag, XMVector3Length(XMLoadFloat3(this)));
	return mag.x;
}

float Vector3::sqr_magnitude() const
{
	return powf(length(), 2.0f);
}

Vector3 Vector3::normalized() const
{
	XMFLOAT3 normal;
	XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(this)));
	return normal;
}

Vector4 Vector3::ToVec4Coord() const
{
	return { x, y, z, 1 };
}

Vector4 Vector3::ToVec4Vec() const
{
	return { x, y, z, 0 };
}

Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
{	
	return 
	{
		(a.x < b.x) ? b.x : a.x,
		(a.y < b.y) ? b.y : a.y,
		(a.z < b.z) ? b.z : a.z
	};
}

Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
{
	return
	{
		(a.x < b.x) ? a.x : b.x,
		(a.y < b.y) ? a.y : b.y,
		(a.z < b.z) ? a.z : b.z
	};
}

Vector3 Vector3::Scale(const Vector3& a, const Vector3& b)
{
	return a * b;
}

void Vector3::OrthoNormalize(Vector3& normal, Vector3& tangent)
{
	normal = normal.normalized();
	tangent = tangent.normalized();		// view vector

	Vector3 up = Cross(normal, tangent).normalized();
	tangent = Cross(up, normal).normalized();
}


Vector3 Vector3::MoveTowards(const Vector3& current, const Vector3& target, float max_distance_delta)
{
	if ((target - current).length() < max_distance_delta)
		return target;

	return current + (target - current).normalized() * max_distance_delta;
}

Vector3 Vector3::RotateTowards(const Vector3& current, const Vector3& target, float max_radians_delta, float max_magnitude_delta)
{
	float omega = acosf(Dot(current.normalized(), target.normalized()));
	Vector3 result = (omega < max_radians_delta)
		? target
		: Slerp(current, target, max_radians_delta / omega);
				
	if (Distance(current, result) < max_magnitude_delta)
		return result;
	else
		return result.normalized() * (current.length() + max_magnitude_delta);
}

float Vector3::Angle(const Vector3& a, const Vector3& b)
{
	return Mathf::Rad2Deg(AngleRad(a, b));
}

float Vector3::AngleRad(const Vector3& a, const Vector3& b)
{
	return Dot(a.normalized(), b.normalized());
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
	return (a - b).length();
}

Vector3 Vector3::ClampMagnitude(const Vector3& vector, float max_length)
{
	if (vector.length() <= max_length)
		return vector;

	return vector.normalized() * max_length;
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b)
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(&a), XMLoadFloat3(&b)));
	return result;
}

Vector3 Vector3::CrossNormal(const Vector3& a, const Vector3& b)
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&a), XMLoadFloat3(&b))));
	return result;
}

float Vector3::Dot(const Vector3& a, const Vector3& b)
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Dot(XMLoadFloat3(&a), XMLoadFloat3(&b)));	
	return result.x;
}

Vector3 Vector3::Project(const Vector3& vector, const Vector3& on_normal)
{
	Vector3 real_on_normal = on_normal.normalized();
	return real_on_normal * Dot(vector, real_on_normal);
}

Vector3 Vector3::Lerp(const Vector3& current, const Vector3& target, float t)
{
	return LerpUnclamped(current, target, Mathf::Clamp01(t));
}

Vector3 Vector3::LerpUnclamped(const Vector3& current, const Vector3& target, float t)
{	
	return current + (target - current) * t;
}

Vector3 Vector3::Slerp(const Vector3& current, const Vector3& target, float t)
{
	return SlerpUnclamped(current, target, Mathf::Clamp01(t));
}

Vector3 Vector3::SlerpUnclamped(const Vector3& current, const Vector3& target, float t)
{
	if (current == target)
		return target;

	float omega = acosf(Dot(current.normalized(), target.normalized()));
	return current * sinf((1 - t) * omega) / sinf(omega) + target * sinf(t * omega) / sinf(omega);
}

Vector3 Vector3::TransformNormal(const Vector3& xmf3Vector, const Matrix4x4& xmmtxTransform)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector), XMLoadFloat4x4(&xmmtxTransform)));
	return(xmf3Result);
}
