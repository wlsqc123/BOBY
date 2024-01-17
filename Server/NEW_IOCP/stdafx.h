#pragma once

#include <winsock2.h>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <array>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#include "_pack.h"
#include "Protocol.h"

#include <tchar.h>

#pragma comment(lib,"ws2_32")
#pragma comment(lib, "MSWSock.lib")

constexpr int MAX_AMMO = 0;
constexpr int MAX_USERS = 400;
constexpr int MAX_ROOM = 100;

constexpr float PLAYER_SPEED = 10;

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
constexpr float OBB_SCALE_Orge_Y = 65.f;
constexpr float OBB_SCALE_Orge_Z = 25.f;
constexpr int HP_Orge = 10;

constexpr float OBB_SCALE_Chest_X = 70.f;
constexpr float OBB_SCALE_Chest_Y = 35.f;
constexpr float OBB_SCALE_Chest_Z = 38.5f;

constexpr float OBB_SCALE_Lever_X = 20.f;
constexpr float OBB_SCALE_Lever_Y = 30.f;
constexpr float OBB_SCALE_Lever_Z = 30.f;

constexpr float OBB_SCALE_Door_X = 162.f;
constexpr float OBB_SCALE_Door_Y = 112.f;
constexpr float OBB_SCALE_Door_Z = 22.5f;

using namespace std;
using namespace DirectX::PackedVector;
using namespace DirectX;


enum Monster { MAGMA, GOLEM, OGRE};
enum STATIC_OBJECT {CHEST, DOOR, LEVER, STONE, MUD, ROCKS};
enum OP_TYPE { OP_RECV, OP_SEND, OP_ACCEPT };
enum PL_STATE { PLST_FREE, PLST_CONNECTED, PLST_INLOBBY, PLST_INGAME, PLST_END };
enum WALL_TYPE {wall, fence};


struct EX_OVER
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsabuf[1];
	unsigned char	m_packetbuf[MAX_BUFFER];
	OP_TYPE			m_op;
	SOCKET			m_csocket;					// OP_ACCEPT������ ���
};

struct player_status
{
	int		maxhp = 0;
	int		hp = 0;
	float	speed = 5.f;
	float	attackSpeed = 0;
	float	attackDamage = 0;
	int		heal = 0;
	int		block = 0;
	int		bossDamage = 0;
	int		killMaxHp = 0;
	int		instantDeath = 0;
	short	ammo;
	short	maxAmmo;
};

struct item_status
{
	ITEM_TYPE item;
	bool getEnable = false;
};

struct Player
{
	mutex				m_slock;
	atomic <PL_STATE>	m_state;
	SOCKET				m_socket;

	EX_OVER				m_recv_over;
	int					m_prev_size;

	////////
	int r_id;
	bool ready;
	char name[MAX_NUM_NAME];
	//...
	/// /////
	int		id;
	player_status ps;
	map<ITEM_TYPE, int> pl_items;
	ITEM_TYPE			currentItem;
	ITEM_TYPE			activeItem;
	WEAPON_TYPE			wp_type;

	Vector3				InitPos;
	Vector3				CurPos;
	Vector3				PrevPos;
	Vector3				pl_look;
	Vector3				cam_look;

	ObjectState	state;
	
	Bullet		bullet[5];
	bool		reloadEnable;

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
	ObjectState	state;
	int			attackRange;
	int			sight;
	int			zoneNum;
	bool		isAttack;
	LONGLONG	coolTime;
	bool		attackPacketEnable;

	Vector3 InitPos;
	Vector3	CurPos;
	Vector3	PrevPos;
	Vector3	Lookvec;
	Vector3	camLookvec;

	// Ÿ�̸� �۾����̿���
	std::chrono::system_clock::time_point timeLastAttack;
	std::chrono::system_clock::time_point timeDeath;


	BoundingOrientedBox OOBB;
};

struct INTERACTION
{
	int			id;
	int			type;
	ObjectState	state;
	STATIC_OBJECT objectName;

	Vector3 Pos;
	Vector3	Lookvec;
	Vector3	camLookvec;
	vector<item_status> item;
	bool interactEnable;
	int			zoneNum;

	BoundingOrientedBox OOBB;
};

struct Structure
{
	Vector3 center;
	Vector3 extend;
	WALL_TYPE type;

	BoundingOrientedBox OOBB;
};

struct RangeAttack
{
	Vector3 pos;
	Vector3 look;

	BoundingOrientedBox OOBB;
	bool	activeEnable;
	float	speed;
	std::chrono::system_clock::time_point liveTime;
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
	ObjectType type;
	WORD id;

	Vector3 pos;
	Vector3 look;
	Vector3 prev_pos;
	float	speed;
};

struct LOBBY_PLAYER_INFO
{
	int		id;
	char	name[MAX_NUM_NAME];
	bool	ready;
};

struct LOBBY_ROOM
{
	vector <LOBBY_PLAYER_INFO> pl;
};

struct ZONE
{
	vector <int> monsterID;
	bool isClear;
};


extern array <Player, MAX_USERS> arr_player;
extern array <LOBBY_ROOM, MAX_ROOM> arr_lobby;



////////////////////////////////////////////////////////////////


//queue <PLAYER_INFO> Lobby;
//queue <PLAYER_INFO> from_Gamemgr_to_Lobby;

