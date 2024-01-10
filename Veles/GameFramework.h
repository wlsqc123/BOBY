#pragma once


#pragma comment (lib, "winmm.lib")

#include "Timer.h"
#include "Player.h"
#include "SceneGame.h"
#include "SceneMgr.h"
#include "ServerMgr.h"

class GameFramework
{
public:
	GameFramework();
	~GameFramework();

	bool Init(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	
	void ChangeSwapChainState();

    void InitScenes();
    void Release();

    void ProcessInput();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void DrawFrame();
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	ServerMgr					server_mgr;
	SceneMgr*					sceneMgr;

	XMFLOAT3					pos;


	HINSTANCE					mainhInstance;
	HWND						mainhWnd; 

	int							mainClientWidth;
	int							mainClientHeight;
        
	IDXGIFactory4				*mainDxgiFactory = NULL;
	IDXGISwapChain3				*mainDxgiSwapChain = NULL;
	ID3D12Device				*mainDevice = NULL;

	bool						msaa4xEnable = false;
	UINT						msaa4xQualityLevels = 0;

	static const UINT			nSwapChainBuffers = 2;
	UINT						nSwapChainBufferIndex;

	vector<ID3D12Resource*>		mainSwapChainBackBuffers;
	ID3D12DescriptorHeap		*mainRtvDescriptorHeap = NULL;

	ID3D12Resource				*mainDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*mainDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*mainCmdAllocator = NULL;
	ID3D12CommandQueue			*mainCmdQueue = NULL;
	ID3D12GraphicsCommandList	*mainCmdList = NULL;

	ID3D12Fence					*mainFence = NULL;
	UINT64						fenceValues[nSwapChainBuffers];
	HANDLE						fenceEvent;

	DWORD						dwOldTime, dwCurrentGameTime;
	FLOAT						rDeltaTime, rAccumlationTime;


#if defined(_DEBUG)
	ID3D12Debug*				d3dDebugController;
#endif

	CGameTimer					gameTimer;
	POINT						oldCursorPos;

	_TCHAR						frameRate[50];

	bool						mainTimerEnable;
};

