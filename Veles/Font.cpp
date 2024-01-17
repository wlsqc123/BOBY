#include "stdafx.h"
#include "Font.h"
#include "Shader.h"

Font::Font(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, Vector2 startPos, Vector2 size) : Effect(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	SetFontData();
	CUiShader* uiShader = new CUiShader();
	uiShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	Vector2 pos = startPos;
	texts.reserve(20);
	for (int i = 0; i < 20; ++i)
	{
		UITexture* word = new UITexture(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, L"Image/word.dds", pos, size, Vector2(10,10), uiShader);
		pos.x += size.x / 2;
		word->SetIsAlive(false);
		texts.push_back(word);
	}
}

void Font::SetFontData()
{
	int num = 0;
	for (char i = ' '; i <= '~'; ++i,++num)
	{
		fontData.insert({i, num}) ;
	}
}

void Font::SetWord(string word)
{
	int temp = 0;
	for (char p : word)
	{
		texts[temp]->SetFrameCount(fontData[p]);
		texts[temp]->SetIsAlive(true);
		++temp;
		Mathf::Clamp(temp, 0, 19);
	}
}

void Font::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (auto& p : texts)
		p->Render(pd3dCommandList, nullptr);
}
