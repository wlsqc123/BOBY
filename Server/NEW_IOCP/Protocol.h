#pragma once

enum obj_type { type_none, type_npc, type_static, type_player, type_item };
enum obj_state { none, move, attack, hit, dead, interact};

constexpr int max_buffer = 4096;

//monster
constexpr int magmamonster_num = 9;
constexpr int golemmonster_num = 1;
constexpr int orgemonster_num = 8;

//interaction
constexpr int chestobject_num = 6;
constexpr int doorobject_num = 6;
constexpr int leverobject_num = 1;

constexpr int max_object = magmamonster_num + golemmonster_num + orgemonster_num;
constexpr int max_interaction = chestobject_num + doorobject_num + leverobject_num + 1/* + 6*/;
constexpr int max_player = 4;
constexpr int max_name = 16;

enum ITEM_TYPE
{
	ITEM_INSTANT_DEATH, //ü�� 10%���� ���� ���óġ
	ITEM_BLOCK_DAMAGE, //Ȯ���� ���� ����
	ITEM_KILL_MAXHPUP, //���� ���Ͻ� �ִ� ü�»��
	ITEM_MONSTER_SLOW,
	ITEM_DAMAGEUP_MAXHPDOWN, //
	ITEM_PLAYER_SPEEDUP,
	ITEM_BOSS_DAMAGEUP,
	ITEM_ATTACK_SPEEDUP,
	ITEM_MAXHPUP,
	ITEM_EMPTY, //������ȹ�� ������
};


enum WEAPON_TYPE
{
	WEAPON_RIFLE,
	WEAPON_SNIPER,
	WEAPON_SHOTGUN,
};

////////////////////////////////////////

//server to client
constexpr int SC_INGAME_PACKET = 1;
constexpr int SC_SET_ID_PACKET = 2;
constexpr int SC_LOBBY_PACKET = 3;
constexpr int SC_LOBBY_TO_GAME_PACKET = 4;
constexpr int SC_GAME_TO_ENDING_PACKET = 5;


//client to server
constexpr int CS_INGAME_PACKET = 1;
constexpr int CS_LOBBY_PACKET = 2;
constexpr int CS_READY_PACKET = 3;
constexpr int CS_LOGIN_PACKET = 4;
constexpr int CS_SHOOT_PACKET = 5;

/////////////////////////////////////////

struct KeyInput
{
	bool Key_W = false;
	bool Key_A = false;
	bool Key_S = false;
	bool Key_D = false;
	bool Key_R = false;
	bool Key_E = false;
	bool Key_Q = false;
	bool Key_B = false;
	bool Key_N = false;
	bool Key_M = false;
};

struct Bullet
{
	bool in_use;
	obj_type type;
	Vector3 pos;
};

struct player_Status
{
	int hp;
	int max_hp;
	float attack_speed;
};

struct CS_ITEM
{
	int chest_id;
	int item_id;
	bool do_send;
};

struct item_packet
{
	bool is_alive;
	ITEM_TYPE item_type;
};

struct attack_packet
{
	Vector3			pos;
	bool			active_enable;
};

struct player_packet
{
	WORD			id;
	Vector3			pos;
	Vector3			look;
	Vector3			camera_look;
	player_Status	ps;
	obj_state		state;
	Bullet			bullet[5];
	bool			reload_enable;
	short			ammo;
	ITEM_TYPE		current_item;
	int				zone_num;
};

struct npc_info
{
	WORD		id;
	Vector3		pos;
	Vector3		look;
	int			hp;
	bool		attack_enable;
	obj_state	state;
};

struct interaction_packet
{
	WORD		id;
	Vector3		pos;
	Vector3		look;
	obj_state	state;
	item_packet   item[4];
	bool		interact_enable;
};

struct sc_ingame_packet
{
	int					size;
	int					type;
	int					id;
	player_packet		player[max_player];
	npc_info			npc[max_object];
	interaction_packet  interaction[max_interaction];
	attack_packet		attack[20];
	attack_packet		stone[10];
	int					play_time;
};

struct cs_ingame_packet
{
	int size;
	int type;
	int id;
	KeyInput input;
	Vector3 look;
	Vector3 cameraLook;
	CS_ITEM item;
};

struct lobby_info
{
	char name[max_name];
	bool ready;
};

struct sc_lobby_packet
{
	int size;
	int type;
	lobby_info info[max_player];
};

struct cs_lobby_packet
{
	int size;
	int type;
	int r_id;
	char name[max_name];
	bool ready;
	WEAPON_TYPE weapon_type;
};


