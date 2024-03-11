#pragma once
#include <DirectXMath.h>

class Quaternion;
class Vector3;
class Vector4;

// 행우선 행렬
class Matrix4x4 : public DirectX::XMFLOAT4X4
{
public:
    Matrix4x4();
    Matrix4x4(
        float _11, float _12, float _13, float _14,
        float _21, float _22, float _23, float _24,
        float _31, float _32, float _33, float _34,
        float _41, float _42, float _43, float _44);
    Matrix4x4(const XMFLOAT4X4 &matrix);
    Matrix4x4(const Vector3 &pos, const Quaternion &q, const Vector3 &s);

    Matrix4x4 operator*(const Quaternion &other) const;
    Matrix4x4 operator*(const XMFLOAT4X4 &other) const;

    Matrix4x4 &operator*=(const Quaternion &other);
    Matrix4x4 &operator*=(const XMFLOAT4X4 &other);

    bool operator==(const XMFLOAT4X4 &other) const;
    bool operator!=(const XMFLOAT4X4 &other) const;

    float determinant() const;
    Matrix4x4 inverse() const;
    bool isIdentity() const;
    Quaternion rotation() const;
    Matrix4x4 transpose() const;

    Vector4 GetColumn(int index) const;
    Vector4 GetRow(int index) const;
    void GetTRS(Vector3 *pos, Quaternion *rot, Vector3 *scale) const;
    void SetColumn(int index, const Vector4 &column);
    void SetRow(int index, const Vector4 &row);
    void SetTRS(const Vector3 &pos, const Quaternion &q, const Vector3 &s);

    Vector3 MultiplyPoint(const Vector3 &point) const; // 4x4 부분을 곱셈. 스케일링을 포함한 위치 변환
    Vector3 MultiplyVector(const Vector3 &vector) const; // 3x3 부분만 곱셈. 방향 변환


public:
    static const Matrix4x4 identity;
    static const Matrix4x4 zero;

    static Matrix4x4 Translate(const Vector3 &pos);
    static Matrix4x4 Rotate(const Quaternion &q);
    static Matrix4x4 Scale(const Vector3 &s);

    static Matrix4x4 LookAt(const Vector3 &eye, const Vector3 &lookat, const Vector3 &dir);
    static Matrix4x4 TRS(const Vector3 &pos, const Quaternion &q, const Vector3 &s);

    static Matrix4x4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
    static Matrix4x4 Perspective(float fov_y, float aspect, float zNear, float zFar);
};
