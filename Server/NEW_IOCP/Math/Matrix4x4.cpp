#include "../stdafx.h"
#include <cassert>
#include "Matrix4x4.h"
#include "Mathf.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

using namespace DirectX;

const Matrix4x4 Matrix4x4::identity{ 
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};
const Matrix4x4 Matrix4x4::zero{
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};

Matrix4x4::Matrix4x4() : XMFLOAT4X4(Matrix4x4::identity)
{
}

Matrix4x4::Matrix4x4(float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44)
	: XMFLOAT4X4(_11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44)
{
}

Matrix4x4::Matrix4x4(const DirectX::XMFLOAT4X4& matrix) : XMFLOAT4X4(matrix)
{
}

Matrix4x4::Matrix4x4(const Vector3& pos, const Quaternion& q, const Vector3& s)
{
	SetTRS(pos, q, s);
}

Matrix4x4 Matrix4x4::operator*(const Quaternion& other) const
{
	return *this * Matrix4x4::Rotate(other);
}

Matrix4x4 Matrix4x4::operator*(const DirectX::XMFLOAT4X4& other) const
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMLoadFloat4x4(this) * XMLoadFloat4x4(&other));
	return result;
}

Matrix4x4& Matrix4x4::operator*=(const Quaternion& other)
{
	*this = *this * other;
	return *this;
}

Matrix4x4& Matrix4x4::operator*=(const DirectX::XMFLOAT4X4& other)
{
	*this = *this * other;
	return *this;
}

bool Matrix4x4::operator==(const DirectX::XMFLOAT4X4& other) const
{
	return Mathf::Approximately(_11, other._11)
		&& Mathf::Approximately(_12, other._12)
		&& Mathf::Approximately(_13, other._13)
		&& Mathf::Approximately(_14, other._14)
		&& Mathf::Approximately(_21, other._21)
		&& Mathf::Approximately(_22, other._22)
		&& Mathf::Approximately(_23, other._23)
		&& Mathf::Approximately(_24, other._24)
		&& Mathf::Approximately(_31, other._31)
		&& Mathf::Approximately(_32, other._32)
		&& Mathf::Approximately(_33, other._33)
		&& Mathf::Approximately(_34, other._34)
		&& Mathf::Approximately(_41, other._41)
		&& Mathf::Approximately(_42, other._42)
		&& Mathf::Approximately(_43, other._43)
		&& Mathf::Approximately(_44, other._44);
}

bool Matrix4x4::operator!=(const DirectX::XMFLOAT4X4& other) const
{
	return !operator==(other);
}

float Matrix4x4::determinant() const
{
	return XMMatrixDeterminant(XMLoadFloat4x4(this)).m128_f32[0];
}

Matrix4x4 Matrix4x4::inverse() const
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixInverse(NULL, XMLoadFloat4x4(this)));
	return result;
}

bool Matrix4x4::isIdentity() const
{
	return *this == Matrix4x4::identity;
}

Quaternion Matrix4x4::rotation() const
{
	Quaternion result;
	DirectX::XMStoreFloat4(
		&(result),
		DirectX::XMQuaternionRotationMatrix(
			DirectX::XMLoadFloat4x4(this))
	);
	return result.Normalized();
}

Matrix4x4 Matrix4x4::transpose() const
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(this)));
	return result;
}

Vector4 Matrix4x4::GetColumn(int index) const
{
	switch (index)
	{
	case 1:return { _11, _12, _13, _14 };
	case 2:return { _21, _22, _23, _24 };
	case 3:return { _31, _32, _33, _34 };
	case 4:return { _41, _42, _43, _44 };
	default:
		assert(!"Vector4 Matrix4x4::GetColumn(int) const : index is out of range.");
		return {};
	}
}

Vector4 Matrix4x4::GetRow(int index) const
{
	switch (index)
	{
	case 1:return { _11, _21, _31, _41 };
	case 2:return { _12, _22, _32, _42 };
	case 3:return { _13, _23, _33, _43 };
	case 4:return { _14, _24, _34, _44 };
	default:
		assert(!"Vector4 Matrix4x4::GetRow(int) const : index is out of range.");
		return {};
	}
}

void Matrix4x4::GetTRS(Vector3* pos, Quaternion* rot, Vector3* scale) const
{
	XMVECTOR xmv_pos, xmv_rot, xmv_scale;
	XMMatrixDecompose(&xmv_scale, &xmv_rot, &xmv_pos, XMLoadFloat4x4(this));

	if(pos)
		XMStoreFloat3(pos, xmv_pos);
	
	if(rot)
		XMStoreFloat4(rot, xmv_rot);
	
	if(scale)
		XMStoreFloat3(scale, xmv_scale);
}

void Matrix4x4::SetColumn(int index, const Vector4& column)
{	
	switch (index)
	{
	case 1:
		_11 = column.x;
		_12 = column.y;
		_13 = column.z;
		_14 = column.w;
		break;
	case 2:
		_21 = column.x;
		_22 = column.y;
		_23 = column.z;
		_24 = column.w;
		break;
	case 3:
		_31 = column.x;
		_32 = column.y;
		_33 = column.z;
		_34 = column.w;
		break;
	case 4:
		_41 = column.x;
		_42 = column.y;
		_43 = column.z;
		_44 = column.w;
		break;
	default:
		assert(!"void Matrix4x4::SetColumn(int, const Vector4&) const : index is out of range.");
		break;
	}
}

void Matrix4x4::SetRow(int index, const Vector4& row)
{
	switch (index)
	{
	case 1:
		_11 = row.x;
		_21 = row.y;
		_31 = row.z;
		_41 = row.w;
		break;
	case 2:
		_12 = row.x;
		_22 = row.y;
		_32 = row.z;
		_42 = row.w;
		break;
	case 3:
		_13 = row.x;
		_23 = row.y;
		_33 = row.z;
		_43 = row.w;
		break;
	case 4:
		_14 = row.x;
		_24 = row.y;
		_34 = row.z;
		_44 = row.w;
		break;
	default:
		assert(!"void Matrix4x4::SetRow(int, const Vector4&) const : index is out of range.");
		break;
	}
}

void Matrix4x4::SetTRS(const Vector3& pos, const Quaternion& q, const Vector3& s)
{	
	*this = Matrix4x4::TRS(pos, q, s);
}

Vector3 Matrix4x4::MultiplyPoint(const Vector3& point) const
{
	return (point.ToVec4Coord() * (*this));
}

Vector3 Matrix4x4::MultiplyVector(const Vector3& vector) const
{
	return vector * (*this);
}


Matrix4x4 Matrix4x4::Translate(const Vector3& pos)
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranslation(pos.x, pos.y, pos.z));
	return result;
}

Matrix4x4 Matrix4x4::Rotate(const Quaternion& q)
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
	return result;
}

Matrix4x4 Matrix4x4::Scale(const Vector3& s)
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixScaling(s.x, s.y, s.z));
	return result;
}

Matrix4x4 Matrix4x4::LookAt(const Vector3& eye, const Vector3& lookat, const Vector3& dir)
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookat), XMLoadFloat3(&dir)));
	return result;
}

Matrix4x4 Matrix4x4::TRS(const Vector3& pos, const Quaternion& q, const Vector3& s)
{
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result,
		XMMatrixAffineTransformation
		(
			XMLoadFloat4(&s.ToVec4Coord()),
			{ 0,0,0,1 },
			XMLoadFloat4(&q),
			XMLoadFloat4(&pos.ToVec4Coord())
		)
	);
	return result;
}

Matrix4x4 Matrix4x4::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	assert(left <= right);
	assert(top <= bottom);
	assert(zNear <= zFar);

	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixOrthographicLH(right - left, bottom - top, zNear, zFar));
	return result;
}

Matrix4x4 Matrix4x4::Perspective(float fov_y, float aspect, float zNear, float zFar)
{
	assert(zNear <= zFar);
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(fov_y, aspect, zNear, zFar));
	return result;
}


