//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "EffectUI.h"
#include "Item.h"
#include "Sound.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = Vector3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = Vector3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (gsCamera) delete gsCamera;
	if (playerHpUi) delete playerHpUi;
	if (playerMagazineUi)  delete playerMagazineUi;
	if (playerInventory)  delete playerInventory;
	if (activeItemUI)  delete activeItemUI;
	if (activeItem)  delete activeItem;
	if (youdiedTexture) delete youdiedTexture;
};

void CPlayer::SetPosition(const Vector3& xmf3Position)
{
	Vector3 deltaPosition(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z);
	Move(deltaPosition, false);
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (gsCamera) gsCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbPlayer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbPlayer->Map(0, NULL, (void **)&m_pcbMappedPlayer);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pd3dcbPlayer)
	{
		m_pd3dcbPlayer->Unmap(0, NULL);
		m_pd3dcbPlayer->Release();
	}
	if (gsCamera) gsCamera->ReleaseShaderVariables();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(PLAYER_CBV, d3dGpuVirtualAddress);
}


// 플레이어 움직임으로 추정되는 곳
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		Vector3 xmf3Shift = Vector3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = xmf3Shift + (m_xmf3Look * fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = xmf3Shift + (m_xmf3Look * -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = xmf3Shift + (m_xmf3Right * fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = xmf3Shift + (m_xmf3Right * -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = xmf3Shift + (m_xmf3Up * fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = xmf3Shift + (m_xmf3Up * -fDistance);

		OutputDebugString(L"메시지\n");

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const Vector3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity += xmf3Shift;
	}
	else
	{
		m_xmf3Position += xmf3Shift;
		gsCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = gsCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		gsCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			Matrix4x4 matRotate; XMStoreFloat4x4(&matRotate, xmmtxRotate);
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		gsCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			Matrix4x4 matRotate; XMStoreFloat4x4(&matRotate, xmmtxRotate);
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			Matrix4x4 matRotate; XMStoreFloat4x4(&matRotate, xmmtxRotate);
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, matRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			Matrix4x4 matRotate; XMStoreFloat4x4(&matRotate, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, matRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, matRotate);
		}
	}

	m_xmf3Look = m_xmf3Look.normalized();
	m_xmf3Right = Vector3::CrossNormal(m_xmf3Up, m_xmf3Look);
	m_xmf3Up = Vector3::CrossNormal(m_xmf3Look, m_xmf3Right);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = m_xmf3Velocity + (m_xmf3Gravity.normalized() * fTimeElapsed);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	Move(m_xmf3Velocity, false);
	if (isAttacked)
	{
		Rotate(Mathf::RandF(-1.f, 1.f), Mathf::RandF(-1.f, 1.f), 0);
	}

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = gsCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) gsCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	gsCamera->SetLookAt(m_xmf3Position);
	gsCamera->RegenerateViewMatrix();

	fLength = m_xmf3Velocity.length();
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = m_xmf3Velocity + (m_xmf3Velocity.normalized() * -fDeceleration);

	playerMagazineUi->SetBullet(numBullet);

	playerHpUi->Update(fTimeElapsed, gsCamera);
	playerMagazineUi->Update(fTimeElapsed, gsCamera);

	rootObject->Update();
	if (chrono::system_clock::now() - attackTime > chrono::milliseconds(500) && isAttacked)
	{
		isAttacked = false;
	}
	CheckReload();
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(gsCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(gsCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(gsCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3(m_xmf3Right.x, 0.0f, m_xmf3Right.z).normalized();
		m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f).normalized();
		m_xmf3Look = Vector3(m_xmf3Look.x, 0.0f, m_xmf3Look.z).normalized();

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(Vector3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && gsCamera)
	{
		m_xmf3Right = gsCamera->GetRightVector();
		m_xmf3Up = gsCamera->GetUpVector();
		m_xmf3Look = gsCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (gsCamera) delete gsCamera;

	return(pNewCamera);
}

CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (gsCamera) ? gsCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(gsCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(200.0f);
		SetGravity(Vector3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		gsCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		gsCamera->SetTimeLag(0.0f);
		gsCamera->SetOffset(Vector3(0.0f, 20.0f, 0.0f));
		gsCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		gsCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		gsCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	//Update(fTimeElapsed);

	return(gsCamera);
}

void CPlayer::OnPrepareRender()
{
	worldMatrix._11 = m_xmf3Right.x; worldMatrix._12 = m_xmf3Right.y; worldMatrix._13 = m_xmf3Right.z;
	worldMatrix._21 = m_xmf3Up.x; worldMatrix._22 = m_xmf3Up.y; worldMatrix._23 = m_xmf3Up.z;
	worldMatrix._31 = m_xmf3Look.x; worldMatrix._32 = m_xmf3Look.y; worldMatrix._33 = m_xmf3Look.z;
	worldMatrix._41 = m_xmf3Position.x; worldMatrix._42 = m_xmf3Position.y; worldMatrix._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::SetHp(int hp, int maxhp)
{
	playerHpUi->SetHp(hp, maxhp);
	if (playerHp != hp && hp != maxhp && chrono::system_clock::now() - attackTime > chrono::milliseconds(500))
	{
		isAttacked = true;
		attackTime = chrono::system_clock::now();
		objectSound->PlayShoot(SOUND_TYPE::HIT_SOUND, 0.5);
	}
	playerHp = hp;
	if (playerHp == 0)
	{
		drawYoudied = true;
		drawGrayScale = true;
		playerInventory->InitItem();
		activeItem->SetIsAlive(false);
	}
	else
	{
		drawYoudied = false;
		drawGrayScale = false;
	}
}

bool CPlayer::CheckReload()
{
	if (rootObject->currentAniNum == rootObject->animationManual["reload"])
	{
		if (rootObject->m_pModelAnimator->aniOncePlay)
		{
			rootObject->AnimationChange(8);
		}
		else
			return true;
	}
	return false;
}

void CPlayer::DrawInventory(bool isDraw)
{
	if (playerHp <= 0) drawGrayScale = false;
	else drawGrayScale = isDraw;
	drawInventory = isDraw;
}

void CPlayer::SetItem(ITEM_TYPE item)
{
	if (item == ITEM_TYPE::ITEM_MAXHPUP || item == ITEM_TYPE::ITEM_MONSTER_SLOW)
	{
		activeItem->SetIsAlive(true);
		activeItem->SetFrameCount(item);
	}
	else
	{
		playerInventory->SetItem(item);
	}
}

void CPlayer::SetActiveItemAlive(bool alive)
{
	activeItem->SetIsAlive(alive);
}

void CPlayer::SetPositionBase(const Vector3& xmf3Position)
{
	Vector3 deltaPosition(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z);
	Move(deltaPosition, false);
}

void CPlayer::CameraZoomInOut()
{
	zoomEnable = !zoomEnable;
	if (zoomEnable)
	{
		isAlive = false;
		gsCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 30.0f);
	}
	else
	{
		isAlive = true;
		gsCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	}
}


CRiflePlayer::CRiflePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, int nMeshes, CShader * shader, CShader* uiShader) : CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	isAlive = true;
	gsCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/carbineColor.dds";
	textureinfo.normal = L"Image/carbineNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("carbine", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("lens", textureinfo));
	textureinfo.diffuse = L"Image/armColor.dds";
	textureinfo.normal = L"Image/armNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("arms", textureinfo));
	CShader* skinShader = new CSkinnedAnimationShader;
	skinShader->SetPso(shader->GetPso());
	skinShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 13, 26);
	std::string error;
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene;
	scene = importer.ReadFile("Models/arms@carbine.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);
	/*scene = importer.ReadFile("Models/arms@shotgun.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);*/
	
	if (!scene) {
		error = (importer.GetErrorString());
	}
	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, skinShader,scene, "arms@carbine");

	asAnimationTrack* track = new asAnimationTrack;

	// 시간에서 *20을 해줘야 저장된 클립의 시간으로 변환된다.
	// 예) 대기모션 = 0.3초 => 0.3 * 20 = 6.0f = 클립에 저장된 부분
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->timeEndPosition = 0.5f;
	track->animationSpeed = 1.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->timeStartPosition = 68.0f;
	track->timeEndPosition = 66.0f;
	track->animationSpeed = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.insert(std::pair<std::string, int>("playeridle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("reload", 1));
	objectType = OBJECT_TYPE::FPS_PLAYER;

	objectStates.push_back(OBJECT_STATE::IDLE);
	objectStates.push_back(OBJECT_STATE::ATTACK); 
	objectStates.push_back(OBJECT_STATE::REROAD);

	maxBullet = 30; numBullet = 30;
	attackSpeed = 0.15;

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	rootObject->SetScale(0.2f, 0.2f, 0.2f);
	SetPositionBase(Vector3(pTerrain->GetWidth() * 0.7f, 200.0f, pTerrain->GetLength() * 0.5f));
	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	playerHpUi = new UIPlayerHp(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	playerMagazineUi = new UIMagazine(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, maxBullet, uiShader, WEAPON_RIFLE);
	playerInventory = new Inventory(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	activeItemUI = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/active.dds", Vector2(0.6, -0.85), Vector2(0.135f, 0.24f), Vector2(1, 1), uiShader);
	activeItem = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/item.dds", Vector2(0.6, -0.85), Vector2(0.09f, 0.16f), Vector2(3, 3), uiShader);
	activeItem->SetIsAlive(false);
	youdiedTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/youdied.dds", Vector2(0, 0), Vector2(1.8f, 1.8f), Vector2(1, 1), uiShader);
	attackTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/BloodScreen.dds", Vector2(0, 0), Vector2(2.0f, 2.0f), Vector2(1, 1), uiShader);
	objectSound = snd;
}

CRiflePlayer::~CRiflePlayer()
{
	rootObject->~CFbxModelObject();
}

void CRiflePlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	Matrix4x4 world = Matrix4x4::identity;
	
	world._11 = gsCamera->GetRightVector().x; world._12 = gsCamera->GetRightVector().y; world._13 = gsCamera->GetRightVector().z;
	world._21 = gsCamera->GetUpVector().x; world._22 = gsCamera->GetUpVector().y; world._23 = gsCamera->GetUpVector().z;
	world._31 = gsCamera->GetLookVector().x; world._32 = gsCamera->GetLookVector().y; world._33 = gsCamera->GetLookVector().z;
	world._41 = gsCamera->GetPosition().x - (world._21 * 25) + (world._31 * 15) + (world._11 * 10);
	world._42 = gsCamera->GetPosition().y - (world._22 * 25) + (world._32 * 15) + (world._12 * 10);
	world._43 = gsCamera->GetPosition().z - (world._23 * 25) + (world._33 * 15) + (world._13 * 10);
	rootObject->SetWorld(world);
	rootObject->SetScaleInverse(-1.0f, 1.0f, -1.0f);
	if(isAlive)
		rootObject->Render(pd3dCommandList, pCamera);

	if (renderType == RENDER_TYPE::IDLE_RENDER)
	{
		if (drawInventory)
			playerInventory->Render(pd3dCommandList, pCamera);
		else if(drawYoudied)
			youdiedTexture->Render(pd3dCommandList, pCamera);
		else
		{
			playerHpUi->Render(pd3dCommandList, pCamera);
			playerMagazineUi->Render(pd3dCommandList, pCamera);
			activeItemUI->Render(pd3dCommandList, pCamera);
			activeItem->Render(pd3dCommandList, pCamera);
			if(isAttacked) attackTexture->Render(pd3dCommandList, pCamera);
		}
	}
}

CShotgunPlayer::CShotgunPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, int nMeshes, CShader* shader, CShader* uiShader) : CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	isAlive = true;
	gsCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/shotgunColor.dds";
	textureinfo.normal = L"Image/shotgunNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("shotgun", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("lens", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("Mat", textureinfo));
	textureinfo.diffuse = L"Image/armColor.dds";
	textureinfo.normal = L"Image/armNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("arms", textureinfo));
	CShader* skinShader = new CSkinnedAnimationShader;
	skinShader->SetPso(shader->GetPso());
	skinShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 8, 16);
	std::string error;
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene;
	scene = importer.ReadFile("Models/arms@shotgun.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);

	if (!scene) {
		error = (importer.GetErrorString());
	}
	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, skinShader, scene, "arms@shotgun");

	asAnimationTrack* track = new asAnimationTrack;

	// 시간에서 *20을 해줘야 저장된 클립의 시간으로 변환된다.
	// 예) 대기모션 = 0.3초 => 0.3 * 20 = 6.0f = 클립에 저장된 부분
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->timeEndPosition = 0.5f;
	track->animationSpeed = 1.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->timeStartPosition = 35.0f;
	track->timeEndPosition = 93.0f;
	track->animationSpeed = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.insert(std::pair<std::string, int>("playeridle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("reload", 1));
	objectType = OBJECT_TYPE::FPS_PLAYER;

	objectStates.push_back(OBJECT_STATE::IDLE);
	objectStates.push_back(OBJECT_STATE::ATTACK);
	objectStates.push_back(OBJECT_STATE::REROAD);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	rootObject->SetScale(0.2f, 0.2f, 0.2f);
	SetPositionBase(Vector3(pTerrain->GetWidth() * 0.7f, 200.0f, pTerrain->GetLength() * 0.5f));
	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	maxBullet = 7; numBullet = 7;
	attackSpeed = 0.5f;

	playerHpUi = new UIPlayerHp(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	playerMagazineUi = new UIMagazine(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, maxBullet, uiShader,WEAPON_SHOTGUN);
	playerInventory = new Inventory(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	activeItemUI = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/active.dds", Vector2(0.6, -0.85), Vector2(0.135f, 0.24f), Vector2(1, 1), uiShader);
	activeItem = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/item.dds", Vector2(0.6, -0.85), Vector2(0.09f, 0.16f), Vector2(3, 3), uiShader);
	activeItem->SetIsAlive(false);
	youdiedTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/youdied.dds", Vector2(0, 0), Vector2(1.8f, 1.8f), Vector2(1, 1), uiShader);
	attackTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/BloodScreen.dds", Vector2(0, 0), Vector2(2.0f, 2.0f), Vector2(1, 1), uiShader);
	objectSound = snd;

}

CShotgunPlayer::~CShotgunPlayer()
{
	rootObject->~CFbxModelObject();
}

void CShotgunPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	Matrix4x4 world = Matrix4x4::identity;

	world._11 = gsCamera->GetRightVector().x; world._12 = gsCamera->GetRightVector().y; world._13 = gsCamera->GetRightVector().z;
	world._21 = gsCamera->GetUpVector().x; world._22 = gsCamera->GetUpVector().y; world._23 = gsCamera->GetUpVector().z;
	world._31 = gsCamera->GetLookVector().x; world._32 = gsCamera->GetLookVector().y; world._33 = gsCamera->GetLookVector().z;
	world._41 = gsCamera->GetPosition().x - (world._21 * 25) + (world._31 * 12) + (world._11 * 13);
	world._42 = gsCamera->GetPosition().y - (world._22 * 25) + (world._32 * 12) + (world._12 * 13);
	world._43 = gsCamera->GetPosition().z - (world._23 * 25) + (world._33 * 12) + (world._13 * 13);
	rootObject->SetWorld(world);
	rootObject->SetScaleInverse(-1.0f, 1.0f, -1.0f);
	if (isAlive)
		rootObject->Render(pd3dCommandList, pCamera);

	if (renderType == RENDER_TYPE::IDLE_RENDER)
	{
		if (drawInventory)
			playerInventory->Render(pd3dCommandList, pCamera);
		else if (drawYoudied)
			youdiedTexture->Render(pd3dCommandList, pCamera);
		else
		{
			playerHpUi->Render(pd3dCommandList, pCamera);
			playerMagazineUi->Render(pd3dCommandList, pCamera);
			activeItemUI->Render(pd3dCommandList, pCamera);
			activeItem->Render(pd3dCommandList, pCamera);
			if (isAttacked) attackTexture->Render(pd3dCommandList, pCamera);
		}
	}
}

CGrenadelauncherPlayer::CGrenadelauncherPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, int nMeshes, CShader* shader, CShader* uiShader) : CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	isAlive = true;
	gsCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/grenadelauncherColor.dds";
	textureinfo.normal = L"Image/grenadelauncherNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("glauncher", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("lens", textureinfo));
	textureinfo.diffuse = L"Image/armColor.dds";
	textureinfo.normal = L"Image/armNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("arms", textureinfo));
	CShader* skinShader = new CSkinnedAnimationShader;
	skinShader->SetPso(shader->GetPso());
	skinShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 13, 26);
	std::string error;
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene;
	scene = importer.ReadFile("Models/arms@Glauncher.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);
	/*scene = importer.ReadFile("Models/arms@shotgun.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);*/

	if (!scene) {
		error = (importer.GetErrorString());
	}
	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, skinShader, scene, "arms@Glauncher");

	asAnimationTrack* track = new asAnimationTrack;

	// 시간에서 *20을 해줘야 저장된 클립의 시간으로 변환된다.
	// 예) 대기모션 = 0.3초 => 0.3 * 20 = 6.0f = 클립에 저장된 부분
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->timeEndPosition = 0.5f;
	track->animationSpeed = 1.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->timeStartPosition = 8.0f;
	track->timeEndPosition = 205.5f;
	track->animationSpeed = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.insert(std::pair<std::string, int>("playeridle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("reload", 1));
	objectType = OBJECT_TYPE::FPS_PLAYER;

	objectStates.push_back(OBJECT_STATE::IDLE);
	objectStates.push_back(OBJECT_STATE::ATTACK);
	objectStates.push_back(OBJECT_STATE::REROAD);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	rootObject->SetScale(0.2f, 0.2f, 0.2f);
	SetPositionBase(Vector3(pTerrain->GetWidth() * 0.7f, 200.0f, pTerrain->GetLength() * 0.5f));
	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	maxBullet = 30; numBullet = 30;
	attackSpeed = 0.15;

	playerHpUi = new UIPlayerHp(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	playerMagazineUi = new UIMagazine(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, maxBullet, uiShader,WEAPON_SHOTGUN);
	playerInventory = new Inventory(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	activeItemUI = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/active.dds", Vector2(0.6, -0.85), Vector2(0.135f, 0.24f), Vector2(1, 1), uiShader);
	activeItem = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/item.dds", Vector2(0.6, -0.85), Vector2(0.09f, 0.16f), Vector2(3, 3), uiShader);
	activeItem->SetIsAlive(false);
	youdiedTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/youdied.dds", Vector2(0, 0), Vector2(1.8f, 1.8f), Vector2(1, 1), uiShader);
	objectSound = snd;
}

CGrenadelauncherPlayer::~CGrenadelauncherPlayer()
{
	rootObject->~CFbxModelObject();
}

void CGrenadelauncherPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	Matrix4x4 world = Matrix4x4::identity;

	world._11 = gsCamera->GetRightVector().x; world._12 = gsCamera->GetRightVector().y; world._13 = gsCamera->GetRightVector().z;
	world._21 = gsCamera->GetUpVector().x; world._22 = gsCamera->GetUpVector().y; world._23 = gsCamera->GetUpVector().z;
	world._31 = gsCamera->GetLookVector().x; world._32 = gsCamera->GetLookVector().y; world._33 = gsCamera->GetLookVector().z;
	world._41 = gsCamera->GetPosition().x - (world._21 * 25) + (world._31 * 15) + (world._11 * 10);
	world._42 = gsCamera->GetPosition().y - (world._22 * 25) + (world._32 * 15) + (world._12 * 10);
	world._43 = gsCamera->GetPosition().z - (world._23 * 25) + (world._33 * 15) + (world._13 * 10);
	rootObject->SetWorld(world);
	rootObject->SetScaleInverse(-1.0f, 1.0f, -1.0f);
	rootObject->Render(pd3dCommandList, pCamera);

	if (renderType == RENDER_TYPE::IDLE_RENDER)
	{
		if (drawInventory)
			playerInventory->Render(pd3dCommandList, pCamera);
		else if (drawYoudied)
			youdiedTexture->Render(pd3dCommandList, pCamera);
		else
		{
			playerHpUi->Render(pd3dCommandList, pCamera);
			playerMagazineUi->Render(pd3dCommandList, pCamera);
			activeItemUI->Render(pd3dCommandList, pCamera);
			activeItem->Render(pd3dCommandList, pCamera);
		}
	}
}

CSniperPlayer::CSniperPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, int nMeshes, CShader* shader, CShader* uiShader) : CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	isAlive = true;
	gsCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/sniper1Color.dds";
	textureinfo.normal = L"Image/sniper1Normal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("sniper", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("lens", textureinfo));
	textureinfo.diffuse = L"Image/armColor.dds";
	textureinfo.normal = L"Image/armNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("arms", textureinfo));
	CShader* skinShader = new CSkinnedAnimationShader;
	skinShader->SetPso(shader->GetPso());
	skinShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 13, 26);
	std::string error;
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene;
	scene = importer.ReadFile("Models/arms@sniper.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded
	);

	if (!scene) {
		error = (importer.GetErrorString());
	}
	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, skinShader, scene, "arms@sniper");

	asAnimationTrack* track = new asAnimationTrack;

	// 시간에서 *20을 해줘야 저장된 클립의 시간으로 변환된다.
	// 예) 대기모션 = 0.3초 => 0.3 * 20 = 6.0f = 클립에 저장된 부분
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->timeEndPosition = 0.5f;
	track->animationSpeed = 1.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->timeStartPosition = 40.0f;
	track->timeEndPosition = 110.0f;
	track->animationSpeed = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->timeEndPosition = 30.0f;
	track->animationSpeed = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.insert(std::pair<std::string, int>("playeridle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("reload", 1));
	rootObject->animationManual.insert(std::pair<std::string, int>("attack", 2));
	objectType = OBJECT_TYPE::FPS_PLAYER;

	objectStates.push_back(OBJECT_STATE::IDLE);
	objectStates.push_back(OBJECT_STATE::ATTACK);
	objectStates.push_back(OBJECT_STATE::REROAD);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	rootObject->SetScale(0.2f, 0.2f, 0.2f);
	SetPositionBase(Vector3(pTerrain->GetWidth() * 0.7f, 200.0f, pTerrain->GetLength() * 0.5f));
	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	maxBullet = 5; numBullet = 5;
	attackSpeed = 1.0f;

	playerHpUi = new UIPlayerHp(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	playerMagazineUi = new UIMagazine(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, maxBullet, uiShader,WEAPON_SNIPER);
	playerInventory = new Inventory(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, uiShader);
	activeItemUI = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/active.dds", Vector2(0.6, -0.85), Vector2(0.135f, 0.24f), Vector2(1, 1), uiShader);
	activeItem = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/item.dds", Vector2(0.6, -0.85), Vector2(0.09f, 0.16f), Vector2(3, 3), uiShader);
	activeItem->SetIsAlive(false);
	youdiedTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/youdied.dds", Vector2(0, 0), Vector2(1.8f, 1.8f), Vector2(1, 1), uiShader);
	sniperZoom =  new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/sniper_scope_2.dds", Vector2(0, 0), Vector2(2.0f, 2.0f), Vector2(1, 1), uiShader);
	attackTexture = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/BloodScreen.dds", Vector2(0, 0), Vector2(2.0f, 2.0f), Vector2(1, 1), uiShader);
	objectSound = snd;
}

CSniperPlayer::~CSniperPlayer()
{
	rootObject->~CFbxModelObject();
}

void CSniperPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	Matrix4x4 world = Matrix4x4::identity;

	world._11 = gsCamera->GetRightVector().x; world._12 = gsCamera->GetRightVector().y; world._13 = gsCamera->GetRightVector().z;
	world._21 = gsCamera->GetUpVector().x; world._22 = gsCamera->GetUpVector().y; world._23 = gsCamera->GetUpVector().z;
	world._31 = gsCamera->GetLookVector().x; world._32 = gsCamera->GetLookVector().y; world._33 = gsCamera->GetLookVector().z;
	world._41 = gsCamera->GetPosition().x - (world._21 * 25) + (world._31 * 12) + (world._11 * 15);
	world._42 = gsCamera->GetPosition().y - (world._22 * 25) + (world._32 * 12) + (world._12 * 15);
	world._43 = gsCamera->GetPosition().z - (world._23 * 25) + (world._33 * 12) + (world._13 * 15);
	rootObject->SetWorld(world);
	rootObject->SetScaleInverse(-1.0f, 1.0f, -1.0f);
	if (isAlive)
		rootObject->Render(pd3dCommandList, pCamera);

	if (renderType == RENDER_TYPE::IDLE_RENDER)
	{
		if (drawInventory)
			playerInventory->Render(pd3dCommandList, pCamera);
		else if (drawYoudied)
			youdiedTexture->Render(pd3dCommandList, pCamera);
		else
		{
			playerHpUi->Render(pd3dCommandList, pCamera);
			playerMagazineUi->Render(pd3dCommandList, pCamera);
			activeItemUI->Render(pd3dCommandList, pCamera);
			activeItem->Render(pd3dCommandList, pCamera);
			if(zoomEnable) sniperZoom->Render(pd3dCommandList, pCamera);
			if (isAttacked) attackTexture->Render(pd3dCommandList, pCamera);
		}
	}
}