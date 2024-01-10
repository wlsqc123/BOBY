#include "stdafx.h"
#include "Player.h"
#include "Camera.h"

Matrix4x4 gmtxProjectToTexture
{
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::identity;
	m_xmf4x4Projection = Matrix4x4::identity;
	m_xmf4x4ViewProjTex = Matrix4x4::identity;
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = Vector3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = Vector3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	playerObject = NULL;
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
	{
		*this = *pCamera;
	}
	else
	{
		m_xmf4x4View = Matrix4x4::identity;
		m_xmf4x4Projection = Matrix4x4::identity;
		m_xmf4x4ViewProjTex = Matrix4x4::identity;
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = Vector3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = Vector3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;
		playerObject = NULL;
	}
}

CCamera::~CCamera()
{ 
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::Perspective(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
}

void CCamera::GenerateViewMatrix(Vector3 xmf3Position, Vector3 xmf3LookAt, Vector3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAt(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void CCamera::RegenerateViewMatrix()
{
	m_xmf3Look = m_xmf3Look.normalized();
	m_xmf3Right = Vector3::CrossNormal(m_xmf3Up, m_xmf3Look);
	m_xmf3Up = Vector3::CrossNormal(m_xmf3Look, m_xmf3Right);
	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::Dot(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::Dot(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::Dot(m_xmf3Position, m_xmf3Look);

	GenerateFrustum();
}

void CCamera::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void **)&m_pcbMappedCamera);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_xmf4x4ViewProjTex = m_xmf4x4View * m_xmf4x4Projection * gmtxProjectToTexture;
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4ViewProjTex, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4ViewProjTex)));
	::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(Vector3));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(CAMERA_CBV, d3dGpuVirtualAddress);
}

void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
	}
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::GenerateFrustum()
{

	m_xmFrustum.CreateFromMatrix(m_xmFrustum, XMLoadFloat4x4(&m_xmf4x4Projection));
	XMMATRIX xmmtxInversView = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4View));
	m_xmFrustum.Transform(m_xmFrustum, xmmtxInversView);
}

bool CCamera::IsInFrustum(BoundingOrientedBox& xmBoundingBox)
{
	return(m_xmFrustum.Intersects(xmBoundingBox));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSpaceShipCamera

CSpaceShipCamera::CSpaceShipCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	Matrix4x4 matRotate;
	if (playerObject && (x != 0.0f))
	{
		Vector3 xmf3Right = playerObject->GetRightVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Right), XMConvertToRadians(x));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
		m_xmf3Position = m_xmf3Position * matRotate;
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
	}
	if (playerObject && (y != 0.0f))
	{
		Vector3 xmf3Up = playerObject->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
		m_xmf3Position = m_xmf3Position * matRotate;
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
	}
	if (playerObject && (z != 0.0f))
	{
		Vector3 xmf3Look = playerObject->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
		m_xmf3Position = m_xmf3Position * matRotate;
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFirstPersonCamera

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = m_xmf3Right.normalized();
			m_xmf3Look = m_xmf3Look.normalized();
		}
	}
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	Matrix4x4 matRotate;
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
	}
	if (playerObject && (y != 0.0f))
	{
		Vector3 xmf3Up = playerObject->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
	}
	if (playerObject && (z != 0.0f))
	{
		Vector3 xmf3Look = playerObject->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		XMStoreFloat4x4(&matRotate, xmmtxRotate);
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
		m_xmf3Position = m_xmf3Position * matRotate;
		m_xmf3Position = m_xmf3Position + playerObject->GetPosition();
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
	}
}

void CFirstPersonCamera::SetLookAt(Vector3& xmf3LookAt)
{
	//Matrix4x4 mtxLookAt = Matrix4x4::LookAt(m_xmf3Position, xmf3LookAt, gsPlayer->GetUpVector());
	//mtxLookAt = gsPlayer->worldMatrix* mtxLookAt;
	//string s;
	//s += "player:  " + to_string(mtxLookAt._41) + " " + to_string(mtxLookAt._42) + " " + to_string(mtxLookAt._43) + "\n";
	//OutputDebugStringA(s.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CThirdPersonCamera

CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = m_xmf3Right.normalized();
			m_xmf3Look = m_xmf3Right.normalized();
		}
	}
}

void CThirdPersonCamera::Update(Vector3& xmf3LookAt, float fTimeElapsed)
{
	if (playerObject)
	{
		Matrix4x4 xmf4x4Rotate = Matrix4x4::identity;
		Vector3 xmf3Right = playerObject->GetRightVector();
		Vector3 xmf3Up = playerObject->GetUpVector();
		Vector3 xmf3Look = playerObject->GetLookVector();
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		Vector3 xmf3Offset = m_xmf3Offset * xmf4x4Rotate;
		Vector3 xmf3Position = playerObject->GetPosition()+ xmf3Offset;
		Vector3 xmf3Direction = xmf3Position - m_xmf3Position;
		float fLength = xmf3Direction.length();
		xmf3Direction = xmf3Direction.normalized();
		float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = m_xmf3Position + (xmf3Direction * fDistance);
			SetLookAt(xmf3LookAt);
		}
	}
}

void CThirdPersonCamera::SetLookAt(Vector3& xmf3LookAt)
{
	Matrix4x4 mtxLookAt = Matrix4x4::LookAt(m_xmf3Position, xmf3LookAt, playerObject->GetUpVector());
	m_xmf3Right = Vector3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = Vector3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = Vector3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}
