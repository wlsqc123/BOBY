#pragma once
#include "Object.h"

class Fmod_snd;
class CGameObject;
class UIHpBar;
class Item;
class EffectBossAttack;
enum ITEM_TYPE;

struct CB_MONSTER_INFO
{
	Matrix4x4				worldMatrix;
};

class CMovableObject : public CGameObject
{
public:
	CMovableObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, int nMeshes = 1);
	virtual ~CMovableObject();


	virtual Vector3 GetPosition() { return(m_xmf3Position); }
	virtual Vector3 GetLookVector() { return(m_xmf3Look); }

	Vector3 GetUpVector() { return(m_xmf3Up); }
	Vector3 GetRightVector() { return(m_xmf3Right); }

	virtual void SetPosition(const Vector3& xmf3Position) { worldMatrix._41 = xmf3Position.x; worldMatrix._42 = xmf3Position.y; worldMatrix._43 = xmf3Position.z; }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	
	void SetObjectID(WORD id) { Object_Id = id; }
	WORD GetObjectID() { return Object_Id; }

protected:
	ID3D12Resource*			m_pd3dcbMonster = NULL;
	CB_MONSTER_INFO*		m_pcbMappedMonster = NULL;

	Vector3					m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3					m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
	Vector3					m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
	Vector3					m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);
	
	WORD					Object_Id = 99;
	Fmod_snd*				objectSound;
};

class CMovableHierarchyObject : public CMovableObject
{
public:
	CMovableHierarchyObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CMovableHierarchyObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual void SetPosition(const Vector3& xmf3Position);
	virtual void SetScale(float x, float y, float z);

	virtual void SetWorld(Matrix4x4 world);
	virtual void SetLook(Vector3 look, Vector3 lightLook);

	virtual void Rotate(Vector3* pxmf3Axis, float fAngle);
	virtual void Move(Vector3 pos);

	virtual void SetServerData(Vector3 pos, Vector3 look, Vector3 lightLook);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Update(float fTimeElapsed, CCamera* camera);

	CFbxModelObject* GetFbxObject() { return rootObject; };

	void CreateBoundingBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	void ChangebbRender() { isRenderBoundingBox = !isRenderBoundingBox; };
	virtual void SetHp(UINT damage);
	void SetObjectState();

	virtual void Attack() {}
	virtual void Die() {}

protected:
	CFbxModelObject*		rootObject = NULL;
	BoundingOrientedBox		baseBoundingBox;
	float					objectZPosition = 0.f;
	float					boundingBoxPosition = 0.f;
	UIHpBar*				objectHpBar = NULL;
	int						objectHp = 100;
	int						objectMaxHp = 100;
	bool					isRenderBoundingBox = true;
};


class CMagmaMonster : public CMovableHierarchyObject
{
private:

public:
	CMagmaMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~CMagmaMonster();

	virtual void Die();
};

class COGREMonster : public CMovableHierarchyObject
{
public:
	COGREMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~COGREMonster();

	virtual void SetLook(Vector3 look, Vector3 lightLook);
	virtual void Die();
};

class CGolemModel : public CMovableHierarchyObject
{
public:
	CGolemModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~CGolemModel();

	virtual void Die();
	virtual void Attack();
	virtual void Update(float fTimeElapsed, CCamera* camera) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)override;
	EffectBossAttack* bossAttack;
};


class CTestPlayerModel : public CMovableHierarchyObject
{
public:
	WORD Player_ID = 99;
	CTestPlayerModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~CTestPlayerModel();

	virtual void SetLook(Vector3 look, Vector3 lightLook);
};

class InteractionObject : public CMovableHierarchyObject
{
protected:
	vector<Item*> itemObjects;
public:
	InteractionObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~InteractionObject() {};
	virtual void Interact() {}
	virtual vector<Item*> GetItem() { return itemObjects; }
	virtual void SetItem(int index, ITEM_TYPE type) {}
	virtual void SetItemAlive(int index, bool alive) {}
};

class ChestObject : public InteractionObject
{
private:
	bool isOpen = false;
public:
	ChestObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~ChestObject();
	virtual void Update(float fTimeElapsed, CCamera* camera) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Interact();
	virtual void SetItem(int index, ITEM_TYPE type);
	virtual void SetItemAlive(int index, bool alive);
};

class DoorObject : public InteractionObject
{
public:
	DoorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~DoorObject();
	virtual void Interact();
};

class LeverObject : public InteractionObject
{
public:
	LeverObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~LeverObject() {}
	virtual void Interact();
};

class Stone4Object : public InteractionObject
{
public:
	Stone4Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~Stone4Object() {};
};

class Stone5Object : public InteractionObject
{
public:
	Stone5Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~Stone5Object() {};
};

class Stone6Object : public InteractionObject
{
public:
	Stone6Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~Stone6Object() {};
};

class MudObject : public InteractionObject
{
public:
	MudObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~MudObject() {};
};

class RocksAObject : public InteractionObject
{
public:
	RocksAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~RocksAObject() {};
};

class StairObject : public InteractionObject
{
public:
	StairObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~StairObject() {};
};

class WallObject : public InteractionObject
{
public:
	WallObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL, const aiScene* scene = nullptr, CShader* psoShader = nullptr);
	virtual ~WallObject() {};
};