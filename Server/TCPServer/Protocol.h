#pragma once

enum obj_type { type_none, type_npc, type_wall, type_player };
enum obj_state { none, move, attack, hit, dead, reload };


constexpr int MAGMAMONSTER_NUM = 9;
constexpr int GOLEMMONSTER_NUM = 1;
constexpr int ORGEMONSTER_NUM = 8;
constexpr int CHESTOBJECT_NUM = 2;

constexpr int MAX_OBJECT = MAGMAMONSTER_NUM + GOLEMMONSTER_NUM + ORGEMONSTER_NUM + CHESTOBJECT_NUM;
constexpr int MAX_PLAYER = 4;

struct KeyInput
{
	bool Key_W;
	bool Key_A;
	bool Key_S;
	bool Key_D;
	bool Key_R;
};

struct Bullet
{
	bool in_use;
	obj_type type;
	short ammo;
	Vector3 pos;
};

struct player_packet
{
	WORD			id;
	BYTE			type;
	Vector3			pos;
	Vector3			look;
	Vector3			cameraLook;
	int				hp;
	obj_state		state;
	Bullet			bullet;
};

struct npc_packet
{
	WORD		id;
	BYTE		type;
	Vector3		pos;
	Vector3		look;
	int			hp;
	obj_state	state;
};

struct SC_PACKET
{
	WORD			id;
	BYTE			type;
	player_packet	player[MAX_PLAYER];
	npc_packet		npc[MAX_OBJECT];
};

struct CS_PACKET
{
	BYTE size;
	BYTE type;
	WORD id;
	KeyInput input;
	Vector3 look;
	Vector3 cameraLook;
	bool reload;
};
