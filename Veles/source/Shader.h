//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"

class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	//±âº» ·»´õ¸µ
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	//½¦µµ¿ì¸Ê ·»´õ¸µ
	virtual D3D12_SHADER_BYTECODE CreateShadowVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateShadowPixelShader(ID3DBlob** ppd3dShaderBlob);
	//SSAO¸¦ À§ÇÑ ³ë¸», µª½ºÀúÀå
	virtual D3D12_SHADER_BYTECODE CreateDrawNormalVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateDrawNormalPixelShader(ID3DBlob** ppd3dShaderBlob);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);

	void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void CreateConstantBufferViews(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffers, UINT nStride, int nObject);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, Matrix4x4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext=NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObject() { }

	virtual void ReleaseUploadBuffers();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(cbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(cbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(cbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(srvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(srvGPUDescriptorStartHandle); }

	//virtual CGameObject* ColideObjectByRayIntersection(Vector3& position, Vector3& direction, float* distance) { return nullptr; };
	virtual void PrintBoundingBox() {};
	map<RENDER_TYPE, ID3D12PipelineState*> GetPso() { return pipelineStates; }
	void SetPso(map<RENDER_TYPE, ID3D12PipelineState*> pso) { pipelineStates = pso; }
protected:
	map<RENDER_TYPE, ID3D12PipelineState*>	pipelineStates;

	ID3D12DescriptorHeap*					cbvSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE				cbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				cbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE				srvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				srvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE				srvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				srvGPUDescriptorNextHandle;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMonsterShader : public CShader
{
public:
	CMonsterShader();
	virtual ~CMonsterShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};

class CChestShader : public CShader
{
public:
	CChestShader();
	virtual ~CChestShader() {};
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTexturedShader : public CShader
{
public:
	CTexturedShader();
	virtual ~CTexturedShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_BATCH_MATERIAL

class CTestMapShader : public CTexturedShader
{
public:
	CTestMapShader();
    virtual ~CTestMapShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext=NULL);
	virtual void ReleaseObject();
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void PrintBoundingBox();

protected:
	vector<CGameObject*>			wallObjects;
	int								numObjects;
	
	ID3D12Resource*					cbGameObjects = NULL;
	CB_GAMEOBJECT_INFO*				cbMappedGameObjects = NULL;
	
	CMaterial*						mapMaterial = NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainShader : public CTexturedShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};

class CSkinnedAnimationShader : public CShader
{
public:
	CSkinnedAnimationShader();
	virtual ~CSkinnedAnimationShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};


class CBoundingBoxShader : public CShader
{
public:
	CBoundingBoxShader();
	virtual ~CBoundingBoxShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
};

class CEffectShader : public CShader
{
public:
	CEffectShader();
	virtual ~CEffectShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
};

class CHpShader : public CEffectShader
{
public:
	CHpShader();
	virtual ~CHpShader();

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
};


class CUiShader : public CEffectShader
{
public:
	CUiShader();
	virtual ~CUiShader();

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
};

class CFireEffectShader : public CEffectShader
{
public:
	CFireEffectShader() {}
	virtual ~CFireEffectShader() {}

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
};

class CMonsterAttackShader : public CEffectShader
{
public:
	CMonsterAttackShader() {}
	virtual ~CMonsterAttackShader() {}

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
};

class LavaShader : public CShader
{
public:
	LavaShader();
	virtual ~LavaShader();
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};

class ItemShader : public CEffectShader
{
public:
	ItemShader() {}
	virtual ~ItemShader() {}
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};

class PanelShader : public CEffectShader
{
public:
	PanelShader() {}
	virtual ~PanelShader() {}
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
};

class BoneShader : public CShader
{
	BoneShader();
	virtual ~BoneShader();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
};

struct CB_INSTANCE
{
	Matrix4x4 world;
};

class InstancingShader : public CShader
{
public:
	InstancingShader();
	virtual ~InstancingShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateShadowVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateDrawNormalVertexShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
	virtual void ReleaseObject();
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
protected:
	vector<CGameObject*>			wallObjects;
	int								numObjects;

	ID3D12Resource* cbGameObjects = NULL;
	CB_INSTANCE* cbMappedInstance = NULL;

	CMaterial* mapMaterial = NULL;
};

class InstancingRowShader : public InstancingShader
{
public:
	InstancingRowShader() {};
	virtual ~InstancingRowShader() {};

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
};