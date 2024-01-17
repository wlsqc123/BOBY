//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			32
#define MAX_MATERIALS		8 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3


struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

struct LIGHT
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;
    float3 m_vPosition;
    float m_fFalloff;
    float3 m_vDirection;
    float m_fTheta; //cos(m_fTheta)
    float3 m_vAttenuation;
    float m_fPhi; //cos(m_fPhi)
    bool m_bEnable;
    int m_nType;
    float m_fRange;
    bool m_bShadow;
};

cbuffer cbMaterial : register(b4)
{
    MATERIAL gMaterials[MAX_MATERIALS];
};

cbuffer cbLights : register(b5)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    bool isGrayScale;
};

LIGHT DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera)
{
    LIGHT light = gLights[nIndex];
    
    float3 vToLight = -gLights[nIndex].m_vDirection;
    float fDiffuseFactor =dot(vToLight, vNormal);
    float fSpecularFactor = 0.0f;
    if (fDiffuseFactor > 0.0f)
    {
        float specular = 3.1f;
        float vHalf = normalize(vToCamera + vToLight);
        fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), specular);

        light.m_cAmbient = gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient;
        light.m_cDiffuse = gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse;
        light.m_cSpecular = gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular;
    
        return light;
    }
    return (LIGHT) 0;
}

LIGHT PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    LIGHT light;
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        float fDiffuseFactor = dot(vToLight, vNormal);
        if (fDiffuseFactor > 0.0f)
        {
            if (gMaterials[gnMaterial].m_cSpecular.a != 0.0f)
            {
                float3 vReflect = reflect(-vToLight, vNormal);
                fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
            }
            float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

            light.m_cAmbient = gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient * fAttenuationFactor;
            light.m_cDiffuse = gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse * fAttenuationFactor;
            light.m_cSpecular = gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular * fAttenuationFactor;
            return light;
        }
        return (LIGHT) 0;
    }
    return (LIGHT) 0;
}

LIGHT SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    LIGHT light;
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        float fDiffuseFactor = dot(vToLight, vNormal);
        if (fDiffuseFactor > 0.0f)
        {
            if (gMaterials[gnMaterial].m_cSpecular.a != 0.0f)
            {
                float3 vReflect = reflect(-vToLight, vNormal);
                fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
            }

            float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
            float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
            float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

        
            light.m_cAmbient = gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient * fAttenuationFactor * fSpotFactor;
            light.m_cDiffuse = gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse * fAttenuationFactor * fSpotFactor;
            light.m_cSpecular = gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular * fAttenuationFactor * fSpotFactor;
        
            return light;
        }
    }
    return (LIGHT) 0;

}

float4 LightingShadow(float3 vPosition, float3 vNormal, float fShadowFactor[4])
{
    float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
    float3 vToCamera = normalize(vCameraPosition - vPosition);

    LIGHT LightedColor;
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    uint shadowIndex = 0;
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                LightedColor = DirectionalLight(i, vNormal, vToCamera);
                if(gLights[i].m_bShadow)
                {               
                    cColor += (LightedColor.m_cAmbient * fShadowFactor[shadowIndex] + LightedColor.m_cDiffuse * fShadowFactor[shadowIndex] + LightedColor.m_cSpecular * fShadowFactor[shadowIndex]);
                    shadowIndex++;
                }
                else if (!gLights[i].m_bShadow)
                    cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
            }
            else if (gLights[i].m_nType == POINT_LIGHT)
            {
                LightedColor = PointLight(i, vPosition, vNormal, vToCamera);
                if (gLights[i].m_bShadow)
                {
                    cColor += (LightedColor.m_cAmbient * fShadowFactor[shadowIndex] + LightedColor.m_cDiffuse * fShadowFactor[shadowIndex] + LightedColor.m_cSpecular * fShadowFactor[shadowIndex]);
                    shadowIndex++;
                }
                else if (!gLights[i].m_bShadow)
                    cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                LightedColor = SpotLight(i, vPosition, vNormal, vToCamera);
                if (gLights[i].m_bShadow)
                {
                    cColor += (LightedColor.m_cAmbient * fShadowFactor[shadowIndex] + LightedColor.m_cDiffuse * fShadowFactor[shadowIndex] + LightedColor.m_cSpecular * fShadowFactor[shadowIndex]);
                    shadowIndex++;
                }
                else if (!gLights[i].m_bShadow)
                    cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
            }
        }
    }
    
    return (cColor);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
    float3 vToCamera = normalize(vCameraPosition - vPosition);

    LIGHT LightedColor;
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                LightedColor = DirectionalLight(i, vNormal, vToCamera);
                cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
                //cColor += DirectionalLight(i, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == POINT_LIGHT)
            {
                LightedColor = PointLight(i, vPosition, vNormal, vToCamera);
                cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
                // cColor += PointLight(i, vPosition, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                LightedColor = SpotLight(i, vPosition, vNormal, vToCamera);
                cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
                //  cColor += SpotLight(i, vPosition, vNormal, vToCamera);
            }
        }
    }
    
    return (cColor);
}
