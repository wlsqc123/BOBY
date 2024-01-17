#pragma once


#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <directxmath.h>
#include "stdafx.h"
#include "..\Server\NEW_IOCP\Protocol.h"

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000

#define WM_SOCKET	WM_USER + 1

using namespace DirectX;
using namespace DirectX::PackedVector;


class ServerMgr
{
private:

	int retval;
	int recvn(SOCKET s, char* buf, int len, int flags);

	WSADATA		wsa;
	SOCKET		sock;
	SOCKADDR_IN serveraddr;
	HWND async_handle;


public:
	ServerMgr();
	~ServerMgr();

	bool state;

	KeyInput			key_input;
	sc_ingame_packet	SCpacket_ingame;
	cs_ingame_packet	CSpacket_ingame;

	sc_lobby_packet		SCpacket_lobby;
	cs_lobby_packet		CSpacket_lobby;

	int myID;

	bool id_init = false;
	bool connectEnable = false;

	void err_display(char* msg);
	void Initialize(HWND& hwnd);
	void ProcessPacket();
	void RecvPacket(); // 0:lobby, 1:ingame
	void SendPacket(); // 0:lobby, 1:ingame
};