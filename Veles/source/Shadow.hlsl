#include "Shaders.hlsl"

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexOut VS(VertexIn input)
{
    VertexOut output;
    float3 positionB = float3(0.0f, 0.0f, 0.0f);

    if (input.weights.x != 0.0f)
    {
        positionB += input.weights.x * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        positionB += input.weights.y * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        positionB += input.weights.z * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        positionB += input.weights.w * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        output.position = mul(mul(mul(float4(positionB, 1.0f), gmtxWorld), gmtxShadowView), gmtxShadowProjection);

    }
    else
    {
        output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxShadowView), gmtxShadowProjection);
    }

    output.uv = input.uv;

    return (output);
}

VertexOut ShadowVSInstance(VertexIn input, uint nInstanceID : SV_InstanceID)
{
    VertexOut output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject), gmtxShadowView), gmtxShadowProjection);
    output.uv = input.uv;

    return (output);
}

void PS(VertexOut pin)
{

}