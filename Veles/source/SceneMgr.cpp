#include "stdafx.h"
#include "SceneMgr.h"
#include "SceneGame.h"
#include "SceneLoby.h"
#include "SceneEnding.h"

SceneMgr::SceneMgr(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12CommandQueue* cmdQueue) :sceneMgrDevice{ pd3dDevice }, sceneMgrCmdList{ pd3dCommandList }, sceneMgrCmdQueue{ cmdQueue }
{
}

SceneMgr::~SceneMgr()
{
}

void SceneMgr::Init(ID3D12Resource* dsvBuffer, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12DescriptorHeap* dsvDescriptorHeap)
{
	scenes.push_back(new SceneGame);
	scenes.push_back(new SceneLoby);
	scenes.push_back(new SceneEnding);

	for (const auto& q : scenes)
	{
		q->SetDsv(dsvBuffer);
		q->SetRtvDescHeap(rtvDescriptorHeap);
		q->SetDsvDescHeap(dsvDescriptorHeap);
		q->Init(sceneMgrDevice, sceneMgrCmdList);
	}
	currentScene = scenes.at(1);
}

void SceneMgr::ChangeScene()
{
	WEAPON_TYPE type = currentScene->GetWeaponType();
	if (currentScene == scenes.at(1))
		currentScene = scenes.at(0);
	else if(currentScene == scenes.at(0))
		currentScene = scenes.at(2);
	currentScene->InitWeapon(sceneMgrDevice, sceneMgrCmdList, type);
}

void SceneMgr::PopScene()
{
}

void SceneMgr::Release()
{
	for (const auto& q : scenes)
		q->Release();
}

void SceneMgr::ReleaseUploadBuffers()
{
	for (const auto& q : scenes)
		q->ReleaseUploadBuffers();
}

void SceneMgr::Render(ID3D12GraphicsCommandList* pd3dCommandList, RENDER_TYPE rt)
{
	switch (rt)
	{
	case RENDER_TYPE::IDLE_RENDER:
		currentScene->Render(pd3dCommandList);
		break;
	case RENDER_TYPE::PREV_RENDER:
		currentScene->RenderPostProcessing(pd3dCommandList);
		break;
	default:
		break;
	}

}

void SceneMgr::Update(float fTimeElapsed, ServerMgr* servermgr)
{
	if (currentScene->GetSceneChangeFlag()) ChangeScene();
	currentScene->Update(fTimeElapsed, servermgr);
}

void SceneMgr::ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr)
{
	currentScene->ProcessInput(hWnd, timeElapsed, servermgr);
}

void SceneMgr::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	currentScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
}

void SceneMgr::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	currentScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
}
