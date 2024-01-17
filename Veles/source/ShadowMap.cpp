#include "stdafx.h"
#include "ShadowMap.h"


ShadowMap::ShadowMap(ID3D12Device* device, UINT width, UINT height)
{
	d3dDevice = device;

	shadowMapWidth = width;
	shadowMapHeight = height;
	shadowViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	shadowScissorRect = { 0, 0, (int)width, (int)height };
	BuildResource();
}

UINT ShadowMap::Width()const
{
	return shadowMapWidth;
}

UINT ShadowMap::Height()const
{
	return shadowMapHeight;
}

ID3D12Resource* ShadowMap::Resource()
{
	return shadowMaps.Get();
}

D3D12_GPU_DESCRIPTOR_HANDLE ShadowMap::Srv()const
{
	return mhGpuSrv;
}

D3D12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv()const
{
	return mhCpuDsv;
}

D3D12_VIEWPORT ShadowMap::Viewport()const
{
	return shadowViewport;
}

D3D12_RECT ShadowMap::ScissorRect()const
{
	return shadowScissorRect;
}

void ShadowMap::BuildDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE hCpuSrv, D3D12_GPU_DESCRIPTOR_HANDLE hGpuSrv, D3D12_CPU_DESCRIPTOR_HANDLE hCpuDsv)
{

	mhCpuSrv = hCpuSrv;
	mhGpuSrv = hGpuSrv;
	mhCpuDsv = hCpuDsv;

	BuildDescriptors();
}

void ShadowMap::OnResize(UINT newWidth, UINT newHeight)
{
	if ((shadowMapWidth != newWidth) || (shadowMapHeight != newHeight))
	{
		shadowMapWidth = newWidth;
		shadowMapHeight = newHeight;

		BuildResource();
		BuildDescriptors();
	}
}

void ShadowMap::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_SHADOWCAMERA_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	cbShadowCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	cbShadowCamera->Map(0, NULL, (void**)&cbMappedShadowCamera);
}

void ShadowMap::ReleaseShaderVariables()
{
}

void ShadowMap::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&cbMappedShadowCamera->m_xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&shadowView)));
	XMStoreFloat4x4(&cbMappedShadowCamera->m_xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&shadowProj)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = cbShadowCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(SHADOWMAP_CBV, d3dGpuVirtualAddress);
}

void ShadowMap::SetShadowMatrix(Matrix4x4 view, Matrix4x4 proj)
{
	shadowView = view;
	shadowProj = proj;
}

void ShadowMap::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	d3dDevice->CreateShaderResourceView(shadowMaps.Get(), &srvDesc, mhCpuSrv);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	d3dDevice->CreateDepthStencilView(shadowMaps.Get(), &dsvDesc, mhCpuDsv);
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = shadowMapWidth;
	texDesc.Height = shadowMapHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = shadowTextureFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	d3dDevice->CreateCommittedResource(
		&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&shadowMaps));
}
