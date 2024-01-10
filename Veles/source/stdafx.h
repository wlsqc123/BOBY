// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C의 런타임 헤더 파일입니다.
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


//애니메이션 실행 방식
enum class ANIMATION_PLAY_TYPE
{
	ANIMATION_PLAY_ONCE,			//O->끝, 끝에서 멈춤
	ANIMATION_PLAY_LOOP,			//0->끝, 0->끝 반복
	ANIMATION_PLAY_PINGPONG,		//0->끝->0->끝 반복
};

//PSO TYPE 후처리할때 아마 더 추가될듯??
enum class RENDER_TYPE : int
{
	IDLE_RENDER = 0,
	SHADOW_RENDER,
	DRAW_NORMAL,
	PREV_RENDER,
};
//오브젝트 타입에따라 각기다른 애니메이션 state를 지녀야함 어떻게? state를 지니고있게하자
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

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

extern UINT	gnCbvSrvDescriptorIncrementSize;
extern UINT gnDsvDescriptorIncrementSize;
extern UINT gnRtvDescriptorIncrementSize;
extern int Count;
extern ID3D12Resource *CreateBufferResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ID3D12Resource **ppd3dUploadBuffer = NULL);
extern ID3D12Resource *CreateTextureResourceFromDDSFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, ID3D12Resource **ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

// 다른 조명으로 쉐도우맵 따서 쉐도우맵 여러개를 사용한 그림자표현
// 원패스에 쉐도우맵 전부를 딸것
// 다른 플레이어가 바라보는 방향에 맞춰 조명도 바뀌어야함
// 그 조명을 가지고 스포트라이트에 맞는 원근투영 쉐도우맵을 따야함
