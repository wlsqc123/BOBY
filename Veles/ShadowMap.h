#pragma once

struct VS_CB_SHADOWCAMERA_INFO
{
	Matrix4x4						m_xmf4x4View;
	Matrix4x4						m_xmf4x4Projection;
};

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* device,
		UINT width, UINT height);

	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap& operator=(const ShadowMap& rhs) = delete;
	~ShadowMap() = default;

	UINT Width()const;
	UINT Height()const;
	ID3D12Resource* Resource();
	D3D12_GPU_DESCRIPTOR_HANDLE Srv()const;
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv()const;

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void BuildDescriptors(
		D3D12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		D3D12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		D3D12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

	void OnResize(UINT newWidth, UINT newHeight);

	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetShadowMatrix(Matrix4x4, Matrix4x4);

	
private:
	void BuildDescriptors();
	void BuildResource();
private:
	Matrix4x4					shadowView;
	Matrix4x4					shadowProj;

	D3D12_VIEWPORT				shadowViewport;
	D3D12_RECT					shadowScissorRect;
	ID3D12Device*				d3dDevice = nullptr;

	UINT						shadowMapWidth = 0;
	UINT						shadowMapHeight = 0;
	DXGI_FORMAT					shadowTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;

	D3D12_CPU_DESCRIPTOR_HANDLE	mhCpuSrv;
	D3D12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;

	ID3D12Resource*				cbShadowCamera = NULL;
	VS_CB_SHADOWCAMERA_INFO*	cbMappedShadowCamera = NULL;
	ComPtr<ID3D12Resource>		shadowMaps = nullptr;
};

