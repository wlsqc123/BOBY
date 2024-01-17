#include "stdafx.h"
#include "EffectUI.h"
#include "Shader.h"

Effect::Effect(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

Effect::~Effect()
{
}

void Effect::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수
	cbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	cbGameObject->Map(0, NULL, (void**)&cbEffect);
}

void Effect::ReleaseShaderVariables()
{
	if (cbEffect)
	{
		cbGameObject->Unmap(0, NULL);
		cbGameObject->Release();
	}
	if (objectMaterial) objectMaterial->ReleaseShaderVariables();
}

void Effect::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&cbEffect->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	cbEffect->frameCount = frameCount;
	::memcpy(&cbEffect->lookPosition, &lookPosition, sizeof(Vector3));
	cbEffect->cbTime = liveTime;
}

void Effect::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (isAlive)
	{
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

		pd3dCommandList->SetGraphicsRootConstantBufferView(EFFECT_CBV, cbGameObject->GetGPUVirtualAddress());

		if (meshes)
		{
			for (int i = 0; i < numMeshes; i++)
			{
				if (meshes[i]) meshes[i]->Render(pd3dCommandList);
			}
		}
	}
}

EffectGunFire::EffectGunFire(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) : Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;

	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/flame.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(12, -10, 100);
	geoVertex.size = Vector2(40, 40);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CEffectShader* pEffectShader = new CEffectShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

EffectGunFire::~EffectGunFire()
{
}

void EffectGunFire::Update(float fTimeElapsed, CCamera* pCamera)
{
	if (!isAlive) return;
	Matrix4x4 world = Matrix4x4::identity;
	world._11 = pCamera->GetRightVector().x; world._12 = pCamera->GetRightVector().y; world._13 = pCamera->GetRightVector().z;
	world._21 = pCamera->GetUpVector().x; world._22 = pCamera->GetUpVector().y; world._23 = pCamera->GetUpVector().z;
	world._31 = pCamera->GetLookVector().x; world._32 = pCamera->GetLookVector().y; world._33 = pCamera->GetLookVector().z;
	world._41 = pCamera->GetPosition().x;
	world._42 = pCamera->GetPosition().y;
	world._43 = pCamera->GetPosition().z;
	worldMatrix = world;
	frameCount++;
	if (frameCount > timeCount)
	{
		isAlive = false;
		frameCount = 0;
	}
	lookPosition = pCamera->GetPosition();
}


EffectCollision::EffectCollision(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;
	liveTime = 0;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/fireeffect.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	for (int i = 0; i < 40; i++)
	{
		geoVertex.position = Vector3(0, 0, 0);
		float size = Mathf::RandF(1.0f, 3.0f);
		geoVertex.size = Vector2(size, size);
		geoVertex.nWidth = textureWidth;
		geoVertex.nHeight = textureHeight;
		geoVertex.randHeight = Mathf::RandF(30.f, 200.f);
		geoVertex.randDir = Mathf::RandF(0.f, 180.f) - 90.0f;
		vecGeoVertex.push_back(geoVertex);
	}
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CFireEffectShader* pEffectShader = new CFireEffectShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

EffectCollision::~EffectCollision()
{
}

void EffectCollision::Update(float fTimeElapsed, CCamera* pCamera)
{
	if (!isAlive)
	{
		liveTime = 0;
		return;
	}
	frameCount = 0;
	if (frameCount >= timeCount)
		frameCount = 0;
	if (liveTime > 100)
	{
		isAlive = false;

	}
	liveTime += 10.f;
	lookPosition = pCamera->GetPosition();
}

EffectBlood::EffectBlood(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 3; textureHeight = 3;
	timeCount = 9;
	frameCount = 0;

	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/blood.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(40, 40);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CEffectShader* pEffectShader = new CEffectShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

void EffectBlood::Update(float fTimeElapsed, CCamera* pCamera)
{
	if (!isAlive) return;
	frameCount++;

	if (frameCount > timeCount)
	{
		isAlive = false;
		frameCount = 0;
	}
	lookPosition = pCamera->GetPosition();
}

UIHpBar::UIHpBar(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;  //Hp
	isAlive = true;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/HpBar.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(100, 5);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CHpShader* pEffectShader = new CHpShader();
	pEffectShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

UIHpBar::~UIHpBar()
{
}

UINumber::UINumber(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Vector2 pos, Vector2 size, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 4; textureHeight = 4;
	timeCount = 1;
	frameCount = 0;
	isAlive = true;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/numsest.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(pos.x, pos.y, 0);
	geoVertex.size = size;
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CUiShader* pEffectShader = new CUiShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

UINumber::~UINumber()
{
}

void UINumber::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (isAlive)Effect::Render(pd3dCommandList, pCamera);
}

UIPlayerFace::UIPlayerFace(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Vector2 pos, Vector2 size, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 3; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;
	isAlive = true;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/playerFace.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(pos.x, pos.y, 0);
	geoVertex.size = size;
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CUiShader* pEffectShader = new CUiShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

UIPlayerFace::~UIPlayerFace()
{
}

void UIPlayerFace::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (isAlive)Effect::Render(pd3dCommandList, pCamera);
}

UIPlayerHp::UIPlayerHp(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	Vector2 pos = Vector2(-0.9f, -0.9f);
	Vector2 size = Vector2(0.06f, 0.09f);

	UIPlayerFace* pf = new UIPlayerFace(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, Vector2(pos.x, -0.86),Vector2( 0.15, 0.24), shader);
	uiNumHp.push_back(pf);
	pos.x += 0.08;
	for (int i = 0; i < 3; i++)
	{
		UINumber* ph = new UINumber(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pos, size, shader);
		uiNumHp.push_back(ph);
		pos.x += (size.x);
	}
	pos.x -= 0.02;
	size.x = 0.04; size.y = 0.06; pos.y = -0.95;
	for (int i = 0; i < 3; i++)
	{
		UINumber* ph = new UINumber(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pos, size, shader);
		uiNumHp.push_back(ph);
		pos.x += (size.x);
	}
	UITexture* ph = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/hospital.dds", pos, size, Vector2(1,1), shader);
	uiNumHp.push_back(ph);
}

UIPlayerHp::~UIPlayerHp()
{
}

void UIPlayerHp::Update(float fTimeElapsed, CCamera* pCamera)
{
	for (auto a : uiNumHp) a->SetIsAlive(true);

	if (playerHp * 10 > MaxHp * 7)		uiNumHp[0]->SetFrameCount(0);
	else if (playerHp * 10 > MaxHp * 3)	uiNumHp[0]->SetFrameCount(1);
	else					uiNumHp[0]->SetFrameCount(2);

	uiNumHp[1]->SetFrameCount(playerHp / 100);
	uiNumHp[2]->SetFrameCount((playerHp / 10) % 10);
	uiNumHp[3]->SetFrameCount(playerHp % 10);

	if (playerHp / 100 == 0) uiNumHp[1]->SetIsAlive(false);
	if ((playerHp / 100 == 0) && (playerHp / 10 == 0)) uiNumHp[2]->SetIsAlive(false);

	uiNumHp[4]->SetFrameCount(MaxHp / 100);
	uiNumHp[5]->SetFrameCount((MaxHp / 10) % 10);
	uiNumHp[6]->SetFrameCount(MaxHp % 10);

	if (MaxHp / 100 == 0) uiNumHp[4]->SetIsAlive(false);
	if ((MaxHp / 100 == 0) && (MaxHp / 10 == 0)) uiNumHp[5]->SetIsAlive(false);
}

void UIPlayerHp::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (auto a : uiNumHp)
	{
		a->Render(pd3dCommandList, pCamera);
	}
}

UIMagazine::UIMagazine(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int maxBullet, CShader* shader, WEAPON_TYPE type) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	maxBullets = maxBullet;
	Vector2 pos = Vector2(0.75f, -0.9f);
	Vector2 size = Vector2(0.06, 0.09f);

	uiNumHp.reserve(5);
	UITexture* guntex;
	switch (type)
	{
	case WEAPON_RIFLE:
		guntex = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/rifleTex.dds", pos, Vector2(0.15, 0.25), Vector2(1, 1), shader);
		uiNumHp.push_back(guntex);
		break;
	case WEAPON_SNIPER:
		guntex = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/sniperTex.dds", pos, Vector2(0.15, 0.25), Vector2(1, 1), shader);
		uiNumHp.push_back(guntex);
		break;
	case WEAPON_SHOTGUN:
		guntex = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/shotgunTex.dds", pos, Vector2(0.15, 0.25), Vector2(1, 1), shader);
		uiNumHp.push_back(guntex);
		break;
	default:
		break;
	}
	pos.x += 0.1;
	pos.y = -0.9;
	for (int i = 0; i < 2; i++)
	{
		UINumber* ph = new UINumber(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pos, size, shader);
		uiNumHp.push_back(ph);
		pos.x += (size.x);
	}
	pos.x -= 0.02;
	size.x = 0.04; size.y = 0.06; pos.y = -0.95;
	for (int i = 0; i < 2; i++)
	{
		UINumber* ph = new UINumber(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pos, size, shader);
		uiNumHp.push_back(ph);
		pos.x += (size.x);
	}
	uiNumHp[3]->SetFrameCount(maxBullets / 10);
	uiNumHp[4]->SetFrameCount(maxBullets % 10);
}

UIMagazine::~UIMagazine()
{
}

void UIMagazine::Update(float fTimeElapsed, CCamera* pCamera)
{
	for (auto a : uiNumHp)
	{
		a->SetIsAlive(true);
	}
	uiNumHp[1]->SetFrameCount(numBullets / 10);
	uiNumHp[2]->SetFrameCount(numBullets % 10);
	if (numBullets / 10 == 0) uiNumHp[1]->SetIsAlive(false);
	if (maxBullets / 10 == 0) uiNumHp[3]->SetIsAlive(false);
}

void UIMagazine::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (auto a : uiNumHp)
	{
		a->Render(pd3dCommandList, pCamera);
	}
}

UIItem::UIItem(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float posX, float posY, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 3; textureHeight = 3;
	frameCount = 0;
	isAlive = false;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Item.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(posX, posY, 0);
	geoVertex.size = Vector2(0.09, 0.16);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CUiShader* pEffectShader = new CUiShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

UIItem::~UIItem()
{
}

void UIItem::SetItem(ITEM_TYPE item)
{
	frameCount = item;
	isAlive = true;
}

UITexture::UITexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pszFileName, Vector2 pos, Vector2 size, Vector2 textureSize, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = textureSize.x; textureHeight = textureSize.y;
	timeCount = 1;
	frameCount = 0;
	isAlive = true;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pszFileName, RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(pos.x, pos.y, 0);
	geoVertex.size = size;
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CUiShader* pEffectShader = new CUiShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

UITexture::~UITexture()
{
}

EffectDecal::EffectDecal(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;
	isAlive = false;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/decal.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(10, 10);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CEffectShader* pEffectShader = new CEffectShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

void EffectDecal::SetCameraPos(Vector3 pos)
{
	isAlive = true;
	lookPosition = pos;
	startTime = chrono::system_clock::now();
}

void EffectDecal::Update(float fTimeElapsed, CCamera* camera)
{
	if (!isAlive) return;
	std::chrono::duration<double> sec = chrono::system_clock::now() - startTime;
	if (sec.count() > 5) isAlive = false;
}

EffectRangeAttack::EffectRangeAttack(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;
	isAlive = false;

	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/fireball.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(20, 20);
	geoVertex.nWidth = textureWidth;
	geoVertex.nHeight = textureHeight;

	vecGeoVertex.push_back(geoVertex);

	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CEffectShader* pEffectShader = new CEffectShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

EffectRangeAttack::~EffectRangeAttack()
{
}

void EffectRangeAttack::Update(float fTimeElapsed, CCamera* camera)
{
	if (!isAlive) return;

	lookPosition = camera->GetPosition();
}

EffectBossAttack::EffectBossAttack(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader) :Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	textureWidth = 1; textureHeight = 1;
	timeCount = 1;
	frameCount = 0;
	liveTime = 0;
	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/fireeffect.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	//몬스터의 위치값을 넘겨줄수있음
	//60개의 점을 몬스터 둘레에 배치해야함
	//몬스터의 위치와 점의 위치를 가지고 나온 벡터의 노말라이즈한 방향벡터로 sin곡선을 이루게 움직이게한다
	for (int i = 0; i < 180; i++)
	{
		geoVertex.position = Vector3(0, 0, 0);
		float size = Mathf::RandF(3.0f, 9.0f);
		geoVertex.size = Vector2(size, size);
		geoVertex.nWidth = textureWidth;
		geoVertex.nHeight = textureHeight;
		geoVertex.randHeight = Mathf::RandF(100.f, 200.f);
		geoVertex.randDir = i * 2;
		vecGeoVertex.push_back(geoVertex);
	}
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_EFFECT_INFO) + 255) & ~255); //256의 배수

	CMonsterAttackShader* pEffectShader = new CMonsterAttackShader();
	pEffectShader->SetPso(shader->GetPso());
	pEffectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pEffectShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	pEffectShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	pEffectShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(pEffectShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pEffectShader);
}

EffectBossAttack::~EffectBossAttack()
{
}

void EffectBossAttack::SetCameraPos(Vector3 pos)
{
	pos.y -= -160;
	lookPosition = pos;
}

void EffectBossAttack::Update(float fTimeElapsed, CCamera* camera)
{
	if (!isAlive)
	{
		liveTime = 0;
		return;
	}
	frameCount = 0;
	if (frameCount >= timeCount)
		frameCount = 0;
	if (liveTime > 200)
	{
		isAlive = false;
	}
	liveTime += 5.f;
	//lookPosition = camera->GetPosition();
}
