#pragma once
#include "EffectUI.h"
#include <map>
class Font : public Effect
{
public:
	Font(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Vector2 startPos, Vector2 size);
	~Font() {};
	void SetFontData();
	void SetWord(string word);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
private:
	map<char, int> fontData;
	vector<UITexture*> texts;
};
