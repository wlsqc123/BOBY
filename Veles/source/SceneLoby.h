#pragma once

#include "Scene.h"
#include "font.h"
class SceneLoby : public Scene
{
public:
	SceneLoby();
	~SceneLoby() {};

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr);
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	
	virtual void Update(float fTimeElapsed, ServerMgr* servermgr);

	bool CheckMousePos(Vector2 min, Vector2 max);
	virtual WEAPON_TYPE GetWeaponType() { return glWeaponType; }
private:
	ID3D12RootSignature*	glRootSignature = nullptr;
	UITexture*		glLobyTexture;
	vector<Font*>	glUserName;
	D3D12_VIEWPORT	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

	POINT			cursorPos;
	WEAPON_TYPE		glWeaponType = WEAPON_RIFLE;
	
	UITexture* glRifleIcon;
	UITexture* glSniperIcon;
	UITexture* glShotgunIcon;
};

