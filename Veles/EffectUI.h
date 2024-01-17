#pragma once
#include "Object.h"
#include "ServerMgr.h"
struct CB_EFFECT_INFO
{
	Matrix4x4			worldMatrix;
	UINT				frameCount;
	Vector3				lookPosition;
	float				cbTime;
};

class Effect :public CGameObject
{
public:
	Effect(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~Effect();
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	virtual void SetFrameCount(int count) { frameCount = count; }

protected:
	USHORT				textureWidth;
	USHORT				textureHeight;
	USHORT				timeCount;
	USHORT				frameCount;
	CB_EFFECT_INFO*		cbEffect = NULL;
	Vector3				lookPosition;
	float				liveTime = 0;
};


class EffectGunFire : public Effect
{
public:
	EffectGunFire(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectGunFire();

	virtual void Update(float fTimeElapsed, CCamera* camera);
};

class EffectCollision : public Effect
{
public:
	EffectCollision(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectCollision();

	virtual void Update(float fTimeElapsed, CCamera* camera);
};

class EffectBossAttack: public Effect
{
public:
	EffectBossAttack(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectBossAttack();
	void SetCameraPos(Vector3 pos);
	virtual void Update(float fTimeElapsed, CCamera* camera);
};

class EffectBlood : public Effect
{
public:
	EffectBlood(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectBlood() {}

	virtual void Update(float fTimeElapsed, CCamera* camera);
};

class UITexture : public Effect
{
public:
	UITexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pszFileName, Vector2 pos, Vector2 size, Vector2 textureSize, CShader* shader);
	virtual ~UITexture();
};

class UIHpBar : public Effect
{
public:
	UIHpBar(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~UIHpBar();
	void SetHp(UINT hp) { frameCount = hp; };
};

class UINumber : public Effect
{
public:
	UINumber(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature , Vector2 pos, Vector2 size, CShader* shader);
	virtual ~UINumber();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

class UIPlayerFace : public Effect
{
public:
	UIPlayerFace(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Vector2 pos, Vector2 size, CShader* shader);
	virtual ~UIPlayerFace();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

class UIPlayerHp : public Effect
{
private:
	vector<Effect*> uiNumHp;
	int	playerHp = 0;
	int MaxHp = 100;
public:
	UIPlayerHp(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~UIPlayerHp();

	virtual void Update(float fTimeElapsed, CCamera* camera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void SetHp(UINT hp, UINT maxhp) { playerHp = min(hp, MaxHp); MaxHp = maxhp; };
};

class UIMagazine : public Effect
{
private:
	vector<Effect*> uiNumHp;
	int	numBullets = 0;
	int maxBullets = 0;
public:
	UIMagazine(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature , int maxBullet, CShader* shader, WEAPON_TYPE type);
	virtual ~UIMagazine();

	virtual void Update(float fTimeElapsed, CCamera* camera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void SetBullet(UINT hp) { numBullets = min(hp, 999); };
};

class UIItem : public Effect
{
public:
	UIItem(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float posX, float posY, CShader* shader);
	virtual ~UIItem();
	void SetItem(ITEM_TYPE item);
};

class EffectDecal : public Effect
{
private:
	chrono::system_clock::time_point startTime;
public:
	EffectDecal(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectDecal() {};
	void SetCameraPos(Vector3 pos);
	virtual void Update(float fTimeElapsed, CCamera* camera);
};

class EffectRangeAttack : public Effect
{
public:
	EffectRangeAttack(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~EffectRangeAttack();

	virtual void Update(float fTimeElapsed, CCamera* camera);

};