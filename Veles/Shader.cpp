//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"

CShader::CShader()
{
	srvCPUDescriptorStartHandle.ptr = NULL;
	srvGPUDescriptorStartHandle.ptr = NULL;
}

CShader::~CShader()
{
	//if(pipelineStates[RENDER_TYPE::IDLE_RENDER])
	//	pipelineStates[RENDER_TYPE::IDLE_RENDER]->Release();
	//if (pipelineStates[RENDER_TYPE::SHADOW_RENDER])
	//	pipelineStates[RENDER_TYPE::SHADOW_RENDER]->Release();
	//if (pipelineStates[RENDER_TYPE::DRAW_NORMAL])
		//pipelineStates[RENDER_TYPE::DRAW_NORMAL]->Release();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreateShadowVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreateShadowPixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreateDrawNormalVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"DrawNormal.hlsl", "DrawNormalVS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShader::CreateDrawNormalPixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"DrawNormal.hlsl", "DrawNormalPS", "ps_5_1", ppd3dShaderBlob));
}


D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
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

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	UINT nInputElementDescs = 9;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[7] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[8] = { "MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, 96, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;

	ID3D12PipelineState* idlePipelineState;
	ID3D12PipelineState* shadowPipelineState;
	ID3D12PipelineState* drawNormalPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&idlePipelineState);
	pipelineStates[RENDER_TYPE::IDLE_RENDER] = idlePipelineState;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc;
	::ZeroMemory(&shadowPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	 shadowPsoDesc.pRootSignature = pd3dGraphicsRootSignature;
	 shadowPsoDesc.VS = CreateShadowVertexShader(&pd3dVertexShaderBlob);
	 shadowPsoDesc.PS = CreateShadowPixelShader(&pd3dPixelShaderBlob);
	 shadowPsoDesc.RasterizerState = CreateRasterizerState();
	 shadowPsoDesc.RasterizerState.DepthBias = 100000;
	 shadowPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	 shadowPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	 shadowPsoDesc.BlendState = CreateBlendState();
	 shadowPsoDesc.DepthStencilState = CreateDepthStencilState();
	 shadowPsoDesc.InputLayout = CreateInputLayout();
	 shadowPsoDesc.SampleMask = UINT_MAX;
	 shadowPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	 shadowPsoDesc.NumRenderTargets = 1;
	 shadowPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	 shadowPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	 shadowPsoDesc.SampleDesc.Count = 1;
	 shadowPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	 hResult = pd3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, __uuidof(ID3D12PipelineState), (void**)&shadowPipelineState);
	 pipelineStates[RENDER_TYPE::SHADOW_RENDER] = shadowPipelineState;

	 D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalPsoDesc = d3dPipelineStateDesc;
	 drawNormalPsoDesc.VS = CreateDrawNormalVertexShader(&pd3dVertexShaderBlob);
	 drawNormalPsoDesc.PS = CreateDrawNormalPixelShader(&pd3dPixelShaderBlob);
	 drawNormalPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	 drawNormalPsoDesc.SampleDesc.Quality = 0;
	 hResult = pd3dDevice->CreateGraphicsPipelineState(&drawNormalPsoDesc, __uuidof(ID3D12PipelineState), (void**)&drawNormalPipelineState);
	 pipelineStates[RENDER_TYPE::DRAW_NORMAL] = drawNormalPipelineState;

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
	if (shadowPsoDesc.InputLayout.pInputElementDescs) delete[] shadowPsoDesc.InputLayout.pInputElementDescs;
}

void CShader::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&cbvSrvDescriptorHeap);

	cbvCPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cbvGPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	srvCPUDescriptorStartHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	srvGPUDescriptorStartHandle.ptr = cbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	srvCPUDescriptorNextHandle = srvCPUDescriptorStartHandle;
	srvGPUDescriptorNextHandle = srvGPUDescriptorStartHandle;
}

void CShader::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void CShader::CreateConstantBufferViews(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffers, UINT nStride, int nObject)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;

	d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
	d3dCbvCPUDescriptorHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nObject);
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);

}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{

	srvCPUDescriptorStartHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	srvGPUDescriptorStartHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, srvCPUDescriptorStartHandle);
		srvCPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, srvGPUDescriptorStartHandle);
		srvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
}

void CShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CShader::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, Matrix4x4 *pxmf4x4World)
{
}

void CShader::ReleaseShaderVariables()
{
}

void CShader::ReleaseUploadBuffers()
{
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetPipelineState(pipelineStates[renderType]);

	pd3dCommandList->SetDescriptorHeaps(1, &cbvSrvDescriptorHeap);

	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlayerShader::CPlayerShader()
{
}

CPlayerShader::~CPlayerShader()
{
}

D3D12_SHADER_BYTECODE CPlayerShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSPlayer", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CPlayerShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSPlayer", "ps_5_1", ppd3dShaderBlob));
}


CMonsterShader::CMonsterShader()
{
}

CMonsterShader::~CMonsterShader()
{
}

D3D12_SHADER_BYTECODE CMonsterShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTexturedNormalMapLighting", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CMonsterShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedNormalMapLighting", "ps_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", ppd3dShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}


D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", ppd3dShaderBlob));
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkyBox", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", ppd3dShaderBlob));
}



CChestShader::CChestShader()
{
}

D3D12_SHADER_BYTECODE CChestShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTexturedLighting", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CChestShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedLighting", "ps_5_1", ppd3dShaderBlob));
}

CSkinnedAnimationShader::CSkinnedAnimationShader()
{
}

CSkinnedAnimationShader::~CSkinnedAnimationShader()
{
}

D3D12_SHADER_BYTECODE CSkinnedAnimationShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedTexturedNormalMapLighting", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSkinnedAnimationShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkinnedTexturedNormalMapLighting", "ps_5_1", ppd3dShaderBlob));
}

CBoundingBoxShader::CBoundingBoxShader()
{
}

CBoundingBoxShader::~CBoundingBoxShader()
{
}

D3D12_SHADER_BYTECODE CBoundingBoxShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CBoundingBoxShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

D3D12_RASTERIZER_DESC CBoundingBoxShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

CEffectShader::CEffectShader()
{
}

CEffectShader::~CEffectShader()
{
}

D3D12_INPUT_LAYOUT_DESC CEffectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 6;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "WIDTH", 0, DXGI_FORMAT_R32_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "HEIGHT", 0, DXGI_FORMAT_R32_UINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "RANDHEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "RANDDIR", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CEffectShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = FALSE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void CEffectShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL, * pd3dGeoShaderBlob = NULL;

	ID3D12PipelineState* idlePipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeoShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&idlePipelineState);
	pipelineStates[RENDER_TYPE::IDLE_RENDER] = idlePipelineState;

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	if (pd3dGeoShaderBlob) pd3dGeoShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;

}

D3D12_SHADER_BYTECODE CEffectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VS2dTexture", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CEffectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PS2dTexture", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CEffectShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSEffect", "gs_5_1", ppd3dShaderBlob));
}

CHpShader::CHpShader()
{
}

CHpShader::~CHpShader()
{
}

D3D12_SHADER_BYTECODE CHpShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSUIHpBar", "gs_5_1", ppd3dShaderBlob));
}

CUiShader::CUiShader()
{
}

CUiShader::~CUiShader()
{
}

D3D12_SHADER_BYTECODE CUiShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GS2DUI", "gs_5_1", ppd3dShaderBlob));
}


LavaShader::LavaShader()
{
}

LavaShader::~LavaShader()
{
}

void LavaShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

	ID3D12PipelineState* idlePipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	d3dPipelineStateDesc.BlendState = CreateBlendState();
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&idlePipelineState);
	pipelineStates[RENDER_TYPE::IDLE_RENDER] = idlePipelineState;


	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

D3D12_SHADER_BYTECODE LavaShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSLavaWave", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE LavaShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSLavaWave", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CFireEffectShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSFireEffect", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ItemShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSITEM", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ItemShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSITEM", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE PanelShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSPANEL", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE PanelShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSITEM", "ps_5_1", ppd3dShaderBlob));

}

BoneShader::BoneShader()
{
}

BoneShader::~BoneShader()
{
}

D3D12_SHADER_BYTECODE BoneShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BoneShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

D3D12_RASTERIZER_DESC BoneShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

InstancingShader::InstancingShader()
{
}

InstancingShader::~InstancingShader()
{
}

D3D12_SHADER_BYTECODE InstancingShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSInstance", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE InstancingShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedNormalMapLighting", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE InstancingShader::CreateShadowVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "ShadowVSInstance", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE InstancingShader::CreateDrawNormalVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"DrawNormal.hlsl", "DrawNormalVSInstance", "vs_5_1", ppd3dShaderBlob));
}

void InstancingShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	int xObjects = 20;
	int yObjects = 3;
	int zObjects = 7;

	float xPosition, yPosition, zPosition;
	float wallSize = 250;

	XMMATRIX xmmWorld;

	numObjects = 1331;

	CTexture* pTexture = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/stone2.dds", RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/stone2Normal.dds", RESOURCE_TEXTURE2D, 1);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pTexture, 0, TEXTURE_SRV);

	CMaterial* pCubeMaterial = new CMaterial();
	pCubeMaterial->SetTexture(pTexture);
	pCubeMaterial->SetReflection(0);

	//(10/13) 크기 조정
	CCubeMeshNormalMapTextured* pCubeLineMesh = new CCubeMeshNormalMapTextured(pd3dDevice, pd3dCommandList, wallSize, wallSize, wallSize / 10); //가로벽
	BoundingOrientedBox linebb = pCubeLineMesh->GetBoundingBox();

	wallObjects.reserve(numObjects);

	CTestWallObject* testWallObject = NULL;

	for (int x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				if (z == 0)
				{
					//가로 벽
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = linebb;
					xPosition = x * wallSize + 200;
					zPosition = z * wallSize * 3 + 100;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject); 
				}
				//외각벽
				if (z == zObjects - 1)
				{
					//가로 벽
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = linebb;
					xPosition = x * wallSize + 200;
					zPosition = 5045.f;

					yPosition = pTerrain->GetHeight(xPosition, zPosition) + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject);
				}
				//중앙 큰방 출구쪽 가로벽
				if (x != 15 || y != 0)
				{
					//가로 벽
					xPosition = x * wallSize + 200;
					zPosition = 1815;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) - 50 + wallSize * (0.5f + y);
					if ((x != 1 && x != 2 || y == 2))
					{
						testWallObject = new CTestWallObject(1);
						testWallObject->objectBoundingBox = linebb;

						testWallObject->SetPosition(xPosition, yPosition, zPosition);
						xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

						testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);

						wallObjects.push_back(testWallObject);
					}
				}

				//시작 방 기준 오른쪽벽
				if ((x != xObjects - 2 || y != 0))
				{
					//가로 벽
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = linebb;
					xPosition = x * wallSize + 200;
					zPosition = 4245.f;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + 40 + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject);
				}

				//보스방 가로벽
				if (x != 2 || y != 0)
				{
					//가로 벽
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = linebb;
					xPosition = x * wallSize + 200;
					zPosition = 1060;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + 50 + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject);
				}

			}

		}
	}
	//cout << wallObjects.size() << endl;
	wallObjects.at(0)->SetMesh(0, pCubeLineMesh);
	wallObjects.at(0)->SetMaterial(pCubeMaterial);
}

void InstancingShader::ReleaseObject()
{
	for (auto p : wallObjects)
	{
		delete p;
	}
	wallObjects.clear();
}

void InstancingShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	cbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(CB_INSTANCE) * numObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	cbGameObjects->Map(0, NULL, (void**)&cbMappedInstance);
}

void InstancingShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootShaderResourceView(MAP_SRV, cbGameObjects->GetGPUVirtualAddress());
	for (int i = 0; i < wallObjects.size(); i++)
	{
		cbMappedInstance[i].world = wallObjects.at(i)->GetWorldMatrix();
		XMStoreFloat4x4(&cbMappedInstance[i].world, XMMatrixTranspose(XMLoadFloat4x4(&wallObjects.at(i)->GetWorldMatrix())));
	}
}

void InstancingShader::ReleaseShaderVariables()
{
	if (cbGameObjects)
	{
		cbGameObjects->Unmap(0, NULL);
		cbGameObjects->Release();
	}

	for (auto p : wallObjects)
	{
		p->ReleaseShaderVariables();
	}
}

void InstancingShader::ReleaseUploadBuffers()
{
	for (auto p : wallObjects)
	{
		p->ReleaseUploadBuffers();
	}
}

void InstancingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender(pd3dCommandList);

	wallObjects.at(0)->Render(pd3dCommandList, pCamera, numObjects);
}

void InstancingRowShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	int xObjects = 20;
	int yObjects = 3;
	int zObjects = 7;

	float xPosition, yPosition, zPosition;
	float wallSize = 250;

	XMMATRIX xmmWorld;

	numObjects = 519;

	CTexture* pTexture = new CTexture(2, RESOURCE_TEXTURE2D_ARRAY, 0, 1);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/stone2.dds", RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/stone2Normal.dds", RESOURCE_TEXTURE2D, 1);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pTexture, 0, TEXTURE_SRV);

	CMaterial* pCubeMaterial = new CMaterial();
	pCubeMaterial->SetTexture(pTexture);
	pCubeMaterial->SetReflection(0);

	//(10/13) 크기 조정
	CCubeMeshNormalMapTextured* pCubeRowMesh = new CCubeMeshNormalMapTextured(pd3dDevice, pd3dCommandList, wallSize / 10, wallSize, wallSize); //세로벽
	BoundingOrientedBox Rowbb = pCubeRowMesh->GetBoundingBox();

	wallObjects.reserve(numObjects);

	CTestWallObject* testWallObject = NULL;

	for (int x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			for (int y = 0; y < yObjects; y++)
			{
				if (z == 0)
				{
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = Rowbb;

					xPosition = z * wallSize * 3 + 100;
					zPosition = x * wallSize + 200;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject); 
				}
				//외각벽
				if (z == zObjects - 1)
				{
					//가로 벽
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = Rowbb;

					xPosition = 5045.f;
					zPosition = x * wallSize + 200;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + wallSize * (0.5f + y);

					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());

					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);
					wallObjects.push_back(testWallObject);
				}
				//마그마방 입구기준 오른쪽벽
				if ((x != 1 && x != 5 && x != xObjects - 2) || y != 0)
				{
					//세로 벽ㄴ
					testWallObject = new CTestWallObject(1);
					testWallObject->objectBoundingBox = Rowbb;
					xPosition = 1170;
					zPosition = x * (wallSize)+200;
					yPosition = pTerrain->GetHeight(xPosition, zPosition) + 55 + (wallSize) * (0.5f + y);
					testWallObject->SetPosition(xPosition, yPosition, zPosition);
					xmmWorld = XMLoadFloat4x4(&testWallObject->GetWorldMatrix());
					testWallObject->objectBoundingBox.Transform(testWallObject->objectBoundingBox, xmmWorld);

					wallObjects.push_back(testWallObject);
				}
			}

		}
	}
	wallObjects.at(0)->SetMesh(0, pCubeRowMesh);
	wallObjects.at(0)->SetMaterial(pCubeMaterial);
}

D3D12_SHADER_BYTECODE CMonsterAttackShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "GSMonsterAttack", "gs_5_1", ppd3dShaderBlob));
}
