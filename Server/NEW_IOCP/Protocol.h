#pragma once

enum obj_type { type_none, type_npc, type_static, type_player, type_item };
enum obj_state { none, move, attack, hit, dead, interaction};

constexpr int MAX_BUFFER = 4096;

//monster
constexpr int MAGMAMONSTER_NUM = 9;
constexpr int GOLEMMONSTER_NUM = 1;
constexpr int ORGEMONSTER_NUM = 8;

//interaction
constexpr int CHESTOBJECT_NUM = 6;
constexpr int DOOROBJECT_NUM = 6;
constexpr int LEVEROBJECT_NUM = 1;

constexpr int MAX_OBJECT = MAGMAMONSTER_NUM + GOLEMMONSTER_NUM + ORGEMONSTER_NUM;
constexpr int MAX_INTRACTION = CHESTOBJECT_NUM + DOOROBJECT_NUM + LEVEROBJECT_NUM + 1/* + 6*/;
constexpr int MAX_PLAYER = 4;
constexpr int MAX_NAME = 16;

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
	int maxHp;
	float attackSpeed;
};

struct CS_ITEM
{
	int chestId;
	int itemId;
	bool doSend;
};

struct item_packet
{
	bool isAlive;
	ITEM_TYPE itemType;
};

struct attack_packet
{
	Vector3			pos;
	bool			activeEnable;
};

struct player_packet
{
	WORD			id;
	Vector3			pos;
	Vector3			look;
	Vector3			cameraLook;
	player_Status	ps;
	obj_state		state;
	Bullet			bullet[5];
	bool			reloadEnable;
	short			ammo;
	ITEM_TYPE		currentItem;
	int				zoneNum;
};

struct npc_info
{
	WORD		id;
	Vector3		pos;
	Vector3		look;
	int			hp;
	bool		attackEnable;
	obj_state	state;
};

struct interaction_packet
{
	WORD		id;
	Vector3		pos;
	Vector3		look;
	obj_state	state;
	item_packet   item[4];
	bool		interactEnable;
};

struct sc_ingame_packet
{
	int					size;
	int					type;
	int					id;
	player_packet		player[MAX_PLAYER];
	npc_info			npc[MAX_OBJECT];
	interaction_packet  interaction[MAX_INTRACTION];
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
	char name[MAX_NAME];
	bool ready;
};

struct sc_lobby_packet
{
	int size;
	int type;
	lobby_info info[MAX_PLAYER];
};

struct cs_lobby_packet
{
	int size;
	int type;
	int r_id;
	char name[MAX_NAME];
	bool ready;
	WEAPON_TYPE weapon_type;
};


