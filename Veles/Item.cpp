#include "stdafx.h"
#include "Item.h"
#include "Shader.h"
#include "EffectUI.h"

Item::Item(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	worldMatrix = Matrix4x4::identity;
	isAlive = false;
	itemType = ITEM_TYPE::ITEM_ATTACK_SPEEDUP;

	CTexture* pEffectTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pEffectTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/item.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pEffectMaterial = new CMaterial();
	pEffectMaterial->SetTexture(pEffectTexture);
	pEffectMaterial->SetReflection(1);
	SetMaterial(pEffectMaterial);
	vector<GeometryVertexInfo> vecGeoVertex;
	GeometryVertexInfo geoVertex;
	geoVertex.position = Vector3(0, 0, 0);
	geoVertex.size = Vector2(25, 25);
	geoVertex.nWidth = 1;
	geoVertex.nHeight = 1;

	vecGeoVertex.push_back(geoVertex);
	CGeometryBillboardMesh* effectMesh = new CGeometryBillboardMesh(pd3dDevice, pd3dCommandList, vecGeoVertex);
	SetMesh(0, effectMesh);

	objectBoundingBox = BoundingOrientedBox(Vector3(0, 0, 0), Vector3(25, 25, 25), Vector4(0, 0, 0, 1));

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö

	ItemShader* itemShader = new ItemShader();
	itemShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	itemShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	itemShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1);
	itemShader->CreateConstantBufferViews(pd3dDevice, 1, cbGameObject, ncbElementBytes);
	itemShader->CreateShaderResourceViews(pd3dDevice, pEffectTexture, 0, TEXTURE_SRV);

	SetCbvGPUDescriptorHandle(itemShader->GetGPUCbvDescriptorStartHandle());

	SetShader(itemShader);
}

Item::~Item()
{
}

void Item::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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

void Item::Update()
{
	objectBoundingBox.Center = Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);
}

void Item::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&cbMappedGameObject->worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	if (objectMaterial) cbMappedGameObject->m_nMaterial = itemType;
}

Inventory::Inventory(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader)
{
	float posX = -0.8f;
	float posY = 0.5f;
	for (int i = 0; i < 20; i++)
	{
		UIItem* item = new UIItem(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, posX, posY, shader);
		items.push_back(item);
		posX += 0.177f;
		if (i == 9)
		{
			posY -= 0.17f;
			posX = -0.8f;
		}
	}
	UITexture* word = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/inven.dds", Vector2(0, 0), Vector2(1.8, 1.8), Vector2(1, 1), shader);
	word->SetIsAlive(true);
	uiWord.push_back(word);
}

Inventory::~Inventory()
{
}

void Inventory::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (auto& p : items)
		p->Render(pd3dCommandList, pCamera);
	for (auto& p : uiWord)
		p->Render(pd3dCommandList, pCamera);

}

void Inventory::SetItem(ITEM_TYPE item)
{
	for (auto& p : items)
	{
		if (!p->GetIsAlive())
		{
			p->SetItem(item);
			break;
		}
	}
}

void Inventory::InitItem()
{
	for (auto& p : items)
	{
		p->SetIsAlive(false);
	}
}
