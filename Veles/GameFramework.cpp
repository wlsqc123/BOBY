#include "stdafx.h"
#include "GameFramework.h"

GameFramework::GameFramework()
{
	mainDxgiFactory = NULL;
	mainDxgiSwapChain = NULL;
	mainDevice = NULL;

	mainSwapChainBackBuffers.reserve(nSwapChainBuffers);
	for (UINT i = 0; i < nSwapChainBuffers; ++i) mainSwapChainBackBuffers.push_back(nullptr);
	nSwapChainBufferIndex = 0;

	mainCmdAllocator = NULL;
	mainCmdQueue = NULL;
	mainCmdList = NULL;

	mainRtvDescriptorHeap = NULL;
	mainDsvDescriptorHeap = NULL;

	fenceEvent = NULL;
	mainFence = NULL;
	for (int i = 0; i < nSwapChainBuffers; i++) fenceValues[i] = 0;

	mainClientWidth = FRAME_BUFFER_WIDTH;
	mainClientHeight = FRAME_BUFFER_HEIGHT;

	_tcscpy_s(frameRate, _T("Veles      (   ("));

	dwOldTime = timeGetTime(); 
	dwCurrentGameTime = timeGetTime();
	rDeltaTime = 0.0f; 
	rAccumlationTime = 0.0f;
	mainTimerEnable = false;
}

GameFramework::~GameFramework()
{

}

bool GameFramework::Init(HINSTANCE hInstance, HWND hMainWnd)			//1����
{
	std::cout << "Init FrameWork" << std::endl;
	mainhInstance = hInstance;
	mainhWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	mainCmdList->Reset(mainCmdAllocator, NULL);

	CreateSwapChain();
	CreateRenderTargetViews();
	CreateDepthStencilView();
	InitScenes();

	server_mgr.Initialize(hMainWnd);
	strcpy(server_mgr.CSpacket_lobby.name, sceneMgr->GetName().c_str());


	return(true);
}

//#define _WITH_SWAPCHAIN

void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(mainhWnd, &rcClient);
	mainClientWidth = rcClient.right - rcClient.left;
	mainClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = mainClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = mainClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = mainhWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif
	HRESULT hResult = mainDxgiFactory->CreateSwapChain(mainCmdQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&mainDxgiSwapChain);

	if (!mainDxgiSwapChain)
	{
		MessageBox(NULL, L"Swap Chain Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	hResult = mainDxgiFactory->MakeWindowAssociation(mainhWnd, DXGI_MWA_NO_ALT_ENTER);
	nSwapChainBufferIndex = mainDxgiSwapChain->GetCurrentBackBufferIndex();
}

void GameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&mainDxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != mainDxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&mainDevice))) break;
	}

	if (!mainDevice)
	{
		hResult = mainDxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void **)&mainDevice);
	}

	if (!mainDevice)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = mainDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	msaa4xEnable = (msaa4xQualityLevels > 1) ? true : false;

	hResult = mainDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&mainFence);
	for (UINT i = 0; i < nSwapChainBuffers; i++) fenceValues[i] = 0;

	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnDsvDescriptorIncrementSize = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	::gnRtvDescriptorIncrementSize = mainDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void GameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = mainDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&mainCmdQueue);

	hResult = mainDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&mainCmdAllocator);

	hResult = mainDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mainCmdAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&mainCmdList);
	hResult = mainCmdList->Close();
}

void GameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = nSwapChainBuffers + 3;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = mainDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&mainRtvDescriptorHeap);

	d3dDescriptorHeapDesc.NumDescriptors = MAX_SHADOWMAP + 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = mainDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&mainDsvDescriptorHeap);
}

void GameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = mainRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < nSwapChainBuffers; i++)
	{
		mainDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&mainSwapChainBackBuffers.at(i));
		mainDevice->CreateRenderTargetView(mainSwapChainBackBuffers.at(i), NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += gnRtvDescriptorIncrementSize;
	}
}

void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = mainClientWidth;
	d3dResourceDesc.Height = mainClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = mainDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mainDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&mainDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	mainDevice->CreateDepthStencilView(mainDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void GameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	mainDxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	mainDxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = mainClientWidth;
	dxgiTargetParameters.Height = mainClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mainDxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < nSwapChainBuffers; i++) if (mainSwapChainBackBuffers.at(i)) mainSwapChainBackBuffers.at(i)->Release();
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	mainDxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	mainDxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	mainDxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	mainDxgiSwapChain->ResizeBuffers(nSwapChainBuffers, mainClientWidth, mainClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
#endif
	nSwapChainBufferIndex = mainDxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void GameFramework::DrawFrame()
{
	if (mainTimerEnable)
	{
		gameTimer.GetFrameRate(frameRate + 12, 37);
		::SetWindowText(mainhWnd, frameRate);
	}
	else
	{
		wstring s = L"Veles";
		::SetWindowText(mainhWnd, s.c_str());
	}
}

void GameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (sceneMgr) sceneMgr->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
}

void GameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	WPARAM key_buffer = wParam;

	if (sceneMgr) sceneMgr->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (key_buffer)
		{
		case VK_F5:
			mainTimerEnable = !mainTimerEnable;
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (key_buffer)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	}
}

LRESULT CALLBACK GameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				gameTimer.Stop();
			else
				gameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;

		case WM_SOCKET:
		{
			switch (WSAGETSELECTEVENT(lParam)) {
			case FD_READ:
				break;
			}
			break;
		}
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:

        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void GameFramework::OnDestroy()
{
    Release();

	::CloseHandle(fenceEvent);

#if defined(_DEBUG)
	if (d3dDebugController) d3dDebugController->Release();
#endif

	if (mainDepthStencilBuffer) mainDepthStencilBuffer->Release();
	if (mainDsvDescriptorHeap) mainDsvDescriptorHeap->Release();

	for (int i = 0; i < nSwapChainBuffers; i++) if (mainSwapChainBackBuffers.at(i)) mainSwapChainBackBuffers.at(i)->Release();
	if (mainRtvDescriptorHeap) mainRtvDescriptorHeap->Release();

	if (mainCmdAllocator) mainCmdAllocator->Release();
	if (mainCmdQueue) mainCmdQueue->Release();
	if (mainCmdList) mainCmdList->Release();

	if (mainFence) mainFence->Release();

	mainDxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (mainDxgiSwapChain) mainDxgiSwapChain->Release();
    if (mainDevice) mainDevice->Release();
	if (mainDxgiFactory) mainDxgiFactory->Release();
	

	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pdxgiDebug->Release();
}

void GameFramework::InitScenes()
{
	cout << "Create Scene" << endl;
	sceneMgr = new SceneMgr(mainDevice, mainCmdList, mainCmdQueue);
	sceneMgr->Init(mainDepthStencilBuffer, mainRtvDescriptorHeap, mainDsvDescriptorHeap);

	mainCmdList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { mainCmdList };
	mainCmdQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (sceneMgr) sceneMgr->ReleaseUploadBuffers();

	gameTimer.Reset();
}

void GameFramework::Release()
{
	sceneMgr->Release();
}

void GameFramework::ProcessInput()
{
	sceneMgr->ProcessInput(mainhWnd, gameTimer.GetTimeElapsed(), &server_mgr);
}
	

void GameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++fenceValues[nSwapChainBufferIndex];
	HRESULT hResult = mainCmdQueue->Signal(mainFence, nFenceValue);

	if (mainFence->GetCompletedValue() < nFenceValue)
	{
		hResult = mainFence->SetEventOnCompletion(nFenceValue, fenceEvent);
		::WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void GameFramework::MoveToNextFrame()
{
	nSwapChainBufferIndex = (nSwapChainBufferIndex + 1) % nSwapChainBuffers;

	UINT64 nFenceValue = ++fenceValues[nSwapChainBufferIndex];
	HRESULT hResult = mainCmdQueue->Signal(mainFence, nFenceValue);

	if (mainFence->GetCompletedValue() < nFenceValue)
	{
		hResult = mainFence->SetEventOnCompletion(nFenceValue, fenceEvent);
		::WaitForSingleObject(fenceEvent, INFINITE);
	}

	DrawFrame();
}


void GameFramework::FrameAdvance()	
{   
	gameTimer.Tick(60);

	ProcessInput();

	server_mgr.SendPacket();
	server_mgr.RecvPacket();

	//서버의 데이터를 가지고 업데이트 
	sceneMgr->Update(gameTimer.GetTimeElapsed(), &server_mgr);


	HRESULT hResult = mainCmdAllocator->Reset();
	hResult = mainCmdList->Reset(mainCmdAllocator, NULL);

	//그림자, ssao, 용암등 사전에 렌더링해야 할 부분
	renderType = RENDER_TYPE::PREV_RENDER;
	sceneMgr->Render(mainCmdList, renderType);

	//화면 렌더링단계
	renderType = RENDER_TYPE::IDLE_RENDER;
	mainCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mainSwapChainBackBuffers.at(nSwapChainBufferIndex),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	D3D12_CPU_DESCRIPTOR_HANDLE hCpuRtv = mainRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	hCpuRtv.ptr += nSwapChainBufferIndex * gnRtvDescriptorIncrementSize;

	mainCmdList->ClearRenderTargetView(hCpuRtv, Colors::Azure, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE hCpuDsv = mainDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mainCmdList->ClearDepthStencilView(hCpuDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	mainCmdList->OMSetRenderTargets(1, &hCpuRtv, TRUE, &hCpuDsv);

	sceneMgr->Render(mainCmdList, renderType);
				
	mainCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mainSwapChainBackBuffers.at(nSwapChainBufferIndex),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	hResult = mainCmdList->Close();
	
	ID3D12CommandList *ppd3dCommandLists[] = { mainCmdList };
	mainCmdQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);
	 
	WaitForGpuComplete();

	mainDxgiSwapChain->Present(0, 0);
	MoveToNextFrame();

}
