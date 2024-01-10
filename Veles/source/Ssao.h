#pragma once
#include "stdafx.h"
#include "Camera.h"

struct CB_SSAO
{
    Matrix4x4 Proj;
    Matrix4x4 InvProj;
    Matrix4x4 ProjTex;
    Vector4   OffsetVectors[14];

    // For SsaoBlur.hlsl
    Vector4 BlurWeights[3];

    Vector2 InvRenderTargetSize = { 0.0f, 0.0f };

    // Coordinates given in view space.
    float OcclusionRadius = 0.5f;
    float OcclusionFadeStart = 0.2f;
    float OcclusionFadeEnd = 2.0f;
    float SurfaceEpsilon = 0.05f;
};


class Ssao
{
public:

    Ssao(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Ssao(const Ssao& rhs) = delete;
    Ssao& operator=(const Ssao& rhs) = delete;
    ~Ssao() = default;

    static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
    static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    static const int MaxBlurRadius = 5;

    UINT SsaoMapWidth()const;
    UINT SsaoMapHeight()const;

    void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);
    std::vector<float> CalcGaussWeights(float sigma);


    ID3D12Resource* NormalMap();
    ID3D12Resource* AmbientMap();

    D3D12_CPU_DESCRIPTOR_HANDLE NormalMapRtv()const;
    D3D12_GPU_DESCRIPTOR_HANDLE NormalMapSrv()const;
    D3D12_GPU_DESCRIPTOR_HANDLE AmbientMapSrv()const;

    void BuildDescriptors(
        ID3D12Resource* depthStencilBuffer,
        D3D12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
        D3D12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
        D3D12_CPU_DESCRIPTOR_HANDLE hCpuRtv,
        UINT cbvSrvUavDescriptorSize,
        UINT rtvDescriptorSize);

    void RebuildDescriptors(ID3D12Resource* depthStencilBuffer);

    void SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso);

    void OnResize(UINT newWidth, UINT newHeight);


    void ComputeSsao(ID3D12GraphicsCommandList* cmdList, int blurCount);

    void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void UpdateShaderVariables(CCamera* mCamera);
    void ReleaseShaderVariables();

    D3D12_GPU_DESCRIPTOR_HANDLE Srv();
private:
    void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, int blurCount);
    void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, bool horzBlur);

    void BuildResources();
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);

    void BuildOffsetVectors();

    ID3D12RootSignature* CreateSsaoRootSignature(ID3D12Device* device);
    void CreateSsaoPipelineState(ID3D12Device* device);

    D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);

    //SSAO¸¦ À§ÇÑ 
    virtual D3D12_SHADER_BYTECODE CreateSsaoVertexShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_SHADER_BYTECODE CreateSsaoPixelShader(ID3DBlob** ppd3dShaderBlob);

    virtual D3D12_SHADER_BYTECODE CreateSsaoBlurVertexShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_SHADER_BYTECODE CreateSsaoBlurPixelShader(ID3DBlob** ppd3dShaderBlob);


private:
    ID3D12Device* mSsaoDevice;

    ID3D12PipelineState* mSsaoPso = nullptr;
    ID3D12PipelineState* mBlurPso = nullptr;

    ID3D12RootSignature* mSsaoRootSignature = nullptr;

    ComPtr<ID3D12Resource> mRandomVectorMap;
    ComPtr<ID3D12Resource> mRandomVectorMapUploadBuffer;
    ComPtr<ID3D12Resource> mNormalMap;
    ComPtr<ID3D12Resource> mAmbientMap0;
    ComPtr<ID3D12Resource> mAmbientMap1;

    D3D12_CPU_DESCRIPTOR_HANDLE mhNormalMapCpuSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE mhNormalMapGpuSrv;
    D3D12_CPU_DESCRIPTOR_HANDLE mhNormalMapCpuRtv;

    D3D12_CPU_DESCRIPTOR_HANDLE mhDepthMapCpuSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE mhDepthMapGpuSrv;

    D3D12_CPU_DESCRIPTOR_HANDLE mhRandomVectorMapCpuSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE mhRandomVectorMapGpuSrv;


    D3D12_CPU_DESCRIPTOR_HANDLE mhAmbientMap0CpuSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE mhAmbientMap0GpuSrv;
    D3D12_CPU_DESCRIPTOR_HANDLE mhAmbientMap0CpuRtv;

    D3D12_CPU_DESCRIPTOR_HANDLE mhAmbientMap1CpuSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE mhAmbientMap1GpuSrv;
    D3D12_CPU_DESCRIPTOR_HANDLE mhAmbientMap1CpuRtv;

    UINT mRenderTargetWidth;
    UINT mRenderTargetHeight;

    DirectX::XMFLOAT4 mOffsets[14];

    D3D12_VIEWPORT mViewport;
    D3D12_RECT mScissorRect;

    ComPtr<ID3D12Resource> mCbSsao = NULL;
    CB_SSAO* mCbMappedSsao = NULL;
};
