//#define _WITH_CONSTANT_BUFFER_SYNTAX

//각 vertex에 영향을 주는 bone의 개수의 최대치
//최대 4개까지만 읽도록 설정
#define MAX_VERTEX_INFLUENCES				4

// bone을 최대 128개 가지고 있다고 예상하고 설정.
// 더 많으면 늘리면 됨
#define SKINNED_ANIMATION_BONES				128

#define MAX_SHADOWMAPS 4

cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
    matrix      gmtxViewProjTex : packoffset(c8);
	float3		gvCameraPosition : packoffset(c12);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxWorld : packoffset(c0);
    uint		gnMaterial : packoffset(c4);
};

cbuffer cbShadowCameraInfo : register(b6)
{
    matrix gmtxShadowView : packoffset(c0);
    matrix gmtxShadowProjection : packoffset(c4);
};

cbuffer cbBoneTransforms : register(b7)
{
    matrix gpmtxBoneFinalTransforms[SKINNED_ANIMATION_BONES];
}

cbuffer cbEffectInfo : register(b8)
{
    matrix gWorld;
    uint gFrame;
    float3 gPosition;
    float gTime;
}

cbuffer cbDrawType : register(b9)
{
    bool drawFog;
    float gFogStart;
}

cbuffer cbShadowMapsInfo : register(b10)
{
    matrix shadowViews[MAX_SHADOWMAPS];
    matrix shadowProjs[MAX_SHADOWMAPS];
}

cbuffer cbWaveInfo : register(b11)
{
    matrix gTextureAnimation;
    float2 gDisplacementMapTexelSize;
    float gGridSpatialStep;
}

static const matrix gmtxProjectToTexture =
{
    0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f
};

static const float4 gFogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
static const float gFogRange = 300.0f;

struct INSTANCEDGAMEOBJECTINFO
{
    matrix m_mtxGameObject;
};

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gGameObjectInfos : register(t0);

Texture2D gDisplacementMap : register(t4);
Texture2D gtxtTerrainBaseTexture : register(t5);
Texture2D gtxtTerrainDetailTexture : register(t6);
Texture2D gtxtTerrainDetailTexture2 : register(t7);
TextureCube gtxtSkyBox : register(t8);
Texture2D gtxtSsaoMap : register(t9);
Texture2D<float> gtxtShadowMap[MAX_SHADOWMAPS] : register(t10);
Texture2D gtxtTexture[10] : register(t14);


SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);
SamplerComparisonState gShadowSamplerState : register(s2);
SamplerState gsamAnisotropicState : register(s3);
SamplerState gPointClampState : register(s4);


#include "Light.hlsl"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

float CalculateShadowFactor(float3 pos, int index)
{
    float4 shadowPosition;
    float depth;
    uint width, height, numMips;
    float percentLit = 0.0f, fBias = 0.005f;

    percentLit = 0.0f;
    shadowPosition = mul(mul(mul(float4(pos, 1.0f), shadowViews[index]), shadowProjs[index]), gmtxProjectToTexture);
    shadowPosition.xyz /= shadowPosition.w;
    depth = shadowPosition.z;
    gtxtShadowMap[index].GetDimensions(0, width, height, numMips);

    float dx = 1.0f / (float) width;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    for (int i = 0; i < 9; i++)
    {
        float fsDepth = gtxtShadowMap[index].SampleCmpLevelZero(gShadowSamplerState, shadowPosition.xy + offsets[i], depth).r;
        if (shadowPosition.z - fBias < fsDepth)
            percentLit += 1.0f;
    }
    percentLit /= 9;
    
    return percentLit;              
}

struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.normal = input.normal;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(float4(input.normal,0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
	output.normal = input.normal;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
    return(float4(input.normal,0));
}


struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
    float4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
    uint materialNum : MATERIAL;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
    float4 ssaoPosition : POSITION;
	float2 uv : TEXCOORD;
    uint materialNum : MATERIAL;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
    float3 positionB = float3(0.0f, 0.0f, 0.0f);
    float3 positionW = float3(0.0f, 0.0f, 0.0f);
  
    if (input.weights.x != 0.0f)
    {
        positionB += input.weights.x * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]);
        positionB += input.weights.y * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]);
        positionB += input.weights.z * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]);
        positionB += input.weights.w * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]);
        positionW = (float3)mul(float4(positionB, 1.0f), gmtxWorld);
    }
    else
    {
        positionW = (float3)(mul(float4(input.position, 1.0f), gmtxWorld));
    }

    output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
    output.ssaoPosition = mul(mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection), gmtxProjectToTexture);
	output.uv = input.uv;
    output.materialNum = input.materialNum;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    float4 cTexture = gtxtTexture[input.materialNum * 2].SampleLevel(gWrapSamplerState, input.uv, 0);
    input.ssaoPosition /= input.ssaoPosition.w;
    float diffuse = gtxtTexture[input.materialNum * 2].Sample(gsamAnisotropicState, input.uv);
    float ambientAccess = gtxtSsaoMap.Sample(gClampSamplerState, input.ssaoPosition.xy, 0.0f).r;
    
    float4 ambient = ambientAccess * gcGlobalAmbientLight * cTexture;
    
    float4 cColor = ambient;
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;


    output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxWorld);
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);

    output.normalW = mul(input.normal, (float3x3) gmtxWorld);
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{

	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gClampSamplerState, input.uv0);

    float4 cDetailTexColor = gtxtTerrainDetailTexture.SampleLevel(gWrapSamplerState, input.uv1, 2);
    float fAlpha = gtxtTerrainDetailTexture2.Sample(gWrapSamplerState, input.uv0);

    float4 cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha))*1.5;
    float4 ambient = gcGlobalAmbientLight * cColor;
    input.normalW = normalize(input.normalW);
    float4 shadowPosition;
    float depth;
    uint width, height, numMips;
    float shadowFactor[MAX_SHADOWMAPS];
    float percentLit = 0.0f, fBias = 0.005f;
    for (int j = 0; j < MAX_SHADOWMAPS; j++)
    {
        shadowFactor[j] = CalculateShadowFactor(input.positionW, j);
    }

    float4 cIllumination = LightingShadow(input.positionW, input.normalW, shadowFactor);
    
    float3 toEyeW = gvCameraPosition - input.positionW;
    float distToEye = length(toEyeW);
    
    float4 fColor = (lerp(ambient, cIllumination, 0.4));
    
    if (isGrayScale)
    {
        float gray = (fColor.r + fColor.g + fColor.b) / 3;
        fColor.rgb = float3(gray, gray, gray);
    }
    
    if (drawFog == 1)
    {
        float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
        return (lerp(fColor, gFogColor, fogAmount));
    }
    else
    {
        return fColor;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyBox.Sample(gClampSamplerState, input.positionL);
    if (isGrayScale)
    {
        float gray = (cColor.r + cColor.g + cColor.b) / 3;
        cColor.rgb = float3(gray, gray, gray);
    }
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD0;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

    output.normalW = mul(input.normal, (float3x3) gmtxWorld);
    output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxWorld);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return (output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    float4 cTexture = gtxtTexture[0].SampleLevel(gWrapSamplerState, input.uv, 0);
    input.normalW = normalize(input.normalW);

	
    float4 shadowPosition;
    float depth;
    uint width, height, numMips;
    float shadowFactor[MAX_SHADOWMAPS];
    float percentLit = 0.0f, fBias = 0.005f;
    for (int j = 0; j < MAX_SHADOWMAPS; j++)
    {
        shadowFactor[j] = CalculateShadowFactor(input.positionW, j);
    }
    float4 cIllumination = LightingShadow(input.positionW, input.normalW, shadowFactor);

    float3 toEyeW = gvCameraPosition - input.positionW;
    float distToEye = length(toEyeW);
    
    float4 fColor = (lerp(cTexture, cIllumination, 0.4));
    
    if (isGrayScale)
    {
        float gray = (fColor.r + fColor.g + fColor.b) / 3;
        fColor.rgb = float3(gray, gray, gray);
    }
    
    if (drawFog == 1)
    {
        float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
        return (lerp(fColor, gFogColor, fogAmount));
    }
    else
    {
        return fColor;
    }
}

VS_TEXTURED_LIGHTING_OUTPUT VSLavaWave(VS_TEXTURED_LIGHTING_INPUT input)
{
    VS_TEXTURED_LIGHTING_OUTPUT output;
    input.position.y += gDisplacementMap.SampleLevel(gWrapSamplerState, input.uv, 1.0f).r;

    float du = gDisplacementMapTexelSize.x;
    float dv = gDisplacementMapTexelSize.y;
    float l = gDisplacementMap.SampleLevel(gPointClampState, input.uv - float2(du, 0.0f), 0.0f).r;
    float r = gDisplacementMap.SampleLevel(gPointClampState, input.uv + float2(du, 0.0f), 0.0f).r;
    float t = gDisplacementMap.SampleLevel(gPointClampState, input.uv - float2(0.0f, dv), 0.0f).r;
    float b = gDisplacementMap.SampleLevel(gPointClampState, input.uv + float2(0.0f, dv), 0.0f).r;
    input.normal = normalize(float3(-r + l, 2.0f * gGridSpatialStep, b - t));
    
    output.positionW = (float3) mul(float4(input.position, 1.0), gmtxWorld);
    output.normalW = mul(input.normal, (float3x3) gmtxWorld);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    return (output);
}

float4 PSLavaWave(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    float2 uv = input.uv;

    uv = mul(float3(input.uv, 1.0f), (float3x3) gTextureAnimation).xy;
    
    float4 cTexture = gtxtTexture[0].SampleLevel(gWrapSamplerState, uv, 0);
    input.normalW = normalize(input.normalW);

    float4 ambient = gcGlobalAmbientLight * cTexture * 2;
 
    float shadowFactor[MAX_SHADOWMAPS];
    for (int i = 0; i < MAX_SHADOWMAPS; i++)
    {
        shadowFactor[i] = 1.f;
    }
    
    float4 cIllumination = LightingShadow(input.positionW, input.normalW, shadowFactor);

    float3 toEyeW = gvCameraPosition - input.positionW;
    float distToEye = length(toEyeW);
    
    float4 fColor = (lerp(ambient, cIllumination, 0.3));
    fColor = fColor * (1 + 0.8 * smoothstep(0.8, 1, 2 * length(fColor.xyz)));
    
    if (isGrayScale)
    {
        float gray = (fColor.r + fColor.g + fColor.b) / 3;
        fColor.rgb = float3(gray, gray, gray);
    }
    
    if (drawFog == 1)
    {
        float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
        return (lerp(fColor, gFogColor, fogAmount));
    }
    else
    {
        return fColor;
    }
}

struct VS_NORMALMAP_TEXTURED_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint materialNum : MATERIAL;
};

struct VS_NORMALMAP_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION0;
    float4 ssaoPosition : POSITION1;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD0;
    uint materialNum : MATERIAL;
};


VS_NORMALMAP_TEXTURED_OUTPUT VSTexturedNormalMapLighting(VS_NORMALMAP_TEXTURED_INPUT input)
{
    VS_NORMALMAP_TEXTURED_OUTPUT output;

    output.positionW = (float3) mul(float4(input.position, 1.0), gmtxWorld);
    output.normalW = mul(input.normal, (float3x3) gmtxWorld);
    output.tangentW = (float3) mul(float4(input.tangent, 1.0f), gmtxWorld);
    output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), gmtxWorld);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    output.materialNum = input.materialNum;
    output.ssaoPosition = mul(float4(output.positionW, 1.0f), gmtxViewProjTex);
    return (output);
}

float4 PSTexturedNormalMapLighting(VS_NORMALMAP_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    float4 cTexture = gtxtTexture[input.materialNum * 2].SampleLevel(gWrapSamplerState, input.uv, 0);
    float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
    float4 cNormal = gtxtTexture[input.materialNum * 2 + 1].SampleLevel(gWrapSamplerState, input.uv, 0);
    float3 vNormal = normalize(cNormal.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
    float3 normalW = normalize(mul(vNormal, TBN));
    
    input.ssaoPosition /= input.ssaoPosition.w;
    float diffuse = gtxtTexture[input.materialNum * 2].Sample(gsamAnisotropicState, input.uv);
    float ambientAccess = gtxtSsaoMap.Sample(gClampSamplerState, input.ssaoPosition.xy, 0.0f).r;
    
    float4 ambient = gcGlobalAmbientLight * cTexture;
    
    float4 shadowPosition;
    float depth;
    uint width, height, numMips;
    float shadowFactor[MAX_SHADOWMAPS];
    float percentLit = 0.0f, fBias = 0.005f;
    for (int j = 0; j < MAX_SHADOWMAPS; j++)
    {
        shadowFactor[j] = CalculateShadowFactor(input.positionW, j);
    }
    
    float4 cIllumination = LightingShadow(input.positionW, normalW, shadowFactor);

    float3 toEyeW = gvCameraPosition - input.positionW;
    float distToEye = length(toEyeW);
    
    float4 fColor = (lerp(ambient, cIllumination, 0.4));
    fColor.a = cTexture.a;
    
    if (isGrayScale)
    {
        float gray = (fColor.r + fColor.g + fColor.b) / 3;
        fColor.rgb = float3(gray, gray, gray);
    }
        
    if (drawFog == 1)
    {
        float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
        return (lerp(fColor, gFogColor, fogAmount));
    }
    else
    {
        return fColor;
    }
}

VS_NORMALMAP_TEXTURED_OUTPUT VSInstance(VS_NORMALMAP_TEXTURED_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_NORMALMAP_TEXTURED_OUTPUT output;

    output.positionW = (float3) mul(float4(input.position, 1.0), gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.normalW = mul(input.normal, (float3x3) gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.tangentW = (float3) mul(float4(input.tangent, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    output.materialNum = input.materialNum;
    output.ssaoPosition = mul(float4(output.positionW, 1.0f), gmtxViewProjTex);
    return (output);
}

//카메라 옮겨서 테스트
struct VS_SKINNED_NORMALMAP_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
    float4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
    uint materialNum : MATERIAL;
};

struct VS_SKINNED_NORMALMAP_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION0;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float4 ssaoPosition : POSITION1;
    float2 uv : TEXCOORD0;
    uint materialNum : MATERIAL;
};

VS_SKINNED_NORMALMAP_TEXTURED_OUTPUT VSSkinnedTexturedNormalMapLighting(VS_SKINNED_NORMALMAP_TEXTURED_INPUT input)
{
	VS_SKINNED_NORMALMAP_TEXTURED_OUTPUT output;

	float3 positionB = float3(0.0f, 0.0f, 0.0f);
	float3 normalB = float3(0.0f, 0.0f, 0.0f);
	float3 tangentB = float3(0.0f, 0.0f, 0.0f);
	float3 bitangentB = float3(0.0f, 0.0f, 0.0f);

	if (input.weights.x != 0.0f)
	{
		positionB += input.weights.x * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]).xyz;
		positionB += input.weights.y * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]).xyz;
		positionB += input.weights.z * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]).xyz;
		positionB += input.weights.w * mul(float4(input.position, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]).xyz;
		output.positionW = mul(float4(positionB, 1.0), gmtxWorld).xyz;

		normalB += input.weights.x * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]).xyz;
		normalB += input.weights.y * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]).xyz;
		normalB += input.weights.z * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]).xyz;
		normalB += input.weights.w * mul(float4(input.normal, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]).xyz;
		output.normalW = mul(normalB, (float3x3) gmtxWorld).xyz;

		tangentB += input.weights.x * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]).xyz;
		tangentB += input.weights.y * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]).xyz;
		tangentB += input.weights.z * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]).xyz;
		tangentB += input.weights.w * mul(float4(input.tangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]).xyz;
		output.tangentW = (float3) mul(float4(tangentB, 1.0), gmtxWorld).xyz;

		bitangentB += input.weights.x * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.x]).xyz;
		bitangentB += input.weights.y * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.y]).xyz;
		bitangentB += input.weights.z * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.z]).xyz;
		bitangentB += input.weights.w * mul(float4(input.bitangent, 1.0f), gpmtxBoneFinalTransforms[input.indices.w]).xyz;
		output.bitangentW = (float3) mul(float4(bitangentB, 1.0), gmtxWorld).xyz;
	}

	else
	{
		output.positionW = (float3) mul(float4(input.position, 1.0), gmtxWorld);
		output.normalW = mul(input.normal, (float3x3) gmtxWorld);
		output.tangentW = (float3) mul(float4(input.tangent, 1.0f), gmtxWorld);
		output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), gmtxWorld);
	}
    output.ssaoPosition = mul(float4(output.positionW, 1.0f), gmtxViewProjTex);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
    output.materialNum = input.materialNum;
	return (output);
}

float4 PSSkinnedTexturedNormalMapLighting(VS_SKINNED_NORMALMAP_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    float4 cTexture = gtxtTexture[input.materialNum * 2].SampleLevel(gWrapSamplerState, input.uv, 0);
    float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
    float4 cNormal = gtxtTexture[input.materialNum * 2 + 1].SampleLevel(gWrapSamplerState, input.uv, 0);
    float3 vNormal = normalize(cNormal.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
    float3 normalW = normalize(mul(vNormal, TBN));

    input.ssaoPosition /= input.ssaoPosition.w;
    float diffuse = gtxtTexture[input.materialNum * 2].Sample(gsamAnisotropicState, input.uv);
    float ambientAccess = gtxtSsaoMap.Sample(gClampSamplerState, input.ssaoPosition.xy, 0.0f).r;
    
    float4 ambient;
    if (gnMaterial == 2)
        ambient = gcGlobalAmbientLight * cTexture;
    else
        ambient = ambientAccess * gcGlobalAmbientLight * cTexture;
    
    float4 shadowPosition;
    float depth;
    uint width, height, numMips;
    float shadowFactor[MAX_SHADOWMAPS];
    float percentLit = 0.0f, fBias = 0.005f;
    for (int j = 0; j < MAX_SHADOWMAPS; j++)
    {
        shadowFactor[j] = CalculateShadowFactor(input.positionW, j);
    }

    float4 fLight = LightingShadow(input.positionW, normalW, shadowFactor);
    
    float3 toEyeW = gvCameraPosition - input.positionW;
    float distToEye = length(toEyeW);
    float4 fColor = (lerp(ambient, fLight, 0.4));
    fColor.a = cTexture.a;
    
    if (isGrayScale)
    {
        float gray = (fColor.r + fColor.g + fColor.b) / 3;
        fColor.rgb = float3(gray, gray, gray);
    }
    
    if (drawFog == 1)
    {
        float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
        return (lerp(fColor, gFogColor, fogAmount));
    }
    else
    {
        return fColor;
    }
}

struct VS_BILLBOARD_INPUT
{
    float3 center : POSITION;
    float2 size : TEXCOORD;
    uint width : WIDTH;
    uint height : HEIGHT;
    float randomHeight : RANDHEIGHT;
    float randomDir : RANDDIR;
};

VS_BILLBOARD_INPUT VS2dTexture(VS_BILLBOARD_INPUT input)
{
    return (input);
}

struct GS_BILLBOARD_GEOMETRY_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
    uint nTexture : TEXTURE;
};

static float2 pf2UVs[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

[maxvertexcount(4)]
void GSEffect(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(gmtxView._12, gmtxView._22, gmtxView._32);

    input[0].center = (float3) mul(float4(input[0].center, 1.0), gWorld); /*float3(0, 0, 0);*/
    float3 f3Look = normalize(gPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fHalfWidth = input[0].size.x * 0.5f;
    float fHalfHeight = input[0].size.y * 0.5f;

    int col = gFrame % input[0].width;
    int row = gFrame / input[0].width;
    float next = 1.0f / input[0].width;

    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);

    
    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        pf2UVs[i].x = pf2UVs[i].x / input[0].width + next * col;
        pf2UVs[i].y = pf2UVs[i].y / input[0].width + next * row;
        output.uv = pf2UVs[i];
        output.nTexture = input[0].width * input[0].height;

        outStream.Append(output);
    }
}

[maxvertexcount(4)]
void GSFireEffect(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(gmtxView._12, gmtxView._22, gmtxView._32);
    float range = distance(gPosition, gWorld._41_42_43);
    
    float xrad = acos((gPosition.x - gWorld._41) / range);
    float zrad = asin((gPosition.z - gWorld._43) / range);
   
    float3 randPos;
    randPos.x = gWorld._41 + cos(radians(input[0].randomDir) + xrad) * range;
    randPos.z = gWorld._43 + sin(radians(input[0].randomDir) + zrad) * range;
    randPos.y = gPosition.y;
    
    float3 f3dir = normalize(randPos - gWorld._41_42_43);
  
    matrix world = gWorld;
     
    world._41 = world._41 + (gTime * f3dir.x);
    world._42 = world._42 + input[0].randomHeight * sin(radians(gTime + 45)) - (input[0].randomHeight / 2);
    world._43 = world._43 + (gTime * f3dir.z);
    
    input[0].center = (float3) mul(float4(input[0].center, 1.0), world); /*float3(0, 0, 0);*/
    float3 f3Look = normalize(gPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fHalfWidth = input[0].size.x * 0.5f;
    float fHalfHeight = input[0].size.y * 0.5f;

    int col = gFrame % input[0].width;
    int row = gFrame / input[0].width;
    float next = 1.0f / input[0].width;

    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);

    
    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        pf2UVs[i].x = pf2UVs[i].x / input[0].width + next * col;
        pf2UVs[i].y = pf2UVs[i].y / input[0].width + next * row;
        output.uv = pf2UVs[i];
        output.nTexture = input[0].width * input[0].height;

        outStream.Append(output);
    }
}

[maxvertexcount(4)]
void GSMonsterAttack(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(gmtxView._12, gmtxView._22, gmtxView._32);
    matrix world = gWorld;
    world._41 += cos(input[0].randomDir) * 300;
    world._43 += sin(input[0].randomDir) * 300;

    float3 f3dir = normalize(gPosition - world._41_42_43);

    world._41 = world._41 + (gTime * -f3dir.x) / 2;
    world._42 = world._42 + input[0].randomHeight * sin(radians(gTime)) + (input[0].randomHeight / 2);
    world._43 = world._43 + (gTime * -f3dir.z) / 2;

    input[0].center = (float3) mul(float4(input[0].center, 1.0), world); /*float3(0, 0, 0);*/
    float3 f3Look = normalize(gPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fHalfWidth = input[0].size.x * 0.5f;
    float fHalfHeight = input[0].size.y * 0.5f;

    int col = gFrame % input[0].width;
    int row = gFrame / input[0].width;
    float next = 1.0f / input[0].width;

    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);


    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        pf2UVs[i].x = pf2UVs[i].x / input[0].width + next * col;
        pf2UVs[i].y = pf2UVs[i].y / input[0].width + next * row;
        output.uv = pf2UVs[i];
        output.nTexture = input[0].width * input[0].height;

        outStream.Append(output);
    }
}

float4 PS2dTexture(GS_BILLBOARD_GEOMETRY_OUTPUT input) : SV_TARGET
{

    float4 cColor = gtxtTexture[0].Sample(gWrapSamplerState, input.uv);
    if (cColor.a <= 0.3f)
        discard; //clip(cColor.a - 0.3f);
    
    return (cColor);
}


[maxvertexcount(4)]
void GSUIHpBar(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(gmtxView._12, gmtxView._22, gmtxView._32);
    input[0].center = (float3) mul(float4(input[0].center, 1.0), gWorld);
    float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fHalfWidth = input[0].size.x * 0.5f;
    float fWidth = input[0].size.x;
    float fHalfHeight = input[0].size.y * 0.5f;
    float fDamage = float(100 - gFrame) / 100.f;
    
    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fWidth * f3Right) * fDamage - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fWidth * f3Right) * fDamage + (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        output.uv = pf2UVs[i];
        output.nTexture = input[0].width * input[0].height;

        outStream.Append(output);
    }
}

[maxvertexcount(4)]
void GS2DUI(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float fWidth = input[0].size.x * 0.5f;
    float fHeight = input[0].size.y * 0.5f;

    float3 fUp = float3(0, 1, 0);
    float3 fRight = float3(1, 0, 0);
    float4 pf4Vertices[4];

    pf4Vertices[0] = float4(input[0].center.xyz - (fWidth * fRight) - (fHeight * fUp), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz - (fWidth * fRight) + (fHeight * fUp), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz + (fWidth * fRight) - (fHeight * fUp), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz + (fWidth * fRight) + (fHeight * fUp), 1.0f);
    int col = gFrame % input[0].width;
    int row = gFrame / input[0].width;
    float nextWidth = 1.0f / input[0].width;
    float nextHeight = 1.0f / input[0].height;
    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = pf4Vertices[i];
        pf2UVs[i].x = pf2UVs[i].x / input[0].width + nextWidth * col;
        pf2UVs[i].y = pf2UVs[i].y / input[0].height + nextHeight * row;
        output.uv = pf2UVs[i];
        output.nTexture = input[0].width * input[0].height;

        outStream.Append(output);
    }
}

[maxvertexcount(4)]
void GSITEM(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(0, 1, 0);
    input[0].center = (float3) mul(float4(input[0].center, 1.0), gmtxWorld);
    float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fWidth = input[0].size.x * 0.5f;
    float fHeight = input[0].size.y * 0.5f;

    
    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz - (fWidth * f3Right) - (fHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz - (fWidth * f3Right) + (fHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz + (fWidth * f3Right) - (fHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz + (fWidth * f3Right) + (fHeight * f3Up), 1.0f);

    int col = gnMaterial % 3;
    int row = gnMaterial / 3;
    float next = 1.0f / 3.0f;


    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        pf2UVs[i].x = pf2UVs[i].x / 3 + next * col;
        pf2UVs[i].y = pf2UVs[i].y / 3 + next * row;
        output.uv = pf2UVs[i];
        output.nTexture = 0;

        outStream.Append(output);
    }
}

float4 PSITEM(GS_BILLBOARD_GEOMETRY_OUTPUT input) : SV_TARGET
{

    float4 cColor = gtxtTexture[input.nTexture].Sample(gWrapSamplerState, input.uv);
    if (cColor.a <= 0.3f)
        discard; //clip(cColor.a - 0.3f);
    
    return (cColor);
}

[maxvertexcount(4)]
void GSPANEL(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream)
{
    float3 f3Up = float3(0, 1, 0);
    input[0].center = (float3) mul(float4(input[0].center, 1.0), gmtxWorld);
    float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
    float3 f3Right = cross(f3Up, f3Look);
    float fWidth = input[0].size.x * 0.5f;
    float fHeight = input[0].size.y * 0.5f;


    float4 pf4Vertices[4];
    pf4Vertices[0] = float4(input[0].center.xyz - (fWidth * f3Right) - (fHeight * f3Up), 1.0f);
    pf4Vertices[1] = float4(input[0].center.xyz - (fWidth * f3Right) + (fHeight * f3Up), 1.0f);
    pf4Vertices[2] = float4(input[0].center.xyz + (fWidth * f3Right) - (fHeight * f3Up), 1.0f);
    pf4Vertices[3] = float4(input[0].center.xyz + (fWidth * f3Right) + (fHeight * f3Up), 1.0f);

    int col = gnMaterial % 3;
    int row = gnMaterial / 3;
    float next = 1.0f / 3.0f;


    GS_BILLBOARD_GEOMETRY_OUTPUT output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pf4Vertices[i].xyz;
        output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
        pf2UVs[i].x = pf2UVs[i].x / 4 + next * col;
        //pf2UVs[i].y = pf2UVs[i].y / 3 + next * row;
        output.uv = pf2UVs[i];
        output.nTexture = 0;

        outStream.Append(output);
    }
}

float4 PSPANEL(GS_BILLBOARD_GEOMETRY_OUTPUT input) : SV_TARGET
{

    float4 cColor = gtxtTexture[input.nTexture].Sample(gWrapSamplerState, input.uv);
    if (cColor.a <= 0.3f)
        discard; //clip(cColor.a - 0.3f);

    return (cColor);
}