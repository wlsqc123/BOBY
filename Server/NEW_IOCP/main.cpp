#pragma once
#include "./LobbyMgr/LobbyMgr.h"
#include <winsock2.h>

using namespace std;
using namespace chrono;

LobbyMgr	lobby_mgr;
HANDLE		h_iocp;

void AI()
{
	lobby_mgr.Update();
}

void lobby(HANDLE h_iocp, SOCKET l_socket)
{
    lobby_mgr.g_worker(h_iocp, l_socket);
}


int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	wcout.imbue(locale("korean"));
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, SERVERID, 0);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	EX_OVER accept_over;
	accept_over.m_op = op_accept;
	memset(&accept_over.m_over, 0, sizeof(accept_over.m_over));
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	accept_over.m_csocket = c_socket;
	BOOL ret = AcceptEx(listenSocket, c_socket,
		accept_over.m_packetbuf, 0, 32, 32, NULL, &accept_over.m_over);
	if (FALSE == ret) {
		int err_num = WSAGetLastError();
		if (err_num != WSA_IO_PENDING)
			cout << "Accept Error" << endl;
	}

	// Connect to DB
	lobby_mgr.db_connect();

	vector <thread> worker_threads;
	for (int i = 0; i < 1; ++i)
		worker_threads.emplace_back(lobby, h_iocp, listenSocket);

	thread AI_thread{AI};

	for (auto& th : worker_threads)
		th.join();

	AI_thread.join();
 
	closesocket(listenSocket);
	WSACleanup();
}