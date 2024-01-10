#include "stdafx.h"
#include "Ssao.h"

Ssao::Ssao(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    UINT width, UINT height)

{
    mSsaoDevice = device;

    OnResize(width, height);
    BuildOffsetVectors();
    BuildRandomVectorTexture(cmdList);
    mSsaoRootSignature = CreateSsaoRootSignature(device);
    CreateSsaoPipelineState(device);
}

UINT Ssao::SsaoMapWidth()const
{
    return mRenderTargetWidth / 2;
}

UINT Ssao::SsaoMapHeight()const
{
    return mRenderTargetHeight / 2;
}

void Ssao::GetOffsetVectors(DirectX::XMFLOAT4 offsets[14])
{
    std::copy(&mOffsets[0], &mOffsets[14], &offsets[0]);
}

std::vector<float> Ssao::CalcGaussWeights(float sigma)
{
    float twoSigma2 = 2.0f * sigma * sigma;

    int blurRadius = (int)ceil(2.0f * sigma);

    assert(blurRadius <= MaxBlurRadius);

    std::vector<float> weights;
    weights.resize(2 * blurRadius + 1);

    float weightSum = 0.0f;

    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float x = (float)i;

        weights[i + blurRadius] = expf(-x * x / twoSigma2);

        weightSum += weights[i + blurRadius];
    }

    // Divide by the sum so all the weights add up to 1.0.
    for (int i = 0; i < weights.size(); ++i)
    {
        weights[i] /= weightSum;
    }

    return weights;
}

ID3D12Resource* Ssao::NormalMap()
{
    return mNormalMap.Get();
}

ID3D12Resource* Ssao::AmbientMap()
{
    return mAmbientMap0.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Ssao::NormalMapRtv()const
{
    return mhNormalMapCpuRtv;
}

D3D12_GPU_DESCRIPTOR_HANDLE Ssao::NormalMapSrv()const
{
    return mhNormalMapGpuSrv;
}

D3D12_GPU_DESCRIPTOR_HANDLE Ssao::AmbientMapSrv()const
{
    return mhAmbientMap0GpuSrv;
}

void Ssao::BuildDescriptors(
    ID3D12Resource* depthStencilBuffer,
    D3D12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
    D3D12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
    D3D12_CPU_DESCRIPTOR_HANDLE hCpuRtv,
    UINT cbvSrvUavDescriptorSize,
    UINT rtvDescriptorSize)
{

    mhAmbientMap0CpuSrv = hCpuSrv;
    mhAmbientMap1CpuSrv.ptr = hCpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhNormalMapCpuSrv.ptr = mhAmbientMap1CpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhDepthMapCpuSrv.ptr = mhNormalMapCpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhRandomVectorMapCpuSrv.ptr = mhDepthMapCpuSrv.ptr + cbvSrvUavDescriptorSize;

    mhAmbientMap0GpuSrv = hGpuSrv;
    mhAmbientMap1GpuSrv.ptr = hGpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhNormalMapGpuSrv.ptr = mhAmbientMap1GpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhDepthMapGpuSrv.ptr = mhNormalMapGpuSrv.ptr + cbvSrvUavDescriptorSize;
    mhRandomVectorMapGpuSrv.ptr = mhDepthMapGpuSrv.ptr + cbvSrvUavDescriptorSize;

    mhNormalMapCpuRtv = hCpuRtv;
    mhAmbientMap0CpuRtv.ptr = mhNormalMapCpuRtv.ptr + rtvDescriptorSize;
    mhAmbientMap1CpuRtv.ptr = mhAmbientMap0CpuRtv.ptr + rtvDescriptorSize;

    RebuildDescriptors(depthStencilBuffer);
}

void Ssao::RebuildDescriptors(ID3D12Resource* depthStencilBuffer)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = NormalMapFormat;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    mSsaoDevice->CreateShaderResourceView(mNormalMap.Get(), &srvDesc, mhNormalMapCpuSrv);

    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    mSsaoDevice->CreateShaderResourceView(depthStencilBuffer, &srvDesc, mhDepthMapCpuSrv);

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    mSsaoDevice->CreateShaderResourceView(mRandomVectorMap.Get(), &srvDesc, mhRandomVectorMapCpuSrv);

    srvDesc.Format = AmbientMapFormat;
    mSsaoDevice->CreateShaderResourceView(mAmbientMap0.Get(), &srvDesc, mhAmbientMap0CpuSrv);
    mSsaoDevice->CreateShaderResourceView(mAmbientMap1.Get(), &srvDesc, mhAmbientMap1CpuSrv);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = NormalMapFormat;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    mSsaoDevice->CreateRenderTargetView(mNormalMap.Get(), &rtvDesc, mhNormalMapCpuRtv);

    rtvDesc.Format = AmbientMapFormat;
    mSsaoDevice->CreateRenderTargetView(mAmbientMap0.Get(), &rtvDesc, mhAmbientMap0CpuRtv);
    mSsaoDevice->CreateRenderTargetView(mAmbientMap1.Get(), &rtvDesc, mhAmbientMap1CpuRtv);
}

void Ssao::SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso)
{
    mSsaoPso = ssaoPso;
    mBlurPso = ssaoBlurPso;
}

void Ssao::OnResize(UINT newWidth, UINT newHeight)
{
    if (mRenderTargetWidth != newWidth || mRenderTargetHeight != newHeight)
    {
        mRenderTargetWidth = newWidth;
        mRenderTargetHeight = newHeight;

        // We render to ambient map at half the resolution.
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = mRenderTargetWidth / 2.0f;
        mViewport.Height = mRenderTargetHeight / 2.0f;
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        mScissorRect = { 0, 0, (int)mRenderTargetWidth / 2, (int)mRenderTargetHeight / 2 };

        BuildResources();
    }
}

void Ssao::ComputeSsao(ID3D12GraphicsCommandList* cmdList, int blurCount)
{
    cmdList->SetGraphicsRootSignature(mSsaoRootSignature);
    cmdList->RSSetViewports(1, &mViewport);
    cmdList->RSSetScissorRects(1, &mScissorRect);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mAmbientMap0.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    cmdList->ClearRenderTargetView(mhAmbientMap0CpuRtv, clearValue, 0, nullptr);

    cmdList->OMSetRenderTargets(1, &mhAmbientMap0CpuRtv, true, nullptr);

    auto ssaoCBAddress = mCbSsao.Get()->GetGPUVirtualAddress();
    cmdList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);
    cmdList->SetGraphicsRoot32BitConstant(1, 0, 0);

    cmdList->SetGraphicsRootDescriptorTable(2, mhNormalMapGpuSrv);

    cmdList->SetGraphicsRootDescriptorTable(3, mhRandomVectorMapGpuSrv);

    cmdList->SetPipelineState(mSsaoPso);

    cmdList->IASetVertexBuffers(0, 0, nullptr);
    cmdList->IASetIndexBuffer(nullptr);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(6, 1, 0, 0);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mAmbientMap0.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

    BlurAmbientMap(cmdList, blurCount);
}

void Ssao::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* cmdList)
{
    UINT ncbElementBytes = ((sizeof(CB_SSAO) + 255) & ~255); //256ÀÇ ¹è¼ö
    mCbSsao = ::CreateBufferResource(pd3dDevice, cmdList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

    mCbSsao->Map(0, NULL, (void**)&mCbMappedSsao);
}

void Ssao::UpdateShaderVariables(CCamera *mCamera)
{

    Matrix4x4 P = mCamera->GetProjectionMatrix();

    Matrix4x4 T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    mCbMappedSsao->Proj = P.transpose();
    mCbMappedSsao->InvProj = (P.inverse()).transpose();
    mCbMappedSsao->ProjTex = (P * T).transpose();

    GetOffsetVectors(mCbMappedSsao->OffsetVectors);

    auto blurWeights = CalcGaussWeights(2.5f);
    mCbMappedSsao->BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
    mCbMappedSsao->BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
    mCbMappedSsao->BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

    mCbMappedSsao->InvRenderTargetSize = XMFLOAT2(1.0f / SsaoMapWidth(), 1.0f / SsaoMapHeight());

    mCbMappedSsao->OcclusionRadius = 0.5f;
    mCbMappedSsao->OcclusionFadeStart = 0.2f;
    mCbMappedSsao->OcclusionFadeEnd = 1.0f;
    mCbMappedSsao->SurfaceEpsilon = 0.05f;
}

void Ssao::ReleaseShaderVariables()
{
}

D3D12_GPU_DESCRIPTOR_HANDLE Ssao::Srv()
{
    return mhAmbientMap0GpuSrv;
}

void Ssao::BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, int blurCount)
{
    cmdList->SetPipelineState(mBlurPso);

    auto ssaoCBAddress = mCbSsao.Get()->GetGPUVirtualAddress();
    cmdList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);

    for (int i = 0; i < blurCount; ++i)
    {
        BlurAmbientMap(cmdList, true);
        BlurAmbientMap(cmdList, false);
    }
}

void Ssao::BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, bool horzBlur)
{
    ID3D12Resource* output = nullptr;
    CD3DX12_GPU_DESCRIPTOR_HANDLE inputSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE outputRtv;

    if (horzBlur == true)
    {
        output = mAmbientMap1.Get();
        inputSrv = mhAmbientMap0GpuSrv;
        outputRtv = mhAmbientMap1CpuRtv;
        cmdList->SetGraphicsRoot32BitConstant(1, 1, 0);
    }
    else
    {
        output = mAmbientMap0.Get();
        inputSrv = mhAmbientMap1GpuSrv;
        outputRtv = mhAmbientMap0CpuRtv;
        cmdList->SetGraphicsRoot32BitConstant(1, 0, 0);
    }

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(output,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    cmdList->ClearRenderTargetView(outputRtv, clearValue, 0, nullptr);

    cmdList->OMSetRenderTargets(1, &outputRtv, true, nullptr);

    cmdList->SetGraphicsRootDescriptorTable(2, mhNormalMapGpuSrv);

    cmdList->SetGraphicsRootDescriptorTable(3, inputSrv);

    cmdList->IASetVertexBuffers(0, 0, nullptr);
    cmdList->IASetIndexBuffer(nullptr);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(6, 1, 0, 0);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(output,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void Ssao::BuildResources()
{
    mNormalMap = nullptr;
    mAmbientMap0 = nullptr;
    mAmbientMap1 = nullptr;

    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mRenderTargetWidth;
    texDesc.Height = mRenderTargetHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = Ssao::NormalMapFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;


    float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    CD3DX12_CLEAR_VALUE optClear(NormalMapFormat, normalClearColor);
    mSsaoDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&mNormalMap));

    texDesc.Width = mRenderTargetWidth / 2;
    texDesc.Height = mRenderTargetHeight / 2;
    texDesc.Format = Ssao::AmbientMapFormat;

    float ambientClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    optClear = CD3DX12_CLEAR_VALUE(AmbientMapFormat, ambientClearColor);

    mSsaoDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&mAmbientMap0));

   mSsaoDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&mAmbientMap1));
}

void Ssao::BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList)
{
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = 256;
    texDesc.Height = 256;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    mSsaoDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mRandomVectorMap));

    const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mRandomVectorMap.Get(), 0, num2DSubresources);

    mSsaoDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(mRandomVectorMapUploadBuffer.GetAddressOf()));
    vector<XMCOLOR> initData;
    initData.reserve(256 * 256);
    for (int i = 0; i < 256; ++i)
    {
        for (int j = 0; j < 256; ++j)
        {
            XMFLOAT3 v(Mathf::RandF(), Mathf::RandF(), Mathf::RandF());
   
            initData.push_back(XMCOLOR (v.x, v.y, v.z, 0.0f));
        }
    }

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData.data();
    subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
    subResourceData.SlicePitch = subResourceData.RowPitch * 256;

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRandomVectorMap.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources(cmdList, mRandomVectorMap.Get(), mRandomVectorMapUploadBuffer.Get(),
        0, 0, num2DSubresources, &subResourceData); 
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRandomVectorMap.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void Ssao::BuildOffsetVectors()
{
    mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
    mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

    mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
    mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

    mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
    mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

    mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
    mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

    mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
    mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

    mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
    mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

    mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
    mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

    for (int i = 0; i < 14; ++i)
    {
        // Create random lengths in [0.25, 1.0].
        float s = Mathf::RandF(0.25f, 1.0f);

        XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));

        XMStoreFloat4(&mOffsets[i], v);
    }
}

void Ssao::CreateSsaoPipelineState(ID3D12Device* pd3dDevice)
{
    ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPipelineStateDesc;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPipelineStateDesc;

    ::ZeroMemory(&ssaoPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    ssaoPipelineStateDesc.pRootSignature = mSsaoRootSignature;
    ssaoPipelineStateDesc.VS = CreateSsaoVertexShader(&pd3dVertexShaderBlob);
    ssaoPipelineStateDesc.PS = CreateSsaoPixelShader(&pd3dPixelShaderBlob);
    ssaoPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    ssaoPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    ssaoPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    ssaoPipelineStateDesc.DepthStencilState.DepthEnable = false;
    ssaoPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    ssaoPipelineStateDesc.InputLayout = { nullptr, 0 };
    ssaoPipelineStateDesc.SampleMask = UINT_MAX;
    ssaoPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    ssaoPipelineStateDesc.NumRenderTargets = 1;
    ssaoPipelineStateDesc.RTVFormats[0] = AmbientMapFormat;
    ssaoPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ssaoPipelineStateDesc.SampleDesc.Count = 1;
    ssaoPipelineStateDesc.SampleDesc.Quality = 0;
    ssaoPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&ssaoPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&mSsaoPso);
    ssaoBlurPipelineStateDesc = ssaoPipelineStateDesc;
    ssaoBlurPipelineStateDesc.VS = CreateSsaoVertexShader(&pd3dVertexShaderBlob);
    ssaoBlurPipelineStateDesc.PS = CreateSsaoPixelShader(&pd3dPixelShaderBlob);
    hResult = pd3dDevice->CreateGraphicsPipelineState(&ssaoBlurPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&mBlurPso);
    if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
    if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
}

ID3D12RootSignature* Ssao::CreateSsaoRootSignature(ID3D12Device* pd3dDevice)
{
    ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

    D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

    pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[0].NumDescriptors = 2;
    pd3dDescriptorRanges[0].BaseShaderRegister = 0;
    pd3dDescriptorRanges[0].RegisterSpace = 0;
    pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[1].NumDescriptors = 1;
    pd3dDescriptorRanges[1].BaseShaderRegister = 2;
    pd3dDescriptorRanges[1].RegisterSpace = 0;
    pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER pd3dRootParameters[4];

    pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[0].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    pd3dRootParameters[1].Constants.Num32BitValues = 1;
    pd3dRootParameters[1].Constants.ShaderRegister = 1;
    pd3dRootParameters[1].Constants.RegisterSpace = 0;
    pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
    pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
    pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

    pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[0].MipLODBias = 0;
    pd3dSamplerDescs[0].MaxAnisotropy = 16;
    pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    pd3dSamplerDescs[0].MinLOD = 0;
    pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
    pd3dSamplerDescs[0].ShaderRegister = 0;
    pd3dSamplerDescs[0].RegisterSpace = 0;
    pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pd3dSamplerDescs[1].MipLODBias = 0;
    pd3dSamplerDescs[1].MaxAnisotropy = 16;
    pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    pd3dSamplerDescs[1].MinLOD = 0;
    pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
    pd3dSamplerDescs[1].ShaderRegister = 1;
    pd3dSamplerDescs[1].RegisterSpace = 0;
    pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dSamplerDescs[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    pd3dSamplerDescs[2].MipLODBias = 0;
    pd3dSamplerDescs[2].MaxAnisotropy = 0;
    pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    pd3dSamplerDescs[2].MinLOD = 0;
    pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
    pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    pd3dSamplerDescs[2].ShaderRegister = 2;
    pd3dSamplerDescs[2].RegisterSpace = 0;
    pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    pd3dSamplerDescs[3].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pd3dSamplerDescs[3].MipLODBias = 0;
    pd3dSamplerDescs[3].MaxAnisotropy = 16;
    pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    pd3dSamplerDescs[3].MinLOD = 0;
    pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
    pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    pd3dSamplerDescs[3].ShaderRegister = 3;
    pd3dSamplerDescs[3].RegisterSpace = 0;
    pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    ::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
    d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
    d3dRootSignatureDesc.pParameters = pd3dRootParameters;
    d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
    d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
    d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

    ID3DBlob* pd3dSignatureBlob = NULL;
    ID3DBlob* pd3dErrorBlob = NULL;
    D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
    pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
    if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
    if (pd3dErrorBlob) pd3dErrorBlob->Release();

    return(pd3dGraphicsRootSignature);
}

D3D12_SHADER_BYTECODE Ssao::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob* pd3dErrorBlob = NULL;
    ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
    char* pErrorString = NULL;
    if (pd3dErrorBlob)
    {
        pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
    }

    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
    d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

    return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE Ssao::CreateSsaoVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CompileShaderFromFile(L"Ssao.hlsl", "VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Ssao::CreateSsaoPixelShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CompileShaderFromFile(L"Ssao.hlsl", "PS", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Ssao::CreateSsaoBlurVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CompileShaderFromFile(L"SsaoBlur.hlsl", "VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Ssao::CreateSsaoBlurPixelShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CompileShaderFromFile(L"SsaoBlur.hlsl", "PS", "ps_5_1", ppd3dShaderBlob));
}

