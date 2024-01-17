#include "stdafx.h"
#include "SceneLoby.h"
#include "Shader.h"

SceneLoby::SceneLoby() : Scene()
{
}

void SceneLoby::Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	glRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	Vector2 pos = Vector2(0.3f, 0.6f);
	for (UINT i = 0; i < MAX_PLAYER; ++i)
	{
		Font* font = new Font(pd3dDevice, pd3dCommandList, glRootSignature, pos, Vector2(0.12f, 0.18f));
		pos.y -= 0.38f;
		glUserName.push_back(font);
	}
	cout << "Nick name : ";
	getline(cin , word);

	SetName(word);

	//glUserName.at(0)->SetWord(word);
	CUiShader* uiShader = new CUiShader();
	uiShader->CreateShader(pd3dDevice, glRootSignature);
	glLobyTexture = new UITexture(pd3dDevice, pd3dCommandList, glRootSignature, L"Image/loby.dds", Vector2(0, 0), Vector2(2, 2), Vector2(2, 1), uiShader);
	glRifleIcon = new UITexture(pd3dDevice, pd3dCommandList, glRootSignature, L"Image/rifleicon.dds", Vector2(-0.83, 0), Vector2(0.3, 0.45), Vector2(2, 1), uiShader);
	glSniperIcon = new UITexture(pd3dDevice, pd3dCommandList, glRootSignature, L"Image/snipericon.dds", Vector2(-0.5, 0), Vector2(0.3, 0.45), Vector2(2, 1), uiShader);
	glShotgunIcon = new UITexture(pd3dDevice, pd3dCommandList, glRootSignature, L"Image/shotgunicon.dds", Vector2(-0.17, 0), Vector2(0.3, 0.45), Vector2(2, 1), uiShader);
}

ID3D12RootSignature* SceneLoby::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2; //GameObject
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 10;
	pd3dDescriptorRanges[1].BaseShaderRegister = 14; //t14: gtxtTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[OBJECT_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[OBJECT_CBV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[OBJECT_CBV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[OBJECT_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[TEXTURE_SRV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[TEXTURE_SRV].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[TEXTURE_SRV].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[TEXTURE_SRV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[EFFECT_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[EFFECT_CBV].Descriptor.ShaderRegister = 8;
	pd3dRootParameters[EFFECT_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[EFFECT_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[5];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_ANISOTROPIC;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[3].MipLODBias = 0;
	pd3dSamplerDescs[3].MaxAnisotropy = 8;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	pd3dSamplerDescs[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[4].MipLODBias = 0;
	pd3dSamplerDescs[4].MaxAnisotropy = 8;
	pd3dSamplerDescs[4].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[4].MinLOD = 0;
	pd3dSamplerDescs[4].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[4].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[4].ShaderRegister = 4;
	pd3dSamplerDescs[4].RegisterSpace = 0;
	pd3dSamplerDescs[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void SceneLoby::ProcessInput(HWND hWnd, float timeElapsed, ServerMgr* servermgr)
{
}

void SceneLoby::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		::GetCursorPos(&cursorPos);
		ScreenToClient(hWnd, &cursorPos);
		//cout << float(cursorPos.x ) / FRAME_BUFFER_WIDTH << " " << float(cursorPos.y) / FRAME_BUFFER_HEIGHT << endl;
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
		}
		break;
	default:
		break;
	}
}

void SceneLoby::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

bool SceneLoby::CheckMousePos(Vector2 min, Vector2 max)
{
	float posx = float(cursorPos.x) / FRAME_BUFFER_WIDTH;
	float posy = float(cursorPos.y) / FRAME_BUFFER_HEIGHT;
	return (posx > min.x&& posy > min.y) && (posx < max.x && posy < max.y);
}
//ó���� �Ǻ����� �Ǻ�->
//���� if���� ������Ű�� ready or start 
//������ �ƴϸ� ready �����̸� start
void SceneLoby::Update(float fTimeElapsed, ServerMgr* servermgr)
{
	//���⼭ ������ Ŭ�� �غ��
	if (CheckMousePos(Vector2(0.014f, 0.39f), Vector2(0.15f, 0.6f)))
	{
		glRifleIcon->SetFrameCount(1);
		glSniperIcon->SetFrameCount(0);
		glShotgunIcon->SetFrameCount(0);
		glWeaponType = WEAPON_RIFLE;
	}
	else if (CheckMousePos(Vector2(0.178f, 0.39f), Vector2(0.324f, 0.6f)))
	{
		glRifleIcon->SetFrameCount(0);
		glSniperIcon->SetFrameCount(1);
		glShotgunIcon->SetFrameCount(0);
		glWeaponType = WEAPON_SNIPER;
	}
	else if (CheckMousePos(Vector2(0.34f, 0.39f), Vector2(0.49f, 0.6f)))
	{
		glRifleIcon->SetFrameCount(0);
		glSniperIcon->SetFrameCount(0);
		glShotgunIcon->SetFrameCount(1);
		glWeaponType = WEAPON_SHOTGUN;
	}	
	
	servermgr->CSpacket_lobby.weapon_type = glWeaponType;
	strcpy(servermgr->CSpacket_lobby.name, word.c_str());
	servermgr->CSpacket_lobby.type = CS_LOBBY_PACKET;
	servermgr->CSpacket_lobby.size = sizeof(cs_ingame_packet);
	//
	
	if (CheckMousePos(Vector2(0.064f, 0.645f), Vector2(0.4f, 0.828f))) {
		servermgr->CSpacket_lobby.type = CS_READY_PACKET;
		if (servermgr->CSpacket_lobby.ready == false)
			servermgr->CSpacket_lobby.ready = true;
	}

	if (servermgr->SCpacket_lobby.type == SC_LOBBY_PACKET)
	{
		string name;
		for (int i = 0; i < MAX_PLAYER; ++i)
		{
			name = servermgr->SCpacket_lobby.info[i].name;
			glUserName.at(i)->SetWord(name);
		}
	}

	if (servermgr->SCpacket_lobby.type == SC_LOBBY_TO_GAME_PACKET) {
		servermgr->state = 1;
		sceneChangeFlag = true;
		cout << "sceneChangeFlag: " << sceneChangeFlag << endl;

		servermgr->RecvPacket();
		cout << "ingame packet type: " << servermgr->SCpacket_ingame.type << endl;
		if (servermgr->SCpacket_ingame.type == SC_SET_ID_PACKET) {
			cout << "SET_ID_PACKET RECV: " <<  servermgr->SCpacket_ingame.id << endl;
			servermgr->myID = servermgr->SCpacket_ingame.id;
		}
	}
}

void SceneLoby::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(glRootSignature);
		cmdList->RSSetViewports(1, &m_d3dViewport);
		cmdList->RSSetScissorRects(1, &m_d3dScissorRect);
		for (auto& p : glUserName)
			p->Render(cmdList, nullptr);
	glRifleIcon->Render(cmdList, nullptr);
	glSniperIcon->Render(cmdList, nullptr);
	glShotgunIcon->Render(cmdList, nullptr);
	glLobyTexture->Render(cmdList, nullptr);
}
