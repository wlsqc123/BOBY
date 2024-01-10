#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"
#include "ServerMgr.h"

class UIPlayerHp;
class UIMagazine;
class Inventory;
class UITexture;
class Fmod_snd;

struct CB_PLAYER_INFO
{
	Matrix4x4					worldMatrix;
};

struct PLAYER_STATUS
{

};

class CPlayer : public CGameObject
{
protected:
	Vector3					m_xmf3Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3					m_xmf3Right = Vector3(1.0f, 0.0f, 0.0f);
	Vector3					m_xmf3Up = Vector3(0.0f, 1.0f, 0.0f);
	Vector3					m_xmf3Look = Vector3(0.0f, 0.0f, 1.0f);

	Vector3					m_xmf3Velocity = Vector3(0.0f, 0.0f, 0.0f);
	Vector3     			m_xmf3Gravity = Vector3(0.0f, 0.0f, 0.0f);

	float           		m_fPitch = 0.0f;
	float           		m_fYaw = 0.0f;
	float           		m_fRoll = 0.0f;

	float           		m_fMaxVelocityXZ = 0.0f;
	float           		m_fMaxVelocityY = 0.0f;
	float           		m_fFriction = 0.0f;

	LPVOID					m_pPlayerUpdatedContext = NULL;
	LPVOID					m_pCameraUpdatedContext = NULL;

	CCamera					*gsCamera = NULL;

	UINT					playerHp = 100;
	UINT					playerSpeed = 0;
	UINT					playerGunSpeed = 0;
	UINT					playerDamage = 0;
	UINT					maxBullet = 0;
	UINT					numBullet = 0;

	double					attackSpeed = 0;
	UIPlayerHp				* playerHpUi = NULL;
	UIMagazine				* playerMagazineUi = NULL;
	UITexture				* youdiedTexture = NULL;
	UITexture				* activeItemUI = NULL;
	UITexture				* activeItem = NULL;
	UITexture* attackTexture = NULL;
	Inventory				* playerInventory = NULL;
	bool					drawGrayScale = false;
	bool					drawInventory = false;
	bool					drawYoudied = false;
	bool					isAttacked = false;
	chrono::system_clock::time_point	attackTime;
public:
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext=NULL, int nMeshes = 1);
	virtual ~CPlayer();

	Vector3 GetPosition() { return(m_xmf3Position); }
	Vector3 GetLookVector() { return(m_xmf3Look); }
	Vector3 GetUpVector() { return(m_xmf3Up); }
	Vector3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const Vector3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const Vector3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	virtual void SetPosition(const Vector3& xmf3Position);

	const Vector3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(gsCamera); }
	void SetCamera(CCamera *pCamera) { gsCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const Vector3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	virtual void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	void		SetHp(int hp, int maxhp);
	void		SetBullet(int bullet) { numBullet = bullet; }
	int			GetBullet() const { return numBullet; }
	void		SetAttackSpeed(double speed) { attackSpeed = speed; }
	double		GetAttackSpeed() const { return attackSpeed; }
	bool		CheckReload();

	void		DrawInventory(bool isDraw);
	bool		GetDrawGrayScale() { return drawGrayScale; }
	void		SetItem(ITEM_TYPE item);
	void		SetActiveItemAlive(bool alive);
	void		SetPositionBase(const Vector3& xmf3Position);

	void		CameraZoomInOut();

	CFbxModelObject* rootObject = NULL;
	Fmod_snd* objectSound;
protected:
	ID3D12Resource	*m_pd3dcbPlayer = NULL;
	CB_PLAYER_INFO	*m_pcbMappedPlayer = NULL;
	bool			zoomEnable = false;
};

class CRiflePlayer:public CPlayer
{
public:
	CRiflePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, int nMeshes = 1, CShader* shader = nullptr, CShader* uiShader = nullptr);
	virtual ~CRiflePlayer();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CShotgunPlayer :public CPlayer
{
public:
	CShotgunPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, int nMeshes = 1, CShader* shader = nullptr, CShader* uiShader = nullptr);
	virtual ~CShotgunPlayer();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CGrenadelauncherPlayer :public CPlayer
{
public:
	CGrenadelauncherPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, int nMeshes = 1, CShader* shader = nullptr, CShader* uiShader = nullptr);
	virtual ~CGrenadelauncherPlayer();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CSniperPlayer :public CPlayer
{
private:
	UITexture* sniperZoom = NULL;
public:
	CSniperPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Fmod_snd* snd, void* pContext = NULL, int nMeshes = 1, CShader* shader = nullptr, CShader* uiShader = nullptr);
	virtual ~CSniperPlayer();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};