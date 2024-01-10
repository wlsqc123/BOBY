#pragma once
#include <list>
#include "Scene.h"

class CCamera;
class ServerMgr;

class SceneMgr
{
private:
	std::vector<Scene*>			scenes;
	ID3D12Device*				sceneMgrDevice;
	ID3D12GraphicsCommandList*	sceneMgrCmdList;
	ID3D12CommandQueue*			sceneMgrCmdQueue;
	Scene* currentScene;


public:
	SceneMgr(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12CommandQueue* cmdQueue);
	virtual ~SceneMgr();

	void ChangeScene();
	void PopScene();
	void Init(ID3D12Resource* dsvBuffer, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12DescriptorHeap* dsvDescriptorHeap);
	void Release();
	void ReleaseUploadBuffers();
	void Render(ID3D12GraphicsCommandList* cmdList,RENDER_TYPE rt);
	void Update(float timeElapsed, ServerMgr* servermgr);

	void ProcessInput(HWND hWnd, float timeElapesd, ServerMgr* servermgr);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	Scene* GetCurrentScene() { return currentScene; };

	string GetName() { return currentScene->GetName(); };
	
};

