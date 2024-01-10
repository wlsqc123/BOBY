#pragma once
#include "stdafx.h"
#include "mapData.h"

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		512

#define PLAYER_SPEED 10
#define PLAYER_ATTCAKSPEED 0.15
#define SERVERID 0

using namespace std;

class ServerMgr
{
private:
	int retval;
	int addrlen;
	char buf[BUFSIZE + 1];


	WSADATA		wsa;
	SOCKET		sock;
	SOCKET		listen_sock;
	SOCKET		client_sock;
	SOCKADDR_IN	serveraddr;


	vector<Structure> structure;

	vector<BoundingOrientedBox> map_lava;

	vector<vector<int>> zone;


public:
	ServerMgr();
	~ServerMgr();

	void err_quit(char* msg);
	void err_display(char* msg);

	Player		player[MAX_PLAYER];
	vector<NPC> npc;
	vector<INTERACTION> interaction;
	vector<RangeAttack> rangeAttack;

	char								ip_addr[40];
	SOCKADDR_IN							clientaddr;
	std::vector<SOCKET>					sockQueue;

	CS_PACKET							CSpacket;
	SC_PACKET							SCpacket;

	chrono::system_clock::time_point	deltaTime;
	UINT								reboundCount = 0;
	chrono::system_clock::time_point	currentTime;

	bool								isSlow = false;
	chrono::system_clock::time_point	slowTime;

	void Worker(HANDLE h_iocp, SOCKET l_socket);

	void Initialize();
	void acceptClient();
	void disconnect(int p_id);
	void display_error(const char* msg, int err_no);
	int get_new_player_id(SOCKET p_socket);

	void do_recv(int key);
	void do_send(int p_id, void* p);


	void Update();
	void TracePlayer(int n_id);
	void AttackPlayer(int n_id);

	void CheckPlayerDead(int p_id);

	void keyInput(CS_PACKET cspacket);
	void process_packet(int p_id, unsigned char* p_buf);

	void Player_CollCheck(int id);
	void NPC_CollCheck(int id);

	//bullet collision
	bool CollideObjectByRayIntersection(BoundingOrientedBox	objectBoundingBox, Vector3& position, Vector3& direction, float* distance);
	void FindCollideObject(int p_id);
	void FindCollideObjectShotGun(int p_id);
	void PickInteractionObject(int p_id);
	void CheckinteractionObject(int p_id);

	void SetPlayer(int id) { player[id].ps.hp = 100; player[id].state = none; player[id].CurPos.x -= 2000; player[id].CurPos.z -= 2000; };
	void SetHP(int id, int hp);
	void SetPos(int id, XMFLOAT3 pos);
	void SetItem(int id, ITEM_TYPE item);


	Vector3 GetPositionToHeightMap(float x, float z, float addy);


	SC_PACKET	GetPacket(SC_PACKET scpacket);
	SOCKET		GetlistenSock()	{return(listen_sock);};
	XMFLOAT3	GetPos(int id) { return player[id].CurPos; };
	XMFLOAT3	GetLook(int id) { return player[id].pl_look; };
	XMFLOAT3	GetCameraLook(int id) { return player[id].cam_look; };
	XMFLOAT3	GetNPC(int id) { return npc[id].CurPos; };
	float		GetHP(int id) { return player[id].ps.hp; };

public:
	CHeightMapImage* mapData;

};

