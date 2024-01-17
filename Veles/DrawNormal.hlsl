#include "Shaders.hlsl"

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
    uint materialNum : MATERIAL;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    uint materialNum : MATERIAL;
};

VertexOut DrawNormalVS(VertexIn input)
{
    VertexOut output;
    float3 positionB = float3(0.0f, 0.0f, 0.0f);
    float3 normalB = float3(0.0f, 0.0f, 0.0f);
    float3 tangentB = float3(0.0f, 0.0f, 0.0f);
    float3 bitangentB = float3(0.0f, 0.0f, 0.0f);

    if (input.weights.x != 0.0f)
    {
        positionB += input.weights.x * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        positionB += input.weights.y * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        positionB += input.weights.z * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        positionB += input.weights.w * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        output.position = mul(mul(mul(float4(positionB, 1.0f), gmtxWorld), gmtxView), gmtxProjection);

        normalB += input.weights.x * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        normalB += input.weights.y * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        normalB += input.weights.z * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        normalB += input.weights.w * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        output.normalW = (float3) mul(float4(normalB, 1.0), gmtxWorld);

        tangentB += input.weights.x * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        tangentB += input.weights.y * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        tangentB += input.weights.z * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        tangentB += input.weights.w * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        output.tangentW = (float3) mul(float4(tangentB, 1.0), gmtxWorld);

        bitangentB += input.weights.x * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        bitangentB += input.weights.y * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        bitangentB += input.weights.z * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        bitangentB += input.weights.w * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        output.bitangentW = (float3) mul(float4(bitangentB, 1.0), gmtxWorld);
    }
    else
    {
        output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
        output.normalW = mul(input.normal, (float3x3) gmtxWorld);
        output.tangentW = (float3) mul(float4(input.tangent, 1.0f), gmtxWorld);
        output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), gmtxWorld);
    }

    output.uv = input.uv;
    output.materialNum = input.materialNum;
    return (output);
}

VertexOut DrawNormalVSInstance(VertexIn input, uint nInstanceID : SV_InstanceID)
{
    VertexOut output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject), gmtxShadowView), gmtxShadowProjection);
    output.normalW = mul(input.normal, (float3x3)  gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.tangentW = (float3) mul(float4(input.tangent, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.uv = input.uv;
    output.materialNum = input.materialNum;
    return (output);
}

float4 DrawNormalPS(VertexOut input) : SV_TARGET
{
    float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
    float4 cNormal = gtxtTexture[input.materialNum * 2 + 1].SampleLevel(gWrapSamplerState, input.uv, 0);
    float3 vNormal = normalize(cNormal.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
    float3 normalW = normalize(mul(vNormal, TBN));
    
    float3 normalV = mul(normalW, (float3x3) gmtxView);
    return float4(normalV, 0.0f);
}