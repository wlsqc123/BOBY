#pragma once
#include "stdafx.h"
#include "Protocol.h"

#include "mapData.h"

#define SERVERIP	"192.168.131.1"
#define SERVERPORT	9000
#define BUFSIZE		512

#define PLAYER_SPEED 10

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

	Player		player[MAX_PLAYER];

	vector<NPC> npc;
	vector<Structure> structure;

	vector<BoundingOrientedBox> map_lava;


public:
	ServerMgr();
	~ServerMgr();

	void err_quit(char* msg);
	void err_display(char* msg);


	char								ip_addr[40];
	SOCKADDR_IN							clientaddr;
	std::vector<SOCKET>					sockQueue;

	CS_PACKET							CSpacket;
	SC_PACKET							SCpacket;

	chrono::system_clock::time_point	deltaTime;
	UINT								reboundCount = 0;
	chrono::system_clock::time_point	currentTime;

	void Initialize();
	void acceptClient();
	
	void RecvPacket(SOCKET s);
	void SendPacket(SOCKET s);

	void Update();
	void TracePlayer(int n_id);
	void AttackPlayer(int n_id);

	void CheckPlayerDead(int p_id);

	void keyInput(CS_PACKET cspacket);
	void ProcessPacket();	

	void Player_CollCheck(int id);
	void NPC_CollCheck(int id);

	//bullet collision
	bool CollideObjectByRayIntersection(BoundingOrientedBox	objectBoundingBox, Vector3& position, Vector3& direction, float* distance);
	void PickObjectByCameraLookVector(int p_id);

	void SetPlayer(int id) { player[id].hp = 100; player[id].state = none; player[id].CurPos.x -= 2000; player[id].CurPos.z -= 2000; };
	void SetHP(int id, int hp);
	void SetPos(int id, XMFLOAT3 pos);



	SC_PACKET	GetPacket(SC_PACKET scpacket);
	SOCKET		GetlistenSock()	{return(listen_sock);};
	XMFLOAT3	GetPos(int id) { return player[id].CurPos; };
	XMFLOAT3	GetLook(int id) { return player[id].look; };
	XMFLOAT3	GetCameraLook(int id) { return player[id].camera_look; };
	XMFLOAT3	GetNPC(int id) { return npc[id].CurPos; };
	float		GetHP(int id) { return player[id].hp; };

public:
	CHeightMapImage* mapData;

};

