#pragma once
struct LIGHT
{
	Vector4				m_xmf4Ambient;
	Vector4				m_xmf4Diffuse;
	Vector4				m_xmf4Specular;
	Vector3				m_xmf3Position;
	float 				m_fFalloff;
	Vector3				m_xmf3Direction;
	float 				m_fTheta; //cos(m_fTheta)
	Vector3				m_xmf3Attenuation;
	float				m_fPhi; //cos(m_fPhi)
	bool				m_bEnable;
	int					m_nType;
	float				m_fRange;
	bool				m_bShadow = false;
};

struct LIGHTS
{
	LIGHT				m_pLights[MAX_LIGHTS];
	Vector4				m_xmf4GlobalAmbient;
	bool				isGrayScale;
};

class CLight
{
private:

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	bool isGray = false;
public:
	CLight(LIGHTS* light);
	CLight() {};
	~CLight() {};

	LIGHTS* m_pLights = NULL;
	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();
	void setLightPosition(int index, Vector3 position);
	void setLightDirection(int index, Vector3 direction);
	void setLightAlive(int index, bool isalive);
	void setLightRange(int index, float range);
	void setGrayScale(bool isgray) { isGray = isgray; }
};

