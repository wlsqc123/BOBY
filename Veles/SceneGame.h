//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Scene.h"
#include "Shader.h"
#include "Monster.h"
#include "EffectUI.h"
#include "LavaWave.h"
#include "Item.h"
#include "ShadowMap.h"
#include "Ssao.h"

struct CB_SHADOWMAPS_INFO
{
	Matrix4x4 shadowViews[MAX_SHADOWMAP];
	Matrix4x4 shadowProjs[MAX_SHADOWMAP];
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

struct ENVIRONMENT
{
	bool drawFog;
	float fogPos;
};

struct LAVAINFO
{
	Matrix4x4	gTextureAnimation;
	Vector2		gDisplacementMapTexelSize;
	float		gGridSpatialStep;
};

struct ZONE
{
	vector <int> monsterID;
};
class SceneGame : public Scene
{
public:
    SceneGame();
    ~SceneGame();

	virtual void Init(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void BuildLightsAndMaterials();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	virtual void Update(float fTimeElapsed, ServerMgr* servermgr);
    virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void DrawNormalRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void ComputeWaveRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void RenderPostProcessing(ID3D12GraphicsCommandList* pd3dCommandList);
	
	virtual void ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr);
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void CreateShadowMap(ID3D12Device* pd3dDevice);
	void CreateShadowDescriptorHeap(ID3D12Device* pd3dDevice);
	void SetShadowCamera();
	void DrawNormalAndDepth(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseUploadBuffers();
	virtual void Release();
	
	void ShootingBullet();
	void PrintBoundingBox();
	void CheckinteractionObject();
	void CheckinteractionItem();

	InteractionObject* PickObjectByCameraLookVectorForChest();
	Item* PickObjectItem();

	void ReloadAmmo();
	virtual void InitWeapon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, WEAPON_TYPE type);
private:
	//오브젝트
	CPlayer*							gsRiflePlayer = NULL;
	CPlayer* gsSniperPlayer = NULL;
	CPlayer* gsShotgunPlayer = NULL;
	CPlayer* gsPlayer;
	CCamera*							gsCamera = NULL;
	vector<CShader*>					gsMapObject;
	CHeightMapTerrain*					gsTerrain = NULL;
	CSkyBox*							gsSkyBox = NULL;
	unique_ptr<LavaWave>				gsLavaMgr;

	vector<CMovableHierarchyObject*>	gsMonster;
	vector<CTestPlayerModel*>			gsOtherPlayer;
	vector<Effect*>						gsEffects;
	vector<CTerrainLava*>				gsLavaObject;
	vector<InteractionObject*>			gsInteractObjects;
	vector<EffectDecal*>				gsEffectsDecal;
	vector<EffectRangeAttack*>			gsEffectRangeAttack;
	
	vector< InteractionObject*>			gsStones;

	CLight *							gsLights = NULL;
	MATERIALS*							gsMaterials = NULL;

	ID3D12RootSignature*				gsRootSignature = nullptr;

	vector<std::unique_ptr<ShadowMap>>  gsShadowMaps;
	ID3D12DescriptorHeap*				gsShadowSrvDescHeap;

	unique_ptr<Ssao>					gsSsao;

	vector<Matrix4x4>					shadowMapViews;
	vector<Matrix4x4>					shadowMapProjs;

	ID3D12Resource*						gsCbShadowMaps = NULL;
	CB_SHADOWMAPS_INFO*					gsCbMappedShadowMaps = NULL;

	ID3D12Resource*						gsCbMaterials = NULL;
	MATERIAL*							gsCbMappedMaterials = NULL;

	ID3D12Resource*						gsCbEnvironment = NULL;
	ENVIRONMENT*						gsCbMappedEnvironment = NULL;

	ID3D12Resource*						gsCbLavaInfo = NULL;
	LAVAINFO*							gsCbMappedLavaInfo = NULL;
	LAVAINFO*							gsLavaInfo;
	bool								gsDrawfog;
	float								gsFogPos;

	POINT								oldCursorPos;
	short								playerID;
	BYTE								packet_type;
	UINT								reboundCount = 0;
	chrono::system_clock::time_point	startTime;
	chrono::system_clock::time_point	deltaTime;
	chrono::system_clock::time_point	currentTime;
	float								rotateX = 0, rotateY = 0;
	float								timer;
	Fmod_snd*							snd;

	float								bossStagePointRange = 0;
	float								bossStagePointAnimaition = 1.f;

	KeyInput							gsKeyInput;

	CS_ITEM								gsItemPacket;
	WEAPON_TYPE							gsWeaponType;

	vector<ZONE>						gsZone;
	int	gsZoneNum = 0;

	CShader* skinShader;
};
