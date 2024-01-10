//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

//-----------------------------------------------------------------------------
// 파일 입출력용
int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

int ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	int nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(int), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}
//-----------------------------------------------------------------------------

CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture) m_pTexture->Release();
	if (m_pShader) m_pShader->Release();
}


void CMaterial::SetTexture(CTexture *pTexture)
{
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CMaterial::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) m_pShader->ReleaseShaderVariables();
	if (m_pTexture) m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(int nMeshes)
{
	worldMatrix = Matrix4x4::identity;

	numMeshes = nMeshes;
	meshes = NULL;
	if (numMeshes > 0)
	{
		meshes = new CMesh*[numMeshes];
		for (int i = 0; i < numMeshes; i++)	meshes[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->Release();
			meshes[i] = NULL;
		}
		delete[] meshes;
	}
	if (objectMaterial) objectMaterial->Release();
}

void CGameObject::SetMesh(int nIndex, CMesh *pMesh)
{
	if (meshes)
	{
		if (meshes[nIndex]) meshes[nIndex]->Release();
		meshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}

void CGameObject::SetShader(CShader *pShader)
{
	if (!objectMaterial)
	{
		CMaterial *pMaterial = new CMaterial();
		SetMaterial(pMaterial);
	}
	if (objectMaterial) objectMaterial->SetShader(pShader);
}

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (objectMaterial) objectMaterial->Release();
	objectMaterial = pMaterial;
	if (objectMaterial) objectMaterial->AddRef();
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	cbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	cbGameObject->Map(0, NULL, (void **)&cbMappedGameObject);
}

void CGameObject::ReleaseShaderVariables()
{
	if (cbGameObject)
	{
		cbGameObject->Unmap(0, NULL);
		cbGameObject->Release();
	}
	if (objectMaterial) objectMaterial->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&cbMappedGameObject->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	if (objectMaterial) cbMappedGameObject->m_nMaterial = nMat;
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender();

	if (objectMaterial)
	{
		if (objectMaterial->m_pShader)
		{
			objectMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			objectMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (objectMaterial->m_pTexture)
		{
			objectMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
		pd3dCommandList->SetGraphicsRootDescriptorTable(OBJECT_CBV, cbvGPUDescriptorHandle);
	}


	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->Render(pd3dCommandList);
		}
	}
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances)
{
	OnPrepareRender();

	if (objectMaterial)
	{
		if (objectMaterial->m_pShader)
		{
			objectMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			objectMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (objectMaterial->m_pTexture)
		{
			objectMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}


	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->Render(pd3dCommandList, nInstances);
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->ReleaseUploadBuffers();
		}
	}

	if (objectMaterial) objectMaterial->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	worldMatrix._41 = x;
	worldMatrix._42 = y;
	worldMatrix._43 = z;
}

void CGameObject::SetPosition(Vector3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

Vector3 CGameObject::GetPosition()
{
	return(Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43));
}

Vector3 CGameObject::GetLook()
{
	return(Vector3(worldMatrix._31, worldMatrix._32, worldMatrix._33).normalized());
}

Vector3 CGameObject::GetUp()
{
	return(Vector3(worldMatrix._21, worldMatrix._22, worldMatrix._23).normalized());
}

Vector3 CGameObject::GetRight()
{
	return(Vector3(worldMatrix._11, worldMatrix._12, worldMatrix._13).normalized());
}

void CGameObject::MoveStrafe(float fDistance)
{
	Vector3 xmf3Position = GetPosition();
	Vector3 xmf3Right = GetRight();
	xmf3Position = xmf3Position + (xmf3Right * fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	Vector3 xmf3Position = GetPosition();
	Vector3 xmf3Up = GetUp();
	xmf3Position = xmf3Position + (xmf3Up * fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	Vector3 xmf3Position = GetPosition();
	Vector3 xmf3Look = GetLook();
	xmf3Position = xmf3Position + (xmf3Look * fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	Matrix4x4 xmf4x4Rotate;
	XMStoreFloat4x4(&xmf4x4Rotate, mtxRotate);
	worldMatrix = xmf4x4Rotate * worldMatrix;
}

void CGameObject::Rotate(Vector3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	Matrix4x4 xmf4x4Rotate;
	XMStoreFloat4x4(&xmf4x4Rotate, mtxRotate);
	worldMatrix = xmf4x4Rotate * worldMatrix;
}

bool CGameObject::ColideObjectByRayIntersection(Vector3& position, Vector3& direction, float* distance)
{
	XMVECTOR xmRayOrigin = XMLoadFloat3(&position);
	XMVECTOR xmRayDirection = XMLoadFloat3(&direction);
	return objectBoundingBox.Intersects(xmRayOrigin, xmRayDirection, *distance);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	Matrix4x4 xmf4x4Scale;
	XMStoreFloat4x4(&xmf4x4Scale, mtxScale);
	worldMatrix = xmf4x4Scale * worldMatrix;
}

void CGameObject::SetName(const char* name)
{
	objectName = name;
}

void CGameObject::PrintBoundingBox()
{
	cout << objectName << " : Center :(" << objectBoundingBox.Center.x << ", " << objectBoundingBox.Center.y << ", " << objectBoundingBox.Center.z << ") Extent :("
		<< objectBoundingBox.Extents.x << ", " << objectBoundingBox.Extents.y << ", " << objectBoundingBox.Extents.z << ") " << endl;
}

bool CGameObject::IsInFrustum(CCamera* camera)
{
	return camera->IsInFrustum(objectBoundingBox);
}

CTestWallObject::CTestWallObject(int nMeshes)
{
}

CTestWallObject::~CTestWallObject()
{
}

void CTestWallObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, Vector3 xmf3Scale) : CGameObject(0)
{
	CTexture* pTerrainTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1);

	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/detail_Texture_7.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/tileImage.dds", RESOURCE_TEXTURE2D, 1);
	//바닥에 더해지는 이미지
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/map1.dds", RESOURCE_TEXTURE2D, 2);
	CMaterial* pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);

	SetMaterial(pTerrainMaterial);

	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	numMeshes = cxBlocks * czBlocks;
	meshes = new CMesh*[numMeshes];
	for (int i = 0; i < numMeshes; i++)	meshes[i] = NULL;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	//바닥 만드는 곳
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, m_pHeightMapImage);
			SetMesh(x + (z*cxBlocks), pHeightMapGridMesh);
		}
	}

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 3);
	pTerrainShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pTerrainTexture, 0, TERRAIN_SRV);

	SetCbvGPUDescriptorHandle(pTerrainShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pTerrainShader);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(0,pSkyBoxMesh);
	
	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/SkyBox.dds", RESOURCE_TEXTURE_CUBE, 0);

	CMaterial* pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);

	SetMaterial(pSkyBoxMaterial);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	
	
	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pSkyBoxShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1,1);
	pSkyBoxShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pSkyBoxShader->CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, SKYBOX_SRV);

	SetCbvGPUDescriptorHandle(pSkyBoxShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pSkyBoxShader);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	Vector3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	OnPrepareRender();

	if (objectMaterial)
	{
		if (objectMaterial->m_pShader)
		{
			objectMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			objectMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(OBJECT_CBV, cbvGPUDescriptorHandle);

	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (objectMaterial)
			{
				if (objectMaterial->m_pTexture) objectMaterial->m_pTexture->UpdateShaderVariable(pd3dCommandList, 0, i);				
			}
			if (meshes[i]) meshes[i]->Render(pd3dCommandList);
		}
	}
}

CFbxModelObject::CFbxModelObject(int nMeshes):CGameObject(nMeshes)
{
}

CFbxModelObject::~CFbxModelObject()
{
	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->~CFbxModelObject();
	}
	if (m_pModelAnimator)
	{
		m_pModelAnimator->~ModelAnimator();
	}
}

void CFbxModelObject::SetChild(CFbxModelObject* child, int index)
{
	child->m_Parent = this;
	m_Child.push_back(child);
}


void CFbxModelObject::UpdateTransform(Matrix4x4* parent)
{
	worldMatrix = (parent) ? m_Transform * (*parent) : m_RotationMatrix * m_Transform;
	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->UpdateTransform(&worldMatrix);
	}
}

void CFbxModelObject::Update()
{
	if (m_pModelAnimator)
		m_pModelAnimator->Update(currentAniNum, currentAniTime);
}

void CFbxModelObject::DebugWorldMatrix()
{
	std::string s;
	s += objectName + "  " + std::to_string(worldMatrix._41) + " "+ std::to_string(worldMatrix._42)+" "+ std::to_string(worldMatrix._43)+"\n";
}


void CFbxModelObject::SetPosition(const Vector3& xmf3Position)
{
	m_Transform *= Matrix4x4::Translate(xmf3Position);
	UpdateTransform(NULL);
}

void CFbxModelObject::SetScale(float x, float y, float z)
{
	CGameObject::SetScale(x,y,z);
	m_Transform *= Matrix4x4::Scale(Vector3(x, y, z));
	UpdateTransform(NULL);
}

void CFbxModelObject::SetScaleInverse(float x, float y, float z)
{
	CGameObject::SetScale(x, y, z);
	m_Transform = Matrix4x4::Scale(Vector3(x, y, z)) * m_Transform;
	UpdateTransform(NULL);
}

void CFbxModelObject::SetDirectionWithLookVector(Vector3 Look)
{
	Look.normalized();
	Vector3 Up(0,1,0);
	Vector3 Right = Vector3::CrossNormal(Up, Look);
	Up = Vector3::CrossNormal(Look, Right);
	m_RotationMatrix._11 = Right.x;
	m_RotationMatrix._12 = Right.y;
	m_RotationMatrix._13 = Right.z;
	m_RotationMatrix._21 = Up.x;
	m_RotationMatrix._22=  Up.y;
	m_RotationMatrix._23 = Up.z;
	m_RotationMatrix._31 = Look.x;
	m_RotationMatrix._32 = Look.y;
	m_RotationMatrix._33 = Look.z;


	Matrix4x4 rotate = Matrix4x4::Rotate(Quaternion::AngleAxis(90, Vector3(0, 1, 0)));

	m_RotationMatrix = rotate* m_RotationMatrix;
	UpdateTransform(NULL);
}

void CFbxModelObject::SetDirectionWithPlayerLookVector(Vector3 Look)
{
	Look.normalized();
	Vector3 Up(0, 1, 0);
	Vector3 Right = Vector3::CrossNormal(Up, Look);
	Up = Vector3::CrossNormal(Look, Right);
	m_RotationMatrix._11 = Right.x;
	m_RotationMatrix._12 = Right.y;
	m_RotationMatrix._13 = Right.z;
	m_RotationMatrix._21 = Up.x;
	m_RotationMatrix._22 = Up.y;
	m_RotationMatrix._23 = Up.z;
	m_RotationMatrix._31 = Look.x;
	m_RotationMatrix._32 = Look.y;
	m_RotationMatrix._33 = Look.z;


	Matrix4x4 rotate = Matrix4x4::Rotate(Quaternion::AngleAxis(90, Vector3(0, 1, 0)));

	Matrix4x4 rotatex = Matrix4x4::Rotate(Quaternion::AngleAxis(90, Vector3(0, 0, 1)));
	m_RotationMatrix = rotate * m_RotationMatrix;
	UpdateTransform(NULL);
}

void CFbxModelObject::SetDirectionWithMinoLookVector(Vector3 Look)
{
	Look.normalized();
	Vector3 Up(0, 1, 0); 
	Vector3 Right = Vector3::CrossNormal(Up, Look);
	Up = Vector3::CrossNormal(Look, Right);
	m_RotationMatrix._11 = Right.x;
	m_RotationMatrix._12 = Right.y;
	m_RotationMatrix._13 = Right.z;
	m_RotationMatrix._21 = Up.x;
	m_RotationMatrix._22 = Up.y;
	m_RotationMatrix._23 = Up.z;
	m_RotationMatrix._31 = Look.x;
	m_RotationMatrix._32 = Look.y;
	m_RotationMatrix._33 = Look.z;

	Matrix4x4 rotate = Matrix4x4::Rotate(Quaternion::AngleAxis(180, Vector3(0, 1, 0)));

	Matrix4x4 rotatex = Matrix4x4::Rotate(Quaternion::AngleAxis(-90, Vector3(1, 0, 0)));
	m_RotationMatrix = rotatex * rotate* m_RotationMatrix;
	UpdateTransform(NULL);
}

void CFbxModelObject::Move(Vector3 pos)
{
	Move(m_Transform._41 - pos.x, m_Transform._42 - pos.y, m_Transform._43 - pos.z);
}

void CFbxModelObject::Move(float x, float y, float z)
{
	m_Transform._41 -= x;
	m_Transform._42 -= y;
	m_Transform._43 -= z;
	UpdateTransform(NULL);
}

void CFbxModelObject::Rotate(Vector3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	Matrix4x4 matRotate;XMStoreFloat4x4(&matRotate, mtxRotate);
	m_Transform *= matRotate;

	UpdateTransform(NULL);
}

void CFbxModelObject::Rotate(float x, float y, float z)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	Matrix4x4 xmf4x4Rotate;
	XMStoreFloat4x4(&xmf4x4Rotate, mtxRotate);
	m_Transform *= xmf4x4Rotate;

	UpdateTransform(NULL);
}

void CFbxModelObject::SetWorld(Matrix4x4 World)
{
	m_Transform = World;

	UpdateTransform(NULL);
}

void CFbxModelObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	UpdateShaderVariables(pd3dCommandList);
	if (objectMaterial)
	{
		if (objectMaterial->m_pShader)
		{
			objectMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			objectMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);
		}
		if (objectMaterial->m_pTexture)
		{  
			objectMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}

		pd3dCommandList->SetGraphicsRootDescriptorTable(OBJECT_CBV, cbvGPUDescriptorHandle);
	}
	pd3dCommandList->SetGraphicsRootConstantBufferView(SKINNEDBONETRANSFORMS_CBV, BoneTransforms->GetGPUVirtualAddress()); //Materials
	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->Render(pd3dCommandList);
		}
	}
	
	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->Render(pd3dCommandList, pCamera);
	}
}

void CFbxModelObject::SetSsaoMaterial(UINT n)
{
	nMat = n;
	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->SetSsaoMaterial(n);
	}
}

void CFbxModelObject::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();
	if (meshes)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			if (meshes[i]) meshes[i]->Release();
			meshes[i] = NULL;
		}
		delete[] meshes;
	}

	if (BoneTransforms)
	{
		BoneTransforms->Unmap(0, NULL);
		BoneTransforms->Release();
	}

	if (objectMaterial) objectMaterial->Release();

	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->ReleaseShaderVariables();
	}
}

void CFbxModelObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//1. 여기에 map을 해주자
	UINT ncbElementBytes = ((sizeof(CB_BONE_TRANSFORMS) + 255) & ~255); //256의 배수
	BoneTransforms = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	BoneTransforms->Map(0, NULL, (void**)&MappedBoneTransforms);
}

void CFbxModelObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
	::memcpy(MappedBoneTransforms, transforms.data(), sizeof(Matrix4x4) * transforms.size());
}

void CFbxModelObject::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_Child.size(); i++)
	{
		m_Child[i]->ReleaseUploadBuffers();
	}
	CGameObject::ReleaseUploadBuffers();
}

CFbxModelObject* CFbxModelObject::LoadObjectHierarchy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,aiNode* node, const aiScene* scene, std::map< string, TEXTURE_INFO> textures, CShader* shader, int* objectNum, bool isRoot, const char* filename, bool isSkin, CFbxModelObject* parent, CFbxModelObject* root)
{
	std::map<std::string, UINT> boneDataMap;

	CFbxModelObject* pFbxModelObject = new CFbxModelObject(node->mNumMeshes);
	//노드의 이름저장
	pFbxModelObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pFbxModelObject->SetName(node->mName.C_Str());
	pFbxModelObject->worldMatrix = Matrix4x4::identity;
	CTexture* pTexture;

	aiMatrix4x4 aitrs = node->mTransformation;
	pFbxModelObject->m_Transform = Matrix4x4(aitrs).transpose();
	pFbxModelObject->m_nodeTransform = Matrix4x4(aitrs);
	//노드에 메시가있으면 메시저장
	string materialName;
	if (node->mNumMeshes != 0)
	{
		pTexture = new CTexture(node->mNumMeshes * 2, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	}
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if (mesh->mMaterialIndex >= 0)
		{
			materialName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
			TEXTURE_INFO texInfo = textures.find(materialName)->second;
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, texInfo.diffuse, RESOURCE_TEXTURE2D, i * 2);
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, texInfo.normal, RESOURCE_TEXTURE2D, i * 2 + 1);
		}


		if (mesh->HasBones())
		{
			for (UINT j = 0; j < mesh->mNumBones; j++)
			{
				aiBone* aibone = mesh->mBones[j];
				BoneInfo bone;
				bone.boneName = aibone->mName.C_Str();
				bone.boneOffset = Matrix4x4(aibone->mOffsetMatrix);
				
				bone.finalTransform = Matrix4x4::identity;
				
				pFbxModelObject->boneInfos.emplace_back(bone);
				auto i = std::find_if(root->rootBoneInfos.begin(), root->rootBoneInfos.end(), [=](const BoneInfo& a) {
					return a.boneName == bone.boneName;
					});
				if (i == root->rootBoneInfos.end())
				{
					boneDataMap[bone.boneName] = root->rootBoneInfos.size();
					root->rootBoneInfos.push_back(bone);
				}
			}
			if (!root->rootBoneInfos.empty())
			{
				for (int t = 0; t < root->rootBoneInfos.size(); t++)
				{
					boneDataMap[root->rootBoneInfos[t].boneName] = t;
				}
			}
				 
			if (isSkin)
			{
				aiMatrix4x4 aitrs = node->mTransformation;
				pFbxModelObject->m_Transform = Matrix4x4::identity;
			}
		}
		CFbxHierarchyMesh* fbxmesh = new CFbxHierarchyMesh(pd3dDevice, pd3dCommandList, mesh, scene, i, boneDataMap);
		pFbxModelObject->SetMesh(i, fbxmesh);
	}

	if (node->mNumMeshes!= 0)
	{
		CMaterial* pCubeMaterial = new CMaterial();
		pCubeMaterial->SetTexture(pTexture);
		pCubeMaterial->SetReflection(1);
		pFbxModelObject->SetMaterial(pCubeMaterial);

		UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
		int num = *objectNum;
		shader->CreateConstantBufferViews(pd3dDevice,pFbxModelObject->cbGameObject, ncbElementBytes,num);
		shader->CreateShaderResourceViews(pd3dDevice, pTexture, 0, TEXTURE_SRV);

		pFbxModelObject->SetCbvGPUDescriptorHandlePtr(shader->GetGPUCbvDescriptorStartHandle().ptr + static_cast<UINT64>(::gnCbvSrvDescriptorIncrementSize * num));
		*objectNum += 1;
		pFbxModelObject->SetShader(shader);

	}
	
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		if (isRoot)
		{
			CFbxModelObject* pchild = CFbxModelObject::LoadObjectHierarchy(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, node->mChildren[i], scene, textures, shader, objectNum, false, filename, isSkin, pFbxModelObject, pFbxModelObject);
			pFbxModelObject->m_Child.push_back(pchild);
		}
		else
		{
			CFbxModelObject* pchild = CFbxModelObject::LoadObjectHierarchy(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, node->mChildren[i], scene, textures, shader, objectNum, false, filename, isSkin, pFbxModelObject, root);
			pFbxModelObject->m_Child.push_back(pchild);
		}
	}


	if (parent)
		pFbxModelObject->m_Parent = parent;

	if (pFbxModelObject->m_Parent == NULL)
			//1번 달리기, 2번 걷기 .... 이런식으로 저장
			pFbxModelObject->m_pModelAnimator = new ModelAnimator(scene, pFbxModelObject, filename);
			//pFbxModelObject->m_pModelAnimator->m_nAnimationCount = pFbxModelObject->m_pModelAnimator->m_vClips.size();
		

	return pFbxModelObject;
}

void CFbxModelObject::AnimationChange(int num)
{
	switch (num)
	{
	case 4:
		//die
		if (animationManual["death"] == 0 || animationManual["death"])
		{
			if (currentAniNum != animationManual["death"])
			{
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
				currentAniNum = animationManual["death"];
				isAniChange = true;

				currentAniTime = std::chrono::system_clock::now();
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;

				if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
					m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				m_pModelAnimator->aniOncePlay = false;
				m_pModelAnimator->isCurrentAniAttack = false;

			}
			isAniChange = false;
		}
		break;
	case 2:
		//attack
		if (animationManual["attack"] == 0 || animationManual["attack"])
		{
			if (currentAniNum != animationManual["attack"])
			{
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
				currentAniNum = animationManual["attack"];
				isAniChange = true;
				currentAniTime = std::chrono::system_clock::now();
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
				if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
					m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				m_pModelAnimator->aniOncePlay = false;
				m_pModelAnimator->isCurrentAniAttack = true;
			}
				isAniChange = false;
		}
		break;
	case 3:
		//walk (hit)
		if (animationManual["walk"] == 0 || animationManual["walk"])
		{
			if (currentAniNum != animationManual["walk"])
			{
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
				currentAniNum = animationManual["walk"];

				currentAniTime = std::chrono::system_clock::now();
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
				if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
					m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				m_pModelAnimator->isCurrentAniAttack = false;

			}
		}
		break;
	case 0:
		//idle
		if (animationManual["idle"] == 0 || animationManual["idle"])
		{
			if (currentAniNum != animationManual["idle"])
			{
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
				currentAniNum = animationManual["idle"];

				currentAniTime = std::chrono::system_clock::now();
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
				if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
					m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				m_pModelAnimator->isCurrentAniAttack = false;

			}
		}
		break;
	case 1:
		//walk
		if (animationManual["walk"] == 0 || animationManual["walk"])
		{
			if (currentAniNum != animationManual["walk"])
			{
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
				currentAniNum = animationManual["walk"];
				currentAniTime = std::chrono::system_clock::now();
				m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
				if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
					m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				m_pModelAnimator->isCurrentAniAttack = false;

			}
		}
		break;
	case 9:
		//reload
		if (animationManual["reload"] == 0 || animationManual["reload"])
		{
			if (currentAniNum != animationManual["reload"])
			{
				if (!m_pModelAnimator->aniOnceIsPlaying)
				{
					m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
					currentAniNum = animationManual["reload"];
					currentAniTime = std::chrono::system_clock::now();
					m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
					if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
						m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
					m_pModelAnimator->aniOncePlay = false;
				}
			}
		}
		break;
	case 8:
		//player
		if (animationManual["playeridle"] == 0 || animationManual["playeridle"])
		{
			if (currentAniNum != animationManual["playeridle"])
			{
				if (!m_pModelAnimator->aniOnceIsPlaying)
				{
					m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
					currentAniNum = animationManual["playeridle"];

					currentAniTime = std::chrono::system_clock::now();
					m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
					if (m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse)
						m_pModelAnimator->animationTrack[currentAniNum]->animationDurationReverse = false;
				}
			}
		}
		break;
	default:
		break;
	}
}

void CFbxModelObject::IsOpen()
{
	if (!is_open)	//거짓이면 열리는 애니메이션
	{
		m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = false;
		currentAniNum = animationManual["opening"];
		m_pModelAnimator->animationTrack[currentAniNum]->trackEnable = true;
		is_open = true;
		currentAniTime = std::chrono::system_clock::now();
	}
}

CFbxModelObject* CFbxModelObject::LoadFbxRootObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, bool ccw, std::map<string, TEXTURE_INFO> textures, CShader* shader, const aiScene* scene, bool isSkin, const char* filename)
{
	int* numptr;
	int objectnum = 0;
	numptr = &objectnum;
	
	CFbxModelObject* pFbxModelObject = CFbxModelObject::LoadObjectHierarchy(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,scene->mRootNode, scene, textures, shader,numptr, true, filename, isSkin);
	return pFbxModelObject;
}

ModelAnimator::ModelAnimator(const aiScene* scene, CFbxModelObject* fbxModelObejct, const char* filename)
{
	if (fbxModelObejct)
	{
		rootNode = fbxModelObejct;
		for (int i = 0; i < rootNode->rootBoneInfos.size(); i++)
		{
			addBoneTransforms.push_back(Matrix4x4::zero);
		}
	}

	if (scene->HasAnimations())
	{
		SetAnimationClip(scene);
	}
	if (filename != NULL)
	{
		modelName = filename;
	}
}

ModelAnimator::~ModelAnimator()
{

}

void ModelAnimator::Update(int animationNum, std::chrono::system_clock::time_point animationTime)
{

	for (auto a : animationTrack)
	{
		if (a->trackEnable)
		{

			if ((animationTime != currentPlayAnimationStartTime))
			{
				currentPlayAnimationStartTime = animationTime;
			}

			std::chrono::duration<float> sec = std::chrono::system_clock::now() - currentPlayAnimationStartTime;

			//실행할 애니메이션 선택
			currentPlayAnimationNum = a->clipNumber;

			float timeSpeed = sec.count() * a->animationSpeed;

			//애니메이션 시간을 넘어가면? 0이 아닌 1 이상이 나온다.
			//결과가 짝수면? 정상
			//결과가 홀수면? 반대로
			float reverseCheck = timeSpeed / a->timeEndPosition;

			UINT checkCalc = static_cast<UINT>(floor(reverseCheck));

			switch (a->animationPlayType)
			{
			case ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE:
				
				if (checkCalc >= 1)
				{
					timeSpeed = a->timeStartPosition + a->timeEndPosition - 0.1f;
					aniOncePlay = true;
					aniOnceIsPlaying = false;
				}
				else
				{
					aniOnceIsPlaying = true;
					aniOncePlay = false;
				}
				break;
			case ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP:
				break;
			case ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG:
				if (checkCalc % 2 == 0)
				{
					a->animationDurationReverse = false;
				}

				if (checkCalc % 2 == 1)
				{
					a->animationDurationReverse = true;
				}
				break;
			}
			
			ExtractBoneTransforms(timeSpeed, currentPlayAnimationNum, a);
			SetBoneTransform(rootNode, rootNode->rootBoneInfos, a);
		}
	}

}

void ModelAnimator::SetAnimationClip(const aiScene* scene)
{
	for (UINT i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* animation = scene->mAnimations[i];

		asClip* clip = new asClip();
		//애니메이션 클립 이름
		clip->clipName = animation->mName.C_Str();
		
		//애니메이션 프레임 총 개수 + 1
		clip->Duration = (UINT)animation->mDuration;

		//Channel (애니메이션의 Bone정보가 들어가 있는 곳)
		for (UINT j = 0; j < animation->mNumChannels; j++)
		{
			aiNodeAnim* aniNode = animation->mChannels[j];

			asClipNode* aniNodeInfo = new asClipNode();
			aniNodeInfo->clipNodeName = aniNode->mNodeName.C_Str();

			asKeyFramedataVector3 frameDataScale;
			asKeyFramedataQuaternion frameDataRotation;
			asKeyFramedataVector3 frameDataTranslation;

			//position값 저장
			for (int k = 0; k < aniNode->mNumPositionKeys; k++)
			{
				aiVectorKey key = aniNode->mPositionKeys[k];
				frameDataTranslation.value = Vector3(key.mValue.x, key.mValue.y, key.mValue.z);
				frameDataTranslation.Time = (float)aniNode->mPositionKeys[k].mTime;
				aniNodeInfo->KeyframeTranslation.push_back(frameDataTranslation);
			}

			//Rotation
			for (int k = 0; k < aniNode->mNumRotationKeys; k++)
			{
				aiQuatKey key = aniNode->mRotationKeys[k];
				frameDataRotation.value = Vector4(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
				frameDataRotation.Time = (float)aniNode->mRotationKeys[k].mTime;
				aniNodeInfo->KeyframeRotation.push_back(frameDataRotation);
			}

			//Scale
			for (int k = 0; k < aniNode->mNumScalingKeys; k++)
			{
				aiVectorKey key = aniNode->mScalingKeys[k];
				frameDataScale.value = Vector3(key.mValue.x, key.mValue.y, key.mValue.z);
				frameDataScale.Time = (float)aniNode->mScalingKeys[k].mTime;
				aniNodeInfo->KeyframeScale.push_back(frameDataScale);
			}

			clip->keyFrames.push_back(aniNodeInfo);
		}

		animationClips.emplace_back(clip);
	}

	currentPlayAnimationNum = 0;
	currentPlayAnimationStartTime = std::chrono::system_clock::now();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelAnimator::ExtractBoneTransforms(float animationTime, const int animationIndex, asAnimationTrack* track)
{
	animationTime = fmod(animationTime, track->timeEndPosition);

	float animationTimeCale = animationTime;
	animationTimeCale += track->timeStartPosition;
	if (track->animationDurationReverse)			//역방향 시간 흐름(reverse)
		animationTimeCale = track->timeEndPosition - animationTimeCale;
	ReadNodeHierarchy(animationTimeCale, animationIndex, track, rootNode, Matrix4x4::identity);
}

void ModelAnimator::ReadNodeHierarchy(float animationTime, int animationClipNumber, asAnimationTrack* track, CFbxModelObject* node, const Matrix4x4& parentTransform)
{
	asClip* animationClip = animationClips[animationClipNumber];
	Matrix4x4 nodeTransform = Matrix4x4(node->m_nodeTransform);

 	asClipNode* nodeAnim = FindNodeAnim(animationClip, node->GetName());

	if (nodeAnim)
	{
		Vector3 transform;
		Quaternion rotation;
		Vector3 scale;
		
		transform = CalcInterpolatedVectorFromKey(animationTime, nodeAnim->KeyframeTranslation.size(), nodeAnim->KeyframeTranslation);
		rotation = CalcInterpolatedQuaternionFromKey(animationTime, nodeAnim->KeyframeRotation.size(), nodeAnim->KeyframeRotation);
		scale = CalcInterpolatedVectorFromKey(animationTime, nodeAnim->KeyframeScale.size(), nodeAnim->KeyframeScale);

		//nodeTransform = (Matrix4x4(transform, rotation, scale) * track->animationWeight).transpose();
		nodeTransform = Matrix4x4(transform, rotation, scale).transpose();

		node->m_Transform = nodeTransform.transpose();
	}
	Matrix4x4 globalTransform = parentTransform * nodeTransform;
	

	for (int k = 0; k < rootNode->rootBoneInfos.size(); k++)
	{
		if (rootNode->rootBoneInfos[k].boneName == node->GetName())
		{		
			//addBoneTransforms[k] = globalTransform * track->animationWeight/* * rootNode->rootBoneInfos[k].boneOffset*/;
			rootNode->rootBoneInfos[k].finalTransform = globalTransform * track->animationWeight /** rootNode->rootBoneInfos[k].boneOffset*/;
			break;
		}
	}

	// 모든 자식 노드에 대해 재귀 호출
	for (int i = 0; i < node->m_Child.size(); i++)
		ReadNodeHierarchy(animationTime, animationClipNumber, track, node->m_Child[i], globalTransform);
}

void ModelAnimator::SetBoneTransform(CFbxModelObject* node, vector<BoneInfo> rootbone, asAnimationTrack* track)
{
	node->transforms.clear();
	for (UINT i = 0; i < node->boneInfos.size(); i++)
	{
		for (UINT j = 0; j < rootbone.size(); j++)
		{
			if (rootbone[j].boneName == node->boneInfos[i].boneName)
			{
				
				node->boneInfos[i].finalTransform = rootbone[j].finalTransform *rootbone[j].boneOffset/* * track->animationWeight*/;
				//node->boneInfos[i].finalTransform = addBoneTransforms[j] *rootbone[j].boneOffset/* * track->animationWeight*/;
			}
		}
	}

	for (auto a : node->boneInfos)
	{
		node->transforms.push_back(a.finalTransform);
	}

	for (int i = 0; i < node->m_Child.size(); i++)
		SetBoneTransform(node->m_Child[i], rootbone, track);
}

asClipNode* ModelAnimator::FindNodeAnim(asClip* animation, const string nodeName)
{
	for (int i = 0; i < animation->keyFrames.size(); i++)
		if (animation->keyFrames[i]->clipNodeName == nodeName)
			return animation->keyFrames[i];

	return nullptr;
}

Vector3 ModelAnimator::CalcInterpolatedVectorFromKey(float animationTime, const int numKeys, std::vector<asKeyFramedataVector3> keyFrameData)
{
	Vector3 ret;
	for (int k = 0; k < (numKeys - 1); k++)
	{
		if ((keyFrameData[k].Time <= animationTime) && (animationTime < keyFrameData[k + 1].Time))
		{
			float t = (animationTime - (float)keyFrameData[k].Time) / (keyFrameData[k + 1].Time - keyFrameData[k].Time);
			ret = keyFrameData[k].value + (keyFrameData[k + 1].value - keyFrameData[k].value) * t;
			return ret;
		}
	}
	ret = keyFrameData.back().value;
	return ret;

}

Quaternion ModelAnimator::CalcInterpolatedQuaternionFromKey(float animationTime, const int numKeys, std::vector<asKeyFramedataQuaternion> keyFrameData)
{
	Quaternion ret;
	for (int k = 0; k < (numKeys - 1); k++)
	{
		if ((keyFrameData[k].Time <= animationTime) && (animationTime < keyFrameData[k + 1].Time))
		{
			float t = (animationTime - keyFrameData[k].Time) / (keyFrameData[k + 1].Time - keyFrameData[k].Time);
			//ret = (Quaternion::AddQuaternion(Quaternion::MultyFloat(keyFrameData[k].value, (1.0f - t)), Quaternion::MultyFloat(keyFrameData[k + 1].value, t)));
			Quaternion::Interpolate(ret, keyFrameData[k].value, keyFrameData[k + 1].value, t);

			return ret;
		}
	}
	ret = keyFrameData.back().value;
	return ret;
}

void ModelAnimator::SaveAnimationTransform()
{
	for (int k = 0; k < rootNode->rootBoneInfos.size(); k++)
	{
			//여기서부터 갑자기 오류가 난다. 왤까?
			//가설 1
			// 이곳에 여러번 들어와지고 값을 저장하는데, 최종적으로 우리가 쓰던 것은 가장 마지막으로 덮어씌워진 값이여서, += 를 하면 기존 쓰레기 값들도 전부 저장되서 그렇다.

		addBoneTransforms[k] += rootNode->rootBoneInfos[k].finalTransform/* * rootNode->rootBoneInfos[k].boneOffset*/;
	}
}

void ModelAnimator::animationOutputFromFile()
{
	std::ofstream writeFile(modelName + ".txt");
	
	writeFile << animationClips.size();
	writeFile << '\n';

	for (int i = 0; i < animationClips.size(); i++)
	{
		asClip* aniClip = animationClips[i];

		writeFile << "<ClipName>:\n";
		writeFile << aniClip->clipName;
		writeFile << '\n';

		writeFile << "<Duration>:\n";
		writeFile << aniClip->Duration;
		writeFile << '\n';

		writeFile << "<KeyFrames>:\n";

		writeFile << aniClip->keyFrames.size();
		writeFile << '\n';
		for (int j = 0; j < aniClip->keyFrames.size(); j++)
		{
			asClipNode* aniNode = aniClip->keyFrames[j];

			writeFile << "<ClipNodeName>:\n";
			writeFile << aniNode->clipNodeName;
			writeFile << '\n';
			
			writeFile << "<ClipNodeKeyframeTransform>:\n";
			for (int k = 0; k < aniNode->KeyframeTranslation.size(); k++)
			{
				writeFile << aniNode->KeyframeTranslation[k].Time;
				writeFile << ' ';
				writeFile << aniNode->KeyframeTranslation[k].value.x;
				writeFile << ' ';
				writeFile << aniNode->KeyframeTranslation[k].value.y;
				writeFile << ' ';
				writeFile << aniNode->KeyframeTranslation[k].value.z;
				writeFile << ' ';

			}
			writeFile << '\n';

			writeFile << "<ClipNodeKeyframeRotation>:\n";
			for (int k = 0; k < aniNode->KeyframeRotation.size(); k++)
			{
				writeFile << aniNode->KeyframeRotation[k].Time;
				writeFile << ' ';
				writeFile << aniNode->KeyframeRotation[k].value.GetX();
				writeFile << ' ';
				writeFile << aniNode->KeyframeRotation[k].value.GetY();
				writeFile << ' ';
				writeFile << aniNode->KeyframeRotation[k].value.GetZ();
				writeFile << ' ';
				writeFile << aniNode->KeyframeRotation[k].value.GetW();
				writeFile << ' ';

			}
			writeFile << '\n';

			writeFile << "<ClipNodeKeyframeScale>:\n";
			for (int k = 0; k < aniNode->KeyframeScale.size(); k++)
			{
				writeFile << aniNode->KeyframeScale[k].Time;
				writeFile << ' ';
				writeFile << aniNode->KeyframeScale[k].value.x;
				writeFile << ' ';
				writeFile << aniNode->KeyframeScale[k].value.y;
				writeFile << ' ';
				writeFile << aniNode->KeyframeScale[k].value.z;
				writeFile << ' ';

			}
			writeFile << '\n';
		}

	}

	writeFile.close();
}

CTerrainLava::CTerrainLava(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float width, float height, int m, int n, CShader* shader) : CGameObject(1)
{
	CGridMesh* lavaMesh = new CGridMesh(pd3dDevice, pd3dCommandList, width, height, m, n);
	SetMesh(0, lavaMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* lavaTexture = new CTexture(1, RESOURCE_TEXTURE2D_ARRAY, 0, 1);

	lavaTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Lava004_2K_Color.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* lavaMaterial = new CMaterial();
	lavaMaterial->SetTexture(lavaTexture);

	SetMaterial(lavaMaterial);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	LavaShader* lavaShader = new LavaShader();
	lavaShader->SetPso(shader->GetPso());
	lavaShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	lavaShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	lavaShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	lavaShader->CreateShaderResourceViews(pd3dDevice, lavaTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(lavaShader->GetGPUCbvDescriptorStartHandle());
	SetShader(lavaShader);
}

CTerrainLava::~CTerrainLava()
{
}
