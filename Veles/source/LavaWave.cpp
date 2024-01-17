#include "stdafx.h"
#include "LavaWave.h"
#include <algorithm>
#include <cassert>

LavaWave::LavaWave(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	int m, int n, float dx, float dt, float speed, float damping)
{
	md3dDevice = device;

	mNumRows = m;
	mNumCols = n;

	assert((m * n) % 256 == 0);

	mVertexCount = m * n;
	mTriangleCount = (m - 1) * (n - 1) * 2;

	mTimeStep = dt;
	mSpatialStep = dx;

	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);
	mK[0] = (damping * dt - 2.0f) / d;
	mK[1] = (4.0f - 8.0f * e) / d;
	mK[2] = (2.0f * e) / d;
	BuildResources(cmdList);
	mWaveRootSignature = CreateWaveRootSignature(device);
	CreateWavePipelineState(device);
	BuildDescriptors();
}

UINT LavaWave::RowCount()const
{
	return mNumRows;
}

UINT LavaWave::ColumnCount()const
{
	return mNumCols;
}

UINT LavaWave::VertexCount()const
{
	return mVertexCount;
}

UINT LavaWave::TriangleCount()const
{
	return mTriangleCount;
}

float LavaWave::Width()const
{
	return mNumCols * mSpatialStep;
}

float LavaWave::Depth()const
{
	return mNumRows * mSpatialStep;
}

float LavaWave::SpatialStep()const
{
	return mSpatialStep;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE LavaWave::DisplacementMap()const
{
	return mCurrSolSrv;
}

UINT LavaWave::DescriptorCount()const
{
	return 6;
}

void LavaWave::BuildResources(ID3D12GraphicsCommandList* cmdList)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mNumCols;
	texDesc.Height = mNumRows;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mPrevSol));

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mCurrSol));

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mNextSol));

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mCurrSol.Get(), 0, num2DSubresources);

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mPrevUploadBuffer.GetAddressOf()));

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mCurrUploadBuffer.GetAddressOf()));

	std::vector<float> initData(mNumRows * mNumCols, 0.0f);
	for (int i = 0; i < initData.size(); ++i)
		initData[i] = 0.0f;

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.data();
	subResourceData.RowPitch = mNumCols * sizeof(float);
	subResourceData.SlicePitch = subResourceData.RowPitch * mNumRows;


	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mPrevSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, mPrevSol.Get(), mPrevUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mPrevSol.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, mCurrSol.Get(), mCurrUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNextSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void LavaWave::BuildDescriptors()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 6;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuStart;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuStart;

	srvCpuStart = mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	srvGpuStart = mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	md3dDevice->CreateShaderResourceView(mPrevSol.Get(), &srvDesc, srvCpuStart);
	srvCpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	md3dDevice->CreateShaderResourceView(mCurrSol.Get(), &srvDesc, srvCpuStart);
	srvCpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	md3dDevice->CreateShaderResourceView(mNextSol.Get(), &srvDesc, srvCpuStart);
	srvCpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	md3dDevice->CreateUnorderedAccessView(mPrevSol.Get(), nullptr, &uavDesc, srvCpuStart);
	srvCpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	md3dDevice->CreateUnorderedAccessView(mCurrSol.Get(), nullptr, &uavDesc, srvCpuStart);
	srvCpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	md3dDevice->CreateUnorderedAccessView(mNextSol.Get(), nullptr, &uavDesc, srvCpuStart);

	mPrevSolSrv = srvGpuStart;
	srvGpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	mCurrSolSrv = srvGpuStart;
	srvGpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	mNextSolSrv = srvGpuStart;
	srvGpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	mPrevSolUav = srvGpuStart;
	srvGpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	mCurrSolUav = srvGpuStart;
	srvGpuStart.ptr += gnCbvSrvDescriptorIncrementSize;
	mNextSolUav = srvGpuStart;
}

void LavaWave::Update(float timeElapsed,ID3D12GraphicsCommandList* cmdList)
{
	static float t = 0.0f;

	t += timeElapsed;
	cmdList->SetPipelineState(mUpdatePso);
	cmdList->SetComputeRootSignature(mWaveRootSignature);

	if (t >= mTimeStep)
	{
		cmdList->SetComputeRoot32BitConstants(0, 3, mK, 0);

		cmdList->SetComputeRootDescriptorTable(1, mPrevSolUav);
		cmdList->SetComputeRootDescriptorTable(2, mCurrSolUav);
		cmdList->SetComputeRootDescriptorTable(3, mNextSolUav);

		UINT numGroupsX = mNumCols / 16;
		UINT numGroupsY = mNumRows / 16;
		cmdList->Dispatch(numGroupsX, numGroupsY, 1);

		auto resTemp = mPrevSol;
		mPrevSol = mCurrSol;
		mCurrSol = mNextSol;
		mNextSol = resTemp;

		auto srvTemp = mPrevSolSrv;
		mPrevSolSrv = mCurrSolSrv;
		mCurrSolSrv = mNextSolSrv;
		mNextSolSrv = srvTemp;

		auto uavTemp = mPrevSolUav;
		mPrevSolUav = mCurrSolUav;
		mCurrSolUav = mNextSolUav;
		mNextSolUav = uavTemp;

		t = 0.0f;

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}

void LavaWave::Disturb(ID3D12GraphicsCommandList* cmdList, UINT i, UINT j, float magnitude)
{
	cmdList->SetPipelineState(mDisturbPso);
	cmdList->SetComputeRootSignature(mWaveRootSignature);

	UINT disturbIndex[2] = { j, i };
	cmdList->SetComputeRoot32BitConstants(0, 1, &magnitude, 3);
	cmdList->SetComputeRoot32BitConstants(0, 2, disturbIndex, 4);

	cmdList->SetComputeRootDescriptorTable(3, mCurrSolUav);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->Dispatch(1, 1, 1);
}

void LavaWave::CreateWavePipelineState(ID3D12Device* pd3dDevice)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

	D3D12_COMPUTE_PIPELINE_STATE_DESC updatePipelineStateDesc;
	D3D12_COMPUTE_PIPELINE_STATE_DESC disturbPipelineStateDesc;

	::ZeroMemory(&updatePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	updatePipelineStateDesc.pRootSignature = mWaveRootSignature;
	updatePipelineStateDesc.CS = CreateUpdateComputeShader(&pd3dVertexShaderBlob);
	updatePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&updatePipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&mUpdatePso);

	disturbPipelineStateDesc = updatePipelineStateDesc;
	disturbPipelineStateDesc.CS = CreateDisturbComputeShader(&pd3dVertexShaderBlob);

	hResult = pd3dDevice->CreateComputePipelineState(&disturbPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&mDisturbPso);
	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
}

ID3D12RootSignature* LavaWave::CreateWaveRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[4];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 6;
	pd3dRootParameters[0].Constants.ShaderRegister = 0;
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = nullptr;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}



D3D12_SHADER_BYTECODE LavaWave::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char* pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE LavaWave::CreateUpdateComputeShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Wave.hlsl", "UpdateWavesCS", "cs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE LavaWave::CreateDisturbComputeShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CompileShaderFromFile(L"Wave.hlsl", "DisturbWavesCS", "cs_5_1", ppd3dShaderBlob));
}


void LavaWave::SetDescriptorHeap(ID3D12GraphicsCommandList* cmdList)
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}