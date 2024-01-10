#pragma once
#include "stdafx.h"
#include "mapData.h"

class GameMgr
{
private:
	vector<NPC>							npc;

	chrono::system_clock::time_point	deltaTime;
	chrono::system_clock::time_point	currentTime;

	vector<Structure>					structure;
	vector<BoundingOrientedBox>			map_lava;
	vector<INTERACTION>					interaction;
	vector<ZONE>						gameZones;
	vector<RangeAttack>					rangeAttack;
	vector<RangeAttack>					stoneAttack;

	bool								isSlow = false;
	chrono::system_clock::time_point	slowTime;
	int zoneLevel;
	chrono::system_clock::time_point	stoneTime;
public:
	CHeightMapImage* mapData;
	bool	isRunning = false;
	bool    isEnding = false;
	
	GameMgr();
	~GameMgr();

	int r_id;
	int pl_list[MAX_PLAYER];

	void InitGame(int id[4]);


	void Update();
	void TracePlayer(int n_id);
	void AttackPlayer(int n_id);

	void CheckPlayerDead(int p_id);

	void keyInput(cs_ingame_packet cspacket);
	void process_packet(int p_id, unsigned char* p_buf);

	void Player_CollCheck(int id);
	void NPC_CollCheck(int id);

	//bullet collision
	bool CollideObjectByRayIntersection(BoundingOrientedBox objectBoundingBox, Vector3& position, Vector3& direction, float* distance);
	void FindCollideObject(int p_id);
	void FindCollideObjectShotGun(int p_id);
	void PickInteractionObject(int p_id);
	void CheckinteractionObject(int p_id);


	void SetHP(int id, int hp);
	void SetPos(int id, XMFLOAT3 pos);
	void SetItem(int id, ITEM_TYPE item);


	Vector3 GetPositionToHeightMap(float x, float z, float addy);

	sc_ingame_packet	GetPacket(sc_ingame_packet packet);
	XMFLOAT3	GetPos(int id) { return arr_player[id].CurPos; };
	XMFLOAT3	GetLook(int id) { return arr_player[id].pl_look; };
	XMFLOAT3	GetCameraLook(int id) { return arr_player[id].cam_look; };
	XMFLOAT3	GetNPC(int id) { return npc[id].CurPos; };
	float		GetHP(int id) { return arr_player[id].ps.hp; };
	int			GetID(int id) { return r_id; };

	chrono::system_clock::time_point cur_update_time = chrono::system_clock::now();
	chrono::system_clock::time_point s_time;
	chrono::system_clock::time_point e_time;
};

