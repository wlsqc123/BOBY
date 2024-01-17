#include "stdafx.h"
#include "Quaternion.h"
#include "Mathf.h"
#include "Vector3.h"
#include "Matrix4x4.h"

using namespace DirectX;

const Quaternion Quaternion::identity{ { 0, 0, 0, 1} };

Quaternion::Quaternion() : Quaternion(Quaternion::identity)
{
}

Quaternion::Quaternion(const DirectX::XMFLOAT4& q) : XMFLOAT4(q)
{
}

Quaternion::Quaternion(const Quaternion& q) : XMFLOAT4(q)
{
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionMultiply(XMLoadFloat4(this), XMLoadFloat4(&other)));
	return result;
}

Matrix4x4 Quaternion::operator*(const Matrix4x4& other) const
{
	return Matrix4x4::Rotate(*this) * other;
}

Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	*this = *this * other;
	return *this;
}


bool Quaternion::operator==(const Quaternion& other) const
{
	return Mathf::Approximately(x, other.x)
		&& Mathf::Approximately(y, other.y)
		&& Mathf::Approximately(z, other.z)
		&& Mathf::Approximately(w, other.w);
}

bool Quaternion::operator!=(const Quaternion& other) const
{
	return !operator==(other);
}

Quaternion Quaternion::inverse() const
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionInverse(XMLoadFloat4(this)));
	return result;
}

Vector3 Quaternion::eulerAngles() const
{
	//XMFLOAT3 angles;

	//Quaternion n = *this;
	//n.normalized();

	//// roll (x-axis rotation)

	//// div/0 problem
	////float sinr_cosp = +2.0f * (n.w * n.x + n.y * n.z);
	////float cosr_cosp = +1.0f - 2.0f * (n.x * n.x + n.y * n.y);
	////angles.z = atan2f(sinr_cosp, cosr_cosp);

	//angles.z = 1.0f - 2.0f * (powf(y, 2.0f) + powf(z, 2.0f));
	//	
	//// pitch (y-axis rotation)
	//float sinp = +2.0f * (n.w * n.y - n.z * n.x);
	//angles.x = (fabs(sinp) >= 1)
	//	? copysignf(Mathf::pi / 2.0f, sinp)
	//	: asinf(sinp);

	//// yaw (z-axis rotation)
	//
	////// div/0 problem
	////float siny_cosp = +2.0f * (n.w * n.z + n.x * n.y);
	////float cosy_cosp = +1.0f - 2.0f * (n.y * n.y + n.z * n.z);
	////angles.y = atan2f(siny_cosp, cosy_cosp);

	//angles.y = 1.0f - 2.0f * (powf(z, 2.0f) + powf(w, 2.0f));

	//angles.x = Mathf::Rad2Deg(angles.x);
	//angles.y = Mathf::Rad2Deg(angles.y);
	//angles.z = Mathf::Rad2Deg(angles.z);

	//return angles;


	// new code
	Quaternion q = this->normalized();

	Vector3 euler;

	// if the input quaternion is normalized, this is exactly one. Otherwise, this acts as a correction factor for the quaternion's not-normalizedness
	float unit = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);

	// this will have a magnitude of 0.5 or greater if and only if this is a singularity case
	float test = q.x * q.w - q.y * q.z;

	if (test > 0.4995f * unit) // singularity at north pole
	{
		euler.x = Mathf::pi / 2;
		euler.y = 2.f * atan2f(q.y, q.x);
		euler.z = 0;
	}
	else if (test < -0.4995f * unit) // singularity at south pole
	{
		euler.x = -Mathf::pi / 2;
		euler.y = -2.f * atan2f(q.y, q.x);
		euler.z = 0;
	}
	else // no singularity - this is the majority of cases
	{
		euler.x = asinf(2.f * (q.w * q.x - q.y * q.z));
		euler.y = atan2f(2.f * q.w * q.y + 2.f * q.z * q.x, 1 - 2.f * (q.x * q.x + q.y * q.y));
		euler.z = atan2f(2.f * q.w * q.z + 2.f * q.x * q.y, 1 - 2.f * (q.z * q.z + q.x * q.x));
	}

	// all the math so far has been done in radians. Before returning, we convert to degrees...
	euler.x = Mathf::Rad2Deg(euler.x);
	euler.y = Mathf::Rad2Deg(euler.y);
	euler.z = Mathf::Rad2Deg(euler.z);

	//...and then ensure the degree values are between 0 and 360

	
	euler.x = fmod(euler.x, 360.f);
	euler.y = fmod(euler.y, 360.f);
	euler.z = fmod(euler.z, 360.f);

	return euler;
}

Quaternion Quaternion::normalized() const
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionNormalize(XMLoadFloat4(this)));
	return result;
}

void Quaternion::SetFromToRotation(const Vector3& from_direction, const Vector3& to_direction)
{
	*this = FromToRotation(from_direction, to_direction);
}

float Quaternion::Angle(const Quaternion& a, const Quaternion& b)
{
	return Mathf::Rad2Deg(AngleRad(a, b));
}

float Quaternion::AngleRad(const Quaternion& a, const Quaternion& b)
{
	return Dot(a.normalized(), b.normalized());
}

Quaternion Quaternion::AngleAxis(float angle, const Vector3& axis)
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionRotationAxis(XMLoadFloat3(&axis), Mathf::Deg2Rad(angle)));
	return result;
}

float Quaternion::Dot(const Quaternion& a, const Quaternion& b)
{
	return XMQuaternionDot(XMLoadFloat4(&a), XMLoadFloat4(&b)).m128_f32[0];
}

Quaternion Quaternion::Euler(float x, float y, float z)
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionRotationRollPitchYaw(Mathf::Deg2Rad(x), Mathf::Deg2Rad(y), Mathf::Deg2Rad(z)));
	return result;
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
{
	return SlerpUnclamped(a, b, Mathf::Clamp01(t));
}

Quaternion Quaternion::SlerpUnclamped(const Quaternion& a, const Quaternion& b, float t)
{
	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionSlerp(XMLoadFloat4(&a), XMLoadFloat4(&b), t));
	return result;
}

Quaternion Quaternion::FromToRotation(const Vector3& from_direction, const Vector3& to_direction)
{
	Vector3 axis = Vector3::Cross(from_direction, to_direction).normalized();
	float angle = Vector3::AngleRad(from_direction, to_direction);

	XMFLOAT4 result;
	XMStoreFloat4(&result, XMQuaternionRotationAxis(XMLoadFloat3(&axis), angle));
	return result;
}

Quaternion Quaternion::RotateTowards(const Quaternion& from, const Quaternion& to, float maxDegreesDelta)
{
	return Quaternion();
}

