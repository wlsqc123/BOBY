#pragma once
#include "stdafx.h"
#include "Timer.h"
class LavaWave
{
public:
	LavaWave(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, int m, int n, float dx, float dt, float speed, float damping);
	LavaWave(const LavaWave& rhs) = delete;
	LavaWave& operator=(const LavaWave& rhs) = delete;
	~LavaWave() = default;

	UINT RowCount()const;
	UINT ColumnCount()const;
	UINT VertexCount()const;
	UINT TriangleCount()const;
	float Width()const;
	float Depth()const;
	float SpatialStep()const;

	CD3DX12_GPU_DESCRIPTOR_HANDLE DisplacementMap()const;

	UINT DescriptorCount()const;

	void BuildResources(ID3D12GraphicsCommandList * cmdList);

	void BuildDescriptors();

	void Update(float timeElapsed, ID3D12GraphicsCommandList* cmdList);

	void Disturb(ID3D12GraphicsCommandList* cmdList, UINT i, UINT j, float magnitude);

	void CreateWavePipelineState(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* CreateWaveRootSignature(ID3D12Device* pd3dDevice);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateUpdateComputeShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateDisturbComputeShader(ID3DBlob** ppd3dShaderBlob);
	void SetDescriptorHeap(ID3D12GraphicsCommandList* cmdList);

private:

	UINT mNumRows;
	UINT mNumCols;

	UINT mVertexCount;
	UINT mTriangleCount;

	// Simulation constants we can precompute.
	float mK[3];

	float mTimeStep;
	float mSpatialStep;

	ID3D12Device* md3dDevice = nullptr;

	ID3D12PipelineState* mUpdatePso = nullptr;
	ID3D12PipelineState* mDisturbPso = nullptr;

	ID3D12RootSignature* mWaveRootSignature = nullptr;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolSrv;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolUav;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	ComPtr<ID3D12Resource> mPrevSol = nullptr;
	ComPtr<ID3D12Resource> mCurrSol = nullptr;
	ComPtr<ID3D12Resource> mNextSol = nullptr;

	ComPtr<ID3D12Resource> mPrevUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> mCurrUploadBuffer = nullptr;
};

