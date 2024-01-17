#pragma once
#include "Object.h"

class UIPanel;
class Effect;

class Panel : public CGameObject
{
public:
	Panel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~Panel();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void Update();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	
	PANEL_TYPE GetPanelType() const { return panelType; }
	void SetPanelType(PANEL_TYPE type) { panelType = type; }

private:
	PANEL_TYPE panelType;
};

