#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
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

#pragma comment(lib,"ws2_32")
#pragma comment(lib, "MSWSock.lib")

constexpr int max_ammo = 0;
constexpr int max_users = 400;
constexpr int max_room = 100;

constexpr float player_speed = 10;

constexpr float obb_scale_player_x = 30.f;
constexpr float obb_scale_player_y = 30.f;
constexpr float obb_scale_player_z = 30.f;

constexpr float obb_scale_magmaa_x = 36.f;
constexpr float obb_scale_magmaa_y = 60.f;
constexpr float obb_scale_magmaa_z = 45.f;
constexpr int hp_magmaa = 10;

constexpr float obb_scale_golem_x = 120.f;
constexpr float obb_scale_golem_y = 400.f;
constexpr float obb_scale_golem_z = 100.f;
constexpr int hp_golem = 10;

constexpr float obb_scale_orge_x = 45.f;
constexpr float obb_scale_orge_y = 65.f;
constexpr float obb_scale_orge_z = 25.f;
constexpr int hp_orge = 10;

constexpr float obb_scale_chest_x = 70.f;
constexpr float obb_scale_chest_y = 35.f;
constexpr float obb_scale_chest_z = 38.5f;

constexpr float obb_scale_lever_x = 20.f;
constexpr float obb_scale_lever_y = 30.f;
constexpr float obb_scale_lever_z = 30.f;

constexpr float obb_scale_door_x = 162.f;
constexpr float obb_scale_door_y = 112.f;
constexpr float obb_scale_door_z = 22.5f;

using namespace std;
using namespace DirectX::PackedVector;
using namespace DirectX;


enum monster { magma, golem, ogre};
enum static_object {chest, door, lever, stone, mud, rocks};
enum op_type { op_recv, op_send, op_accept };
enum pl_state { plst_free, plst_connected, plst_inlobby, plst_ingame, plst_end };
enum WALL_TYPE {wall, fence};


struct EX_OVER
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsabuf[1];
	unsigned char	m_packetbuf[max_buffer];
	op_type			m_op;
	SOCKET			m_csocket;					// OP_ACCEPT������ ���
};

struct player_status
{
	int		max_hp = 0;
	int		hp = 0;
	float	speed = 5.f;
	float	attack_speed = 0;
	float	attack_damage = 0;
	int		heal = 0;
	int		block = 0;
	int		boss_damage = 0;
	int		kill_max_hp = 0;
	int		instant_death = 0;
	short	ammo;
	short	max_ammo;
};

struct item_status
{
	ITEM_TYPE item;
	bool getEnable = false;
};

struct player
{
	mutex				m_slock;
	atomic <pl_state>	m_state;
	SOCKET				m_socket;

	EX_OVER				m_recv_over;
	int					m_prev_size;

	////////
	int r_id;
	bool ready;
	char name[max_name];
	//...
	/// /////
	int		id;
	player_status ps;
	map<ITEM_TYPE, int> pl_items;
	ITEM_TYPE			current_item;
	ITEM_TYPE			active_item;
	WEAPON_TYPE			wp_type;

	Vector3				init_pos;
	Vector3				cur_pos;
	Vector3				prev_pos;
	Vector3				pl_look;
	Vector3				cam_look;

	obj_state	state;
	
	Bullet		bullet[5];
	bool		reload_enable;

	std::chrono::system_clock::time_point time_dead;
	std::chrono::system_clock::time_point time_reload;

	BoundingOrientedBox OOBB;
};


struct NPC
{
	int			id;
	int			type;
	int			hp;
	float		speed;
	int			dest_pl;
	monster		mob;
	obj_state	state;
	int			attack_range;
	int			sight;
	int			zone_num;
	bool		is_attack;
	LONGLONG	cool_time;
	bool		attack_packet_enable;

	Vector3 init_pos;
	Vector3	cur_pos;
	Vector3	prev_pos;
	Vector3	look_vec;
	Vector3	cam_lookvec;

	// Ÿ�̸� �۾����̿���
	std::chrono::system_clock::time_point time_last_attack;
	std::chrono::system_clock::time_point time_death;


	BoundingOrientedBox OOBB;
};

struct interaction
{
	int			id;
	int			type;
	obj_state	state;
	static_object object_name;

	Vector3 pos;
	Vector3	lookvec;
	Vector3	cam_lookvec;
	vector<item_status> item;
	bool	interact_enable;
	int		zone_num;

	BoundingOrientedBox OOBB;
};

struct structure
{
	Vector3 center;
	Vector3 extend;
	WALL_TYPE type;

	BoundingOrientedBox OOBB;
};

struct range_attack
{
	Vector3 pos;
	Vector3 look;

	BoundingOrientedBox OOBB;
	bool	active_enable;
	float	speed;
	std::chrono::system_clock::time_point live_time;
};

struct client
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
	void set_OOBB(const XMFLOAT3 xm_center, const XMFLOAT3 xm_extends, const XMFLOAT4 xm_orientation) {
		bounding_box = BoundingOrientedBox(xm_center, xm_extends, xm_orientation);
	}
};

struct object
{
	obj_type type;
	WORD id;

	Vector3 pos;
	Vector3 look;
	Vector3 prev_pos;
	float	speed;
};

struct lobby_player_info
{
	int		id;
	char	name[max_name];
	bool	ready;
};

struct lobby_room
{
	vector <lobby_player_info> pl;
};

struct zone
{
	vector <int> monster_id;
	bool is_clear;
};


extern array <player, max_users> arr_player;
extern array <lobby_room, max_room> arr_lobby;
