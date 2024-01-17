#pragma once

enum ObjectType
{
    NoneType,
    NPCType,       // NPC
    StaticType,    // 정적 객체
    PlayerType,    // 플레이어
    ItemType       // 아이템
};

enum ObjectState
{
    NoneState,
    Moving,     // 이동중
    Attacking,  // 공격중
    Hit,        // 피격
    Dead,       // 사망
    Interacting // 상호작용
};

constexpr int MAX_BUFFER = 4096; // 버퍼 최대 크기

//monster
constexpr int NUM_MAGMA_MONSTERS = 9; // 마그마 수
constexpr int NUM_GOLEM_MONSTER = 1; // 골렘 수
constexpr int NUM_ORGE_MONSTER = 8;  // 오거 수

//interaction
constexpr int NUM_CHEST_OBJECT = 6;
constexpr int NUM_DOOR_OBJECT = 6;
constexpr int NUM_LEVER_OBJECT = 1;

constexpr int MAX_NUM_OBJECT = NUM_MAGMA_MONSTERS + NUM_GOLEM_MONSTER + NUM_ORGE_MONSTER;
constexpr int MAX_NUM_INTERACTION = NUM_CHEST_OBJECT + NUM_DOOR_OBJECT + NUM_LEVER_OBJECT + 1;
constexpr int MAX_NUM_PLAYER = 4;
constexpr int MAX_NUM_NAME = 16;

enum ITEM_TYPE
{
    ItemInstantDeath,        // 즉사 아이템
    ItemBlockDamage,         // 데미지 차단 아이템
    ItemKillMaxHpUp,         // 최대 HP 증가 아이템
    ItemMonsterSlow,         // 몬스터 속도 감소 아이템
    ItemDamageUpMaxHpDown,   // 데미지 증가 및 최대 HP 감소 아이템
    ItemPlayerSpeedUp,       // 플레이어 속도 증가 아이템
    ItemBossDamageUp,        // 보스 데미지 증가 아이템
    ItemAttackSpeedUp,       // 공격 속도 증가 아이템
    ItemMaxHpUp,             // 최대 HP 증가 아이템
    ItemEmpty,               // 빈 아이템
    ItemTypeCount            // 아이템 타입 개수
};

enum WEAPON_TYPE
{
	WeaponRifle,
	WeaponSniper,
	WeaponShotgun,
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
	bool isInUse;
	ObjectType type;
	Vector3 position;
};

struct PlayerStatus
{
	int healthPoints;
	int maxHealthPoints;
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
	Vector3			position;
	bool			isActive;
};

struct player_packet
{
	WORD			id;
	Vector3			position;
	Vector3			lookDirection;
	Vector3			cameraLook;
	PlayerStatus	status;
	ObjectState		state;
	Bullet			bullet[5];
	bool			isReloading;
	short			ammoCount;
	ITEM_TYPE		currentItem;
	int				zoneNumber;
};

struct NPCInfo
{
	WORD		id;
	Vector3		position;
	Vector3		lookDirection;
	int			healthPoints;
	bool		isAttack;
	ObjectState	state;
};

struct InteractionPacket
{
	WORD		id;
	Vector3		position;
	Vector3		lookDirection;
	ObjectState	state;
	item_packet items[4];
	bool		isInteracting;
};

struct sc_ingame_packet
{
	int					size;
	int					type;
	int					playerId;
	player_packet		players[MAX_NUM_PLAYER];
	NPCInfo			    npcs[MAX_NUM_OBJECT];
	InteractionPacket   interactions[MAX_NUM_INTERACTION];
	attack_packet		attacks[20];
	attack_packet		stones[10];
	int					playTime;
};

struct cs_ingame_packet
{
	int         size;
	int         type;
	int         playerId;
	KeyInput    input;
	Vector3     lookDirection;
	Vector3     cameraLook;
	CS_ITEM     item;
};

struct lobby_info
{
	char name[MAX_NUM_NAME];
	bool isReady;
};

struct sc_lobby_packet
{
	int         size;
	int         type;
	lobby_info  infos[MAX_NUM_PLAYER];
};

struct cs_lobby_packet
{
	int         size;
	int         type;
	int         roomId;
	char        name[MAX_NUM_NAME];
	bool        isReady;
	WEAPON_TYPE weaponType;
};


