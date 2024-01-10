#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "_pack.h"
#include "Protocol.h"

#include <tchar.h>

#pragma comment(lib,"ws2_32")

constexpr int MAX_AMMO = 0;

constexpr float OBB_SCALE_PLAYER_X = 30.f;
constexpr float OBB_SCALE_PLAYER_Y = 30.f;
constexpr float OBB_SCALE_PLAYER_Z = 30.f;

constexpr float OBB_SCALE_Magmaa_X = 36.f;
constexpr float OBB_SCALE_Magmaa_Y = 60.f;
constexpr float OBB_SCALE_Magmaa_Z = 45.f;
constexpr int HP_Magmaa = 10;

constexpr float OBB_SCALE_Golem_X = 120.f;
constexpr float OBB_SCALE_Golem_Y = 400.f;
constexpr float OBB_SCALE_Golem_Z = 100.f;
constexpr int HP_Golem = 10;

constexpr float OBB_SCALE_Orge_X = 45.f;
constexpr float OBB_SCALE_Orge_Y = 90.f;
constexpr float OBB_SCALE_Orge_Z = 36.f;
constexpr int HP_Orge = 10;

constexpr float OBB_SCALE_Chest_X = 50.f;
constexpr float OBB_SCALE_Chest_Y = 40.f;
constexpr float OBB_SCALE_Chest_Z = 60.f;
constexpr int HP_Chest = 10;

using namespace DirectX::PackedVector;
using namespace DirectX;

int recvn(SOCKET s, char* buf, int len, int flags);

enum Monster { MAGMA, GOLEM, ORGE, CHEST };


struct Player
{
	int		id;
	int		hp;
	float	speed;

	Vector3 InitPos;
	Vector3	CurPos;
	Vector3	PrevPos;
	Vector3	look;
	Vector3	camera_look;

	obj_state	state;
	
	Bullet		bullet;


	std::chrono::system_clock::time_point timeDead;
	std::chrono::system_clock::time_point timeReload;

	BoundingOrientedBox OOBB;
};


struct NPC
{
	int			id;
	int			type;
	int			hp;
	float		speed;
	int			destPl;
	Monster		mob;
	obj_state	state;

	Vector3 InitPos;
	Vector3	CurPos;
	Vector3	PrevPos;
	Vector3	Lookvec;
	Vector3	camLookvec;

	// 타이머 작업중이였음
	std::chrono::system_clock::time_point timeLastAttack;
	std::chrono::system_clock::time_point timeDeath;


	BoundingOrientedBox OOBB;
};

struct Structure
{
	Vector3 center;
	Vector3 extend;

	BoundingOrientedBox OOBB;
};



struct Client
{
	SOCKET		sock;
	WORD		id;

	KeyInput	input;
	Vector3		pos;
	Vector3		look;
	Vector3		camera_look;

	int			hp;
	float		speed;

	BoundingOrientedBox bounding_box;
	void SetOOBB(XMFLOAT3 xmCenter, XMFLOAT3 xmExtends, XMFLOAT4 xmOrientation) {
		bounding_box = BoundingOrientedBox(xmCenter, xmExtends, xmOrientation);
	}
};

struct OBJECT
{
	obj_type type;
	WORD id;

	Vector3 pos;
	Vector3 look;
	Vector3 prev_pos;
	float	speed;
};

/////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////

//
//struct CS_DUMMY_PACKET
//{
//	BYTE size;
//	BYTE type;
//	WORD id;
//	Vector3 lookvector;
//	KeyInput input;
//};
//
//struct CS_PACKET_KEYDOWN
//{
//	BYTE size;
//	BYTE type;
//	Vector3 Lookvec;
//};
//
//struct CS_PACKET_KEYUP
//{
//	BYTE size;
//	BYTE type;
//};
//
//struct CS_PACKET_KEYLEFT
//{
//	BYTE size;
//	BYTE type;
//};
//
//struct CS_PACKET_KEYRIGHT
//{
//	BYTE size;
//	BYTE type;
//};
//
