#include "stdafx.h"
#include "ServerMgr.h"



//#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����




ServerMgr::ServerMgr()
{
    state = 0;
}

ServerMgr::~ServerMgr()
{
}

int ServerMgr::recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

void ServerMgr::ProcessPacket()
{
    if (SCpacket_ingame.type == SC_SET_ID_PACKET) {
        myID = SCpacket_ingame.id;
        CSpacket_ingame.id = myID;
    }
}


void ServerMgr::err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}


void ServerMgr::Initialize(HWND& hwnd)
{
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        printf("WSAStartup Error\n");


    // socket()
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_display("socket()");

    // connect()
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_display("connect()");
    else connectEnable = true;

    async_handle = hwnd;


    WSAAsyncSelect(sock, async_handle, WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ);


}

void ServerMgr::RecvPacket()
{
    if (state == 0) {
        retval = recvn(sock, reinterpret_cast<char*>(&SCpacket_lobby), sizeof(SCpacket_lobby), 0);
        if (retval == SOCKET_ERROR) {
            cout << "recv error" << endl;
        }
    }

    // INGAME
    if (state == 1) {
        retval = recvn(sock, reinterpret_cast<char*>(&SCpacket_ingame), sizeof(SCpacket_ingame), 0);
        if (retval == SOCKET_ERROR) {
            cout << "recv error" << endl;
        }
    }
}

void ServerMgr::SendPacket()
{
    if (state == 0) {
        CSpacket_lobby.size = sizeof(cs_lobby_packet);

        retval = send(sock, reinterpret_cast<char*>(&CSpacket_lobby), sizeof(CSpacket_lobby), 0);
        if (retval == SOCKET_ERROR) {
            cout << "send error" << endl;
        }
    }

    // INGAME
    if (state == 1) {
        CSpacket_ingame.size = sizeof(cs_ingame_packet);

        retval = send(sock, reinterpret_cast<char*>(&CSpacket_ingame), sizeof(CSpacket_ingame), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            cout << "send error" << endl;          
        }
    }

}



