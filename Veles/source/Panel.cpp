#include "stdafx.h"
#include "Panel.h"
#include "Shader.h"
#include "EffectUI.h"

Panel::Panel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	worldMatrix = Matrix4x4::identity;
	isAlive = true;
	panelType = PANEL_TYPE::PANEL_BIRD;

	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/panel.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(30, 30);
	geoVertex.nWidth = 1;
	geoVertex.nHeight = 1;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	objectBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(30, 30, 30), Vector4(0, 0, 0, 1));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	//shader 수정해야됨
	PanelShader* panelShader = new PanelShader();
	panelShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	panelShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	panelShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	panelShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	panelShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(panelShader->GetGPUCbvDescriptorStartHandle());

	SetShader(panelShader);
}

Panel::~Panel()
{
}

void Panel::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (isAlive)
	{
		if (objectMaterial)
		{
			if (objectMaterial->m_pShader)
			{
				objectMaterial->m_pShader->Render(pd3dCommandList, pCamera);
				objectMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

				UpdateShaderVariables(pd3dCommandList);
			}
			if (objectMaterial->m_pTexture)
			{
				objectMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			}
		}

		pd3dCommandList->SetGraphicsRootDescriptorTable(OBJECT_CBV, cbvGPUDescriptorHandle);

		if (meshes)
		{
			for (int i = 0; i < numMeshes; i++)
			{
				if (meshes[i]) meshes[i]->Render(pd3dCommandList);
			}
		}
	}
}

void Panel::Update()
{
	objectBoundingBox.Center = Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

}

void Panel::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&cbMappedGameObject->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	if (objectMaterial) cbMappedGameObject->m_nMaterial = panelType;
}
