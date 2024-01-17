// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>

// C�� ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>
#include <math.h>

#include <string>
#include <shellapi.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "d3dx12.h"
#include <Mmsystem.h>

#include <assert.h>
#include <algorithm>
#include <memory.h>
#include <wrl.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "_pack.h"

//#include <fbxsdk.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "dxguid.lib")	

#define FRAME_BUFFER_WIDTH		1280
#define FRAME_BUFFER_HEIGHT		720

#define MAX_LIGHTS				32
#define MAX_MATERIALS			8 
#define MAX_SHADOWMAP			4

#define MAX_VERTEX_INFLUENCES				4
#define SKINNED_ANIMATION_BONES				128

#define POINT_LIGHT				1
#define SPOT_LIGHT				2
#define DIRECTIONAL_LIGHT		3


#define OBJECT_CBV 0
#define TEXTURE_SRV 1
#define EFFECT_CBV 2
#define PLAYER_CBV 3
#define CAMERA_CBV 4
#define MATERIAL_CBV 5
#define LIGHT_CBV 6
#define TERRAIN_SRV 7
#define SKYBOX_SRV 8
#define SHADOWMAP_SRV 9
#define SHADOWMAP_CBV 10
#define SKINNEDBONETRANSFORMS_CBV 11
#define ENVIROMENT_CBV 12
#define SHADOWMAPS_CBV 13
#define SSAOMAP_SRV 14
#define LAVAWAVE_CBV 15
#define LAVAWAVE_SRV 16
#define MAP_SRV 17

#define EPSILON				1.0e-10f


//�ִϸ��̼� ���� ���
enum class ANIMATION_PLAY_TYPE
{
	ANIMATION_PLAY_ONCE,			//O->��, ������ ����
	ANIMATION_PLAY_LOOP,			//0->��, 0->�� �ݺ�
	ANIMATION_PLAY_PINGPONG,		//0->��->0->�� �ݺ�
};

//PSO TYPE ��ó���Ҷ� �Ƹ� �� �߰��ɵ�??
enum class RENDER_TYPE : int
{
	IDLE_RENDER = 0,
	SHADOW_RENDER,
	DRAW_NORMAL,
	PREV_RENDER,
};
//������Ʈ Ÿ�Կ����� ����ٸ� �ִϸ��̼� state�� ������� ���? state�� ���ϰ��ְ�����
enum class OBJECT_TYPE
{
	FPS_PLAYER,
	TPS_PLAYER,
	MONSTER,
	INTERACTION,
	STATIC,
};

enum class OBJECT_STATE
{
	IDLE,
	MOVE_FORWARD,
	MOVE_SIDE,
	ATTACK,
	REROAD,
	DIE,
	INTERACTION,
};

enum SCENE_TYPE
{
	LOOBY,
	INGAME,
};

enum PANEL_TYPE
{
	PANEL_BIRD,
	PANEL_FISH,
	PANEL_DRAGON,
	PANEL_SNAKE,
};

extern RENDER_TYPE renderType;


/*#pragma comment(lib, "DirectXTex.lib") */

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

extern UINT	gnCbvSrvDescriptorIncrementSize;
extern UINT gnDsvDescriptorIncrementSize;
extern UINT gnRtvDescriptorIncrementSize;
extern int Count;
extern ID3D12Resource *CreateBufferResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ID3D12Resource **ppd3dUploadBuffer = NULL);
extern ID3D12Resource *CreateTextureResourceFromDDSFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, ID3D12Resource **ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

// �ٸ� �������� ������� ���� ������� �������� ����� �׸���ǥ��
// ���н��� ������� ���θ� ����
// �ٸ� �÷��̾ �ٶ󺸴� ���⿡ ���� ���� �ٲ�����
// �� ������ ������ ����Ʈ����Ʈ�� �´� �������� ��������� ������
