#include "stdafx.h"
#include "Light.h"

CLight::CLight(LIGHTS *light)
{
	m_pLights = light;
}

void CLight::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CLight::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
	m_pcbMappedLights->isGrayScale = isGray;
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(LIGHT_CBV, d3dcbLightsGpuVirtualAddress); //Lights
}

void CLight::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CLight::setLightPosition(int index, Vector3 position)
{
	m_pLights->m_pLights[index].m_xmf3Position = position;
}


void CLight::setLightDirection(int index, Vector3 direction)
{
	m_pLights->m_pLights[index].m_xmf3Direction = direction;
}

void CLight::setLightAlive(int index, bool isalive)
{
	m_pLights->m_pLights[index].m_bEnable = isalive;
}

void CLight::setLightRange(int index, float range)
{
	m_pLights->m_pLights[index].m_fRange = range;
}
