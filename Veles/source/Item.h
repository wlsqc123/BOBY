#pragma once
#include "Object.h"
#include "ServerMgr.h"

class UIItem;
class Effect;

class Item :public CGameObject
{
public:
	Item(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~Item();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void Update();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	ITEM_TYPE getItemType() const { return itemType; }
	void setItemType(ITEM_TYPE type) { itemType = type; }
private:
	ITEM_TYPE itemType;
};

class Inventory : public CGameObject
{
public:
	Inventory(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader* shader);
	virtual ~Inventory();

	vector<UIItem*> items;
	vector<Effect*> uiWord;

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void SetItem(ITEM_TYPE item);
	void InitItem();
};