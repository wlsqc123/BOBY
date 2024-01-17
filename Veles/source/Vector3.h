#pragma once
#include <DirectXMath.h>

class Quaternion;
class Vector4;
class Matrix4x4;

class Vector3 : public DirectX::XMFLOAT3
{	   
public:
	Vector3();
	Vector3(const DirectX::XMFLOAT3& vec);
	Vector3(float x, float y, float z);
	
	Vector3 operator+(const DirectX::XMFLOAT3& other) const;
	Vector3 operator-(const DirectX::XMFLOAT3& other) const;
	Vector3 operator*(const DirectX::XMFLOAT3& other) const;
	Vector3 operator*(float f) const;
	Vector3 operator*(const Quaternion& q) const;		// w = 0
	Vector3 operator*(const Matrix4x4& matrix) const;		// w = 0	// vec * matrix   (it's not point * matrix)
	Vector3 operator/(const DirectX::XMFLOAT3& other) const;
	Vector3 operator/(float f) const;

	Vector3& operator+=(const DirectX::XMFLOAT3& other);
	Vector3& operator-=(const DirectX::XMFLOAT3& other);
	Vector3& operator*=(const DirectX::XMFLOAT3& other);
	Vector3& operator*=(float f);
	Vector3& operator*=(const Quaternion& q);	
	Vector3& operator*=(const Matrix4x4& matrix);
	Vector3& operator/=(const DirectX::XMFLOAT3& other);
	Vector3& operator/=(float f);

	bool operator==(const DirectX::XMFLOAT3& other) const;
	bool operator!=(const DirectX::XMFLOAT3& other) const;
		
	float length() const;
	float sqr_magnitude() const;
	Vector3 normalized() const;	
	
	Vector4 ToVec4Coord() const;
	Vector4 ToVec4Vec() const;
	
public:
	static const Vector3 back;
	static const Vector3 down;
	static const Vector3 forward;
	static const Vector3 left;
	static const Vector3 one;
	static const Vector3 right;
	static const Vector3 up;
	static const Vector3 zero;

	static Vector3 Max(const Vector3& a, const Vector3& b);
	static Vector3 Min(const Vector3& a, const Vector3& b);
	static Vector3 Scale(const Vector3& a, const Vector3& b);
	static void OrthoNormalize(Vector3& normal, Vector3& tangent);

	static Vector3 MoveTowards(const Vector3& current, const Vector3& target, float max_distance_delta);
	static Vector3 RotateTowards(const Vector3& current, const Vector3& target, float max_radians_delta, float max_magnitude_delta);
		
	
	static float Angle(const Vector3& a, const Vector3& b);
	static float AngleRad(const Vector3& a, const Vector3& b);

	static float Distance(const Vector3& a, const Vector3& b);
	static Vector3 ClampMagnitude(const Vector3& vector, float max_length);

	static Vector3 Cross(const Vector3& a, const Vector3& b);
	static Vector3 CrossNormal(const Vector3& a, const Vector3& b);
	static float Dot(const Vector3& a, const Vector3& b);
	static Vector3 Project(const Vector3& vector, const Vector3& on_normal);
	   

	static Vector3 Lerp(const Vector3& current, const Vector3& target, float t);
	static Vector3 LerpUnclamped(const Vector3& current, const Vector3& target, float t);
	static Vector3 Slerp(const Vector3& current, const Vector3& target, float t);
	static Vector3 SlerpUnclamped(const Vector3& current, const Vector3& target, float t);

	static Vector3 TransformNormal(const Vector3& xmf3Vector, const Matrix4x4& xmmtxTransform);


	//static Vector3 ProjectOnPlane(const Vector3& vector, const Vector3& plane_normal);
	//static Vector3 Reflect(const Vector3& in_direction, const Vector3& in_normal);
	//static Vector3 SmoothDamp
};

