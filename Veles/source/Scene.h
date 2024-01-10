#pragma once

#include "Sound.h"
#include "ServerMgr.h"
#include "Player.h"
#include "Light.h"

class Scene
{
protected:
	ID3D12Resource*			dsvBuffer = nullptr;
	ID3D12DescriptorHeap*	rtvDescriptorHeap = nullptr;
	ID3D12DescriptorHeap*	dsvDescriptorHeap = nullptr;
	bool					sceneChangeFlag = false;
	string					word;

public:
	Scene() {};
	~Scene() {};

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};

	virtual void ReleaseUploadBuffers() {};
	virtual void Release() {};

	virtual void Update(float fTimeElapsed, ServerMgr* servermgr) {};
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void RenderPostProcessing(ID3D12GraphicsCommandList* pd3dCommandList) {};

	virtual void ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr) {};
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {};
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {};

	virtual void InitWeapon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,WEAPON_TYPE type) {};

	virtual WEAPON_TYPE GetWeaponType() { return WEAPON_RIFLE; }

	void SetDsv(ID3D12Resource* dsv) { dsvBuffer = dsv; }
	void SetDsvDescHeap(ID3D12DescriptorHeap* dsvdesc) { dsvDescriptorHeap = dsvdesc; }
	void SetRtvDescHeap(ID3D12DescriptorHeap* rtvdesc) { rtvDescriptorHeap = rtvdesc; }
	bool GetSceneChangeFlag() { return sceneChangeFlag; }

	void SetName(string str) { word = str; };
	string GetName() { return word; };
};

