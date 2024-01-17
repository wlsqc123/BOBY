#include "stdafx.h"
#include "Monster.h"
#include "Shader.h"
#include "EffectUI.h"
#include "ServerMgr.h"
#include "Sound.h"
#include "Item.h"

//#define DRAW_BB

CMovableObject::CMovableObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);
}

CMovableObject::~CMovableObject()
{
}



void CMovableObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}


CMovableHierarchyObject::CMovableHierarchyObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext) :CMovableObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, 1)
{
}

CMovableHierarchyObject::~CMovableHierarchyObject()
{
	rootObject->~CFbxModelObject();
}

void CMovableHierarchyObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!isAlive) return;
	if (!IsInFrustum(pCamera)) return;
	if (renderType == RENDER_TYPE::IDLE_RENDER)
	{
		if (objectHpBar)
		{
			objectHpBar->Render(pd3dCommandList, pCamera);
		}
		if (isRenderBoundingBox)
		{
			CGameObject::Render(pd3dCommandList, pCamera);
		}
	}
	rootObject->Render(pd3dCommandList, pCamera);
}

void CMovableHierarchyObject::SetPosition(const Vector3& xmf3Position)
{
	rootObject->SetPosition(xmf3Position);
}

void CMovableHierarchyObject::SetScale(float x, float y, float z)
{
	rootObject->SetScale(x,y,z);
}

void CMovableHierarchyObject::SetWorld(Matrix4x4 world)
{
	rootObject->SetWorld(world);
}

void CMovableHierarchyObject::Rotate(Vector3* pxmf3Axis, float fAngle)
{
	rootObject->Rotate(pxmf3Axis,fAngle);
}

void CMovableHierarchyObject::Move(Vector3 pos)
{
	m_xmf3Position = pos;
	pos.y -= objectZPosition;
	rootObject->Move(pos);
}

void CMovableHierarchyObject::SetServerData(Vector3 pos, Vector3 look, Vector3 lightLook)
{
	Move(pos);
	SetLook(look, lightLook);
}

void CMovableHierarchyObject::SetLook(Vector3 look, Vector3 lightLook)
{
	m_xmf3Look = lightLook;
	m_xmf3Up = Vector3(0, 1, 0);
	m_xmf3Right = Vector3::CrossNormal(m_xmf3Look, m_xmf3Up);
	rootObject->SetDirectionWithLookVector(look);
}


void CMovableHierarchyObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	Matrix4x4 bbWorld = rootObject->GetWorldMatrix();
	bbWorld._42 += boundingBoxPosition;
	XMStoreFloat4x4(&cbMappedGameObject->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&bbWorld)));
	if (objectMaterial) cbMappedGameObject->m_nMaterial = 1;
}

void CMovableHierarchyObject::ReleaseShaderVariables()
{
	CMovableObject::ReleaseShaderVariables();
	rootObject->ReleaseShaderVariables();
}

void CMovableHierarchyObject::ReleaseUploadBuffers()
{
	CMovableObject::ReleaseUploadBuffers();
	rootObject->ReleaseUploadBuffers();
}

void CMovableHierarchyObject::Update(float fTimeElapsed, CCamera* camera)
{

	Matrix4x4 bbWorld = rootObject->GetWorldMatrix();
	bbWorld._42 += boundingBoxPosition;
	XMMATRIX xmmWorld = XMLoadFloat4x4(&bbWorld);
	objectBoundingBox = baseBoundingBox;
	objectBoundingBox.Transform(objectBoundingBox, xmmWorld);

	if (!IsInFrustum(camera)) return;

	if (objectHpBar)
	{
		objectHpBar->SetPosition(bbWorld._41, bbWorld._42 + boundingBoxPosition + 20, bbWorld._43);
	}
	rootObject->Update();
}

void CMovableHierarchyObject::CreateBoundingBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CBoundingBoxMesh* bbMesh = new CBoundingBoxMesh(pd3dDevice, pd3dCommandList, baseBoundingBox.Extents.x, baseBoundingBox.Extents.y, baseBoundingBox.Extents.z);
	SetMesh(0, bbMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256�� ���
	CBoundingBoxShader* bbShader = new CBoundingBoxShader();
	bbShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	bbShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	bbShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 0);
	bbShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	SetCbvGPUDescriptorHandle(bbShader->GetGPUCbvDescriptorStartHandle());
	SetShader(bbShader);
}

void CMovableHierarchyObject::SetHp(UINT hp)
{
	if (objectHpBar)
	{
		if (hp > 0)
		{
			isAlive = true;
			Dying = false;
		}
		objectHp = hp;
		float hpPer = static_cast<float>(objectHp) / static_cast<float>(objectMaxHp);
		UINT hpPerInt = static_cast<UINT>(hpPer * 100);
		objectHpBar->SetHp(hpPerInt);

		if (hp == 0 && !Dying) {
			GetFbxObject()->AnimationChange(4);
			Die();		Dying = true;
			//isAlive = false;
		}
		if (GetFbxObject()->m_pModelAnimator->aniOncePlay)
		{
			isAlive = false;
		}
	}
}

void CMovableHierarchyObject::SetObjectState()
{
	switch (objectType)
	{
	case OBJECT_TYPE::MONSTER:
		objectStates.push_back(OBJECT_STATE::IDLE);
		objectStates.push_back(OBJECT_STATE::MOVE_FORWARD);
		objectStates.push_back(OBJECT_STATE::ATTACK);
		objectStates.push_back(OBJECT_STATE::DIE);
		break;
	case OBJECT_TYPE::TPS_PLAYER:
		objectStates.push_back(OBJECT_STATE::IDLE);
		objectStates.push_back(OBJECT_STATE::MOVE_FORWARD);
		objectStates.push_back(OBJECT_STATE::MOVE_SIDE);
		objectStates.push_back(OBJECT_STATE::ATTACK);
		objectStates.push_back(OBJECT_STATE::DIE);
		break;
	case OBJECT_TYPE::INTERACTION:
		objectStates.push_back(OBJECT_STATE::IDLE);
		objectStates.push_back(OBJECT_STATE::INTERACTION);
		break;
	case OBJECT_TYPE::STATIC:
		objectStates.push_back(OBJECT_STATE::IDLE);
		break;
	}
}


CMagmaMonster::CMagmaMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader):CMovableHierarchyObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/magma.dds";
	textureinfo.normal = L"Image/magmaNormal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("material", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 2);
 	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures,shader, scene, true, "magma");
	
	asAnimationTrack* track = new asAnimationTrack;

	//idle
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = true;
	track->clipNumber = 5;
	track->animationSpeed = 10.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[5]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	//attack(speat)
	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->trackEnable = false;
	track->animationSpeed = 10.f;
	track->clipNumber = 0;
	//track->timeStartPosition = 8.0f;
	//track->timeEndPosition = 1.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[0]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	//die
	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->animationSpeed = 10.0f;
	track->clipNumber = 3;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[3]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	//attack(puhch)
	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->animationSpeed = 10.0f;
	track->clipNumber = 9;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[9]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->currentAniTime = std::chrono::system_clock::now();
	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("idle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("attack", 1));
	rootObject->animationManual.insert(std::pair<std::string, int>("death", 2));
	rootObject->animationManual.insert(std::pair<std::string, int>("punch", 3));
	rootObject->currentAniNum = rootObject->animationManual["idle"];
	

	objectMaxHp = 300;
	objectHp = 300;

	objectHpBar = new UIHpBar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	objectHpBar->SetHp(objectHp);
	objectZPosition = 80.f;
	boundingBoxPosition = 70.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(1.5, 1.8, 1.2), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB
	Vector3 xmf3rotate = Vector3(0,1, 0);
	Rotate(&xmf3rotate, -90);
	SetScale(30, 30, 30);

	objectType = OBJECT_TYPE::MONSTER;
	objectSound = snd;
	SetObjectState();
}

CMagmaMonster::~CMagmaMonster()
{
}

void CMagmaMonster::Die()
{
	objectSound->PlayMonster(SOUND_TYPE::MAGMA_DEATH);
}

COGREMonster::COGREMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader):CMovableHierarchyObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/Rock_Monster_Texture_2.dds";
	textureinfo.normal = L"Image/Material__33_Normal_OpenGL.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("Material_33", textureinfo));
	CShader* shader = new CMonsterShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 5, 10);

	objectMaxHp = 250;
	objectHp = 250;

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, shader, scene, true, "SandGolem");

	objectHpBar = new UIHpBar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	objectHpBar->SetHp(objectHp);
	objectZPosition = 80.f;
	boundingBoxPosition = 100.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(900, 1300, 500), Vector4(0, 0, 0, 1));

	Assimp::Importer importer;
	std::string error;
	const aiScene* aniscene;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	aniscene = importer.ReadFile("Models/RockGolem_attack_3ds.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_FlipWindingOrder);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	rootObject->currentAniNum = 2;

	rootObject->currentAniTime = std::chrono::system_clock::now();

	asAnimationTrack* track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = false;
	track->clipNumber = 15;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[15]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = false;
	track->animationWeight = 1.0f;
	track->clipNumber = 2;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[2]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->animationWeight = 1.0f;
	track->trackEnable = false;
	track->clipNumber = 4;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[4]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->animationWeight = 1.0f;
	track->trackEnable = true;
	track->clipNumber = 8;
	track->animationSpeed = 30.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[8]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("attack", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("death", 1));
	rootObject->animationManual.insert(std::pair<std::string, int>("idle", 2));
	rootObject->animationManual.insert(std::pair<std::string, int>("walk", 3));

	rootObject->SetSsaoMaterial(2);
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	
	SetScale(0.05f, 0.05f, 0.05f);
	Vector3 xmf3rotate = Vector3(0, 1, 0);
	Rotate(&xmf3rotate, 270.f);

	objectType = OBJECT_TYPE::MONSTER;
	objectSound = snd;
	SetObjectState();
}

COGREMonster::~COGREMonster()
{
}

void COGREMonster::SetLook(Vector3 look, Vector3 lightLook) 
{
	m_xmf3Look = lightLook;
	rootObject->SetDirectionWithLookVector(look);
}

void COGREMonster::Die()
{
	objectSound->PlayMonster(SOUND_TYPE::MAGMA_DEATH);
}

CTestPlayerModel::CTestPlayerModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader):CMovableHierarchyObject (pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	//�ٲܸ�
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/Body2-lossless.dds";
	textureinfo.normal = L"Image/soldier_Normal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("lambert2", textureinfo));
	textures.insert(std::pair<string, TEXTURE_INFO>("polySurface1SG1", textureinfo));
	textureinfo.diffuse = L"Image/Head2-lossless.dds";
	textureinfo.normal = L"Image/Head_N-lossless.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("polySurface1SG", textureinfo));
	CShader* shader = new CSkinnedAnimationShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 3, 6);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Soldier");
	objectZPosition = 100.f;
	boundingBoxPosition = 80.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(10, 30, 30),Vector4(0,0,0,1));

	Assimp::Importer importer;
	std::string error;
	const aiScene* aniscene;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	aniscene = importer.ReadFile("Models/Animation/Solder/Solder_Running_max.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	aniscene = importer.ReadFile("Models/Animation/Solder/Solder_idle.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	rootObject->currentAniNum = 1;
	rootObject->currentAniTime = std::chrono::system_clock::now();

	asAnimationTrack* track = new asAnimationTrack;

	//idle
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->animationSpeed = 20.0f;
	track->trackEnable = true;
	track->clipNumber = 2;
	track->timeEndPosition = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	//forwordRun
	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->trackEnable = false;
	track->animationSpeed = 20.0f;
	track->animationWeight = 1.f;
	track->clipNumber = 1;
	track->timeEndPosition = 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("idle", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("walk", 1));

#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(2.0f, 2.0f, 2.0f);	
	Vector3 xmf3rotate = Vector3(0, 1, 0);
	Rotate(&xmf3rotate, 90);
	
	objectType = OBJECT_TYPE::TPS_PLAYER;
	objectSound = snd;
	SetObjectState();
}

CTestPlayerModel::~CTestPlayerModel()
{
}

void CTestPlayerModel::SetLook(Vector3 look, Vector3 lightLook)
{
	m_xmf3Look = lightLook;
	rootObject->SetDirectionWithPlayerLookVector(look);
}

CGolemModel::CGolemModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader) :CMovableHierarchyObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/diffuse_fire_elemntal.dds";
	textureinfo.normal = L"Image/normal_fire_elemental.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("fireelemental", textureinfo));
	CShader* shader = new CMonsterShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 5, 10);

	objectMaxHp = 3500;
	objectHp = 3500;

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, true, textures, shader, scene, true, "FireGolem");

	objectHpBar = new UIHpBar(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	objectHpBar->SetHp(objectHp);
	objectZPosition = 80.f;
	boundingBoxPosition = 180.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(1000, 1300, 500), Vector4(0, 0, 0, 1));
	//CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	Assimp::Importer importer;
	std::string error;
	const aiScene* aniscene;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	aniscene = importer.ReadFile("Models/Animation/FireGolem/fireGolem_attack_3ds.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	aniscene = importer.ReadFile("Models/Animation/FireGolem/fireGolem_idle_3ds.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	aniscene = importer.ReadFile("Models/Animation/FireGolem/fireGolem_die_3ds.fbx", aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (!aniscene) { error = (importer.GetErrorString()); }
	rootObject->m_pModelAnimator->SetAnimationClip(aniscene);

	rootObject->currentAniNum = 2;

	rootObject->currentAniTime = std::chrono::system_clock::now();

	asAnimationTrack* track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_PINGPONG;
	track->trackEnable = false;
	track->clipNumber = 0;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[0]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = false;
	track->animationWeight = 1.0f;
	track->clipNumber = 1;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[1]->Duration- 20.0f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->animationWeight = 1.0f;
	track->trackEnable = false;
	track->clipNumber = 3;
	track->animationSpeed = 50.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[3]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_LOOP;
	track->animationWeight = 1.0f;
	track->trackEnable = true;
	track->clipNumber = 2;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[2]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("walk", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("attack", 1));
	rootObject->animationManual.insert(std::pair<std::string, int>("death", 2));
	rootObject->animationManual.insert(std::pair<std::string, int>("idle", 3));

#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(0.15f, 0.15f, 0.15f);
	Vector3 xmf3rotate = Vector3(0, 1, 0);
	Rotate(&xmf3rotate, 90);

	objectType = OBJECT_TYPE::MONSTER;
	objectSound = snd;
	SetObjectState();

	CShader* attackShader = new CMonsterAttackShader;
	attackShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	bossAttack = new EffectBossAttack(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, attackShader);
}

CGolemModel::~CGolemModel()
{
}

void CGolemModel::Die()
{
	objectSound->PlayMonster(SOUND_TYPE::MAGMA_DEATH);
}

void CGolemModel::Attack()
{
	Matrix4x4 colWorld = Matrix4x4::identity;
	colWorld._41 = m_xmf3Position.x;
	colWorld._42 = m_xmf3Position.y - 160;
	colWorld._43 = m_xmf3Position.z;
	if (!bossAttack->GetIsAlive())
	{
		bossAttack->SetIsAlive(true);
		bossAttack->SetWorldMatrix(colWorld);
		objectSound->PlayShoot(SOUND_TYPE::GOLEM_ATTACK_SOUND, 0.3);
	}
	bossAttack->SetCameraPos(m_xmf3Position);
}

void CGolemModel::Update(float fTimeElapsed, CCamera* camera)
{
	CMovableHierarchyObject::Update(fTimeElapsed, camera);
	bossAttack->Update(fTimeElapsed, camera);
}

void CGolemModel::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CMovableHierarchyObject::Render(pd3dCommandList, pCamera);
	if(renderType == RENDER_TYPE::IDLE_RENDER)
		bossAttack->Render(pd3dCommandList, pCamera);
}

InteractionObject::InteractionObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) :CMovableHierarchyObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL)
{
}

ChestObject::ChestObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader):InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/ChestFull_albedo.dds";
	textureinfo.normal = L"Image/ChestFull_normal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("ChestFull", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Chest");

	asAnimationTrack* track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->animationSpeed = 20.0f;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[0]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = false;
	track->animationSpeed = 20.0f;
	track->clipNumber = 1;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[1]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->animationSpeed = 20.0f;
	track->clipNumber = 2;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[2]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = false;
	track->animationSpeed = 20.0f;
	track->clipNumber = 3;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[3]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.clear();

	rootObject->animationManual.insert(std::pair<std::string, int>("closed", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("closing", 1));
	rootObject->animationManual.insert(std::pair<std::string, int>("opened", 2));
	rootObject->animationManual.insert(std::pair<std::string, int>("opening", 3)); 

	Item* item;
	for (int i = 0; i < 4; ++i)
	{
		item = new Item(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		item->setItemType(static_cast<ITEM_TYPE>(Mathf::RandF(0, 8)));
		itemObjects.push_back(item);
	}

	objectZPosition = 40.f;	boundingBoxPosition = 40.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(100, 50, 55), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB
	SetScale(0.7f, 0.7f, 0.7f);

	objectType = OBJECT_TYPE::INTERACTION;
	objectSound = snd;
	SetObjectState();
}

ChestObject::~ChestObject()
{
}
void ChestObject::Update(float fTimeElapsed, CCamera* camera)
{
	CMovableHierarchyObject::Update(fTimeElapsed, camera);
	Vector3 itempos = GetPosition();
	itempos.y += 40;
	itemObjects[0]->SetPosition(itempos + GetLookVector() * 45);
	itemObjects[1]->SetPosition(itempos + GetLookVector() * 15);
	itemObjects[2]->SetPosition(itempos - GetLookVector() * 15);
	itemObjects[3]->SetPosition(itempos - GetLookVector() * 45);
	if (rootObject->m_pModelAnimator->isAniOncePlayFinish() && isOpen)
	{
		isOpen = false;
		for (auto& a : itemObjects) a->SetIsAlive(true);
	}
	for (auto& a : itemObjects) a->Update();
	
}

void ChestObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CMovableHierarchyObject::Render(pd3dCommandList, pCamera);
	if(renderType == RENDER_TYPE::IDLE_RENDER)
		for (auto& a : itemObjects) a->Render(pd3dCommandList, pCamera);
}

void ChestObject::Interact()
{
	if (rootObject->is_open) return;
	isOpen = true;
	rootObject->IsOpen();
}

void ChestObject::SetItem(int index, ITEM_TYPE type)
{
	itemObjects.at(index)->setItemType(type);
}

void ChestObject::SetItemAlive(int index, bool alive)
{
	if(itemObjects.at(index)->GetIsAlive())
		itemObjects.at(index)->SetIsAlive(alive);
}


DoorObject::DoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/DefaultMaterial_albedo.dds";
	textureinfo.normal = L"Image/DefaultMaterial_normal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("DefaultMaterial", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Door");
	
	rootObject->CloseAniNum = 1;
	rootObject->OpenAniNum = 1;

	rootObject->currentAniNum = 1;
	rootObject->currentAniTime = std::chrono::system_clock::now();

	asAnimationTrack* track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->trackEnable = true;
	track->clipNumber = 0;
	track->animationSpeed = 15.0f;
	track->timeStartPosition = 0.0f;
	track->timeEndPosition = 0.1f;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	track = new asAnimationTrack;
	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->animationSpeed = 15.0f;
	track->trackEnable = false;
	track->clipNumber = 0;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[0]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	/*track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 1;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[1]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);*/

	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("close", 0));
	rootObject->animationManual.insert(std::pair<std::string, int>("opening", 1));

	objectZPosition = 90.f;	boundingBoxPosition = 125.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(270, 400, 50), Vector4(0, 0, 0, 1));

#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(0.6f, 0.30f, 0.45f);
	objectType = OBJECT_TYPE::INTERACTION;
	objectSound = snd;
	SetObjectState();
}

DoorObject::~DoorObject()
{
}

void DoorObject::Interact()
{
	if (rootObject->is_open)
		return;
	rootObject->IsOpen();
	objectSound->PlayShoot(SOUND_TYPE::DOOR_OPEN_SOUND, 0.7);
}

LeverObject::LeverObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/lever_diffuseOriginal.dds";
	textureinfo.normal = L"Image/lever_normal.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("Material.001", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Lever");
	
	rootObject->CloseAniNum = 1;
	rootObject->OpenAniNum = 1;
	rootObject->currentAniNum = 0;
	rootObject->currentAniTime = std::chrono::system_clock::now();

	asAnimationTrack* track = new asAnimationTrack;

	track->animationPlayType = ANIMATION_PLAY_TYPE::ANIMATION_PLAY_ONCE;
	track->clipNumber = 0;
	track->animationSpeed = 7.5f;
	track->trackEnable = true;
	track->timeEndPosition = rootObject->m_pModelAnimator->animationClips[0]->Duration;
	rootObject->m_pModelAnimator->animationTrack.push_back(track);

	rootObject->animationManual.clear();
	rootObject->animationManual.insert(std::pair<std::string, int>("opening", 0));


	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(1.0f, 1.0f, 1.0f);
	Vector3 xmf3rotate = Vector3(0, 1, 0);
	Rotate(&xmf3rotate, 180.f);

	objectType = OBJECT_TYPE::INTERACTION;
	objectSound = snd;
	SetObjectState();
}

void LeverObject::Interact()
{
	rootObject->IsOpen();
}

Stone4Object::Stone4Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/T_Rock_04_D.dds";
	textureinfo.normal = L"Image/T_Rock_04_N.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("M_Rock_04", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Stone");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(0.5f, 0.5f, 0.5f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

Stone5Object::Stone5Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = false;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/T_Rock_05_D.dds";
	textureinfo.normal = L"Image/T_Rock_05_N.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("M_Rock_05", textureinfo));

	CShader* shader = new CMonsterShader;
	
	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 3, 6);	

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Stone");
	objectZPosition = 0.f;	boundingBoxPosition = 20.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(200, 150, 300), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(0.25f, 0.25f, 0.25f);
	Vector3 xmf3rotate = Vector3(1, 0, 0);
	Rotate(&xmf3rotate, 90.f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

Stone6Object::Stone6Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = false;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/T_Rock_06_D.dds";
	textureinfo.normal = L"Image/T_Rock_06_N.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("M_Rock_06", textureinfo));

	CShader* shader = new CMonsterShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 3, 6);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Stone");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(200, 150, 250), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(0.15f, 0.15f, 0.15f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

MudObject::MudObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) :InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/mud_A_d.dds";
	textureinfo.normal = L"Image/mud_A_n.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("mud_A", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Mud");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(1.0f, 1.0f, 1.0f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

RocksAObject::RocksAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) : InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/rock_A_d.dds";
	textureinfo.normal = L"Image/rock_A_n.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("rock_A", textureinfo));

	CShader* shader = new CSkinnedAnimationShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Rocks");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(5.0f, 5.0f, 5.0f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

StairObject::StairObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) : InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/T_Rock_04_D.dds";
	textureinfo.normal = L"Image/wall_A_n.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("01 - Default", textureinfo));
	//textures.insert(std::pair<string, TEXTURE_INFO>("Wood_Satin_Max_2", textureinfo));

	CShader* shader = new CMonsterShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Stair");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif //DRAW_BB

	SetScale(1.0f, 1.0f, 1.0f);
	Vector3 xmf3rotate = Vector3(0, 1, 0);
	Rotate(&xmf3rotate, 90.f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}

WallObject::WallObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, const aiScene* scene, CShader* psoShader) : InteractionObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	isAlive = true;
	TEXTURE_INFO textureinfo;
	textureinfo.diffuse = L"Image/wall_A_d.dds";
	textureinfo.normal = L"Image/wall_A_n.dds";
	textures.insert(std::pair<string, TEXTURE_INFO>("wall_A", textureinfo));

	CShader* shader = new CMonsterShader;

	shader->SetPso(psoShader->GetPso());
	shader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 4);

	rootObject = CFbxModelObject::LoadFbxRootObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, false, textures, shader, scene, true, "Walls");

	objectZPosition = 0.f;	boundingBoxPosition = 0.f;
	objectBoundingBox = baseBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(20, 30, 30), Vector4(0, 0, 0, 1));
#ifdef DRAW_BB
	isRenderBoundingBox = true;
	CreateBoundingBoxObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
#endif // DRAW_BB

	SetScale(1.0f, 1.0f, 1.0f);

	objectType = OBJECT_TYPE::STATIC;
	SetObjectState();
}
