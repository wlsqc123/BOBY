#include "stdafx.h"
#include "ServerMgr.h"

using namespace std;
using namespace chrono;

ServerMgr   server_mgr;

HANDLE Event;

void CommThread(SOCKET client_sock);

vector<thread> comm_thread;

void ConnectThread()
{
    printf("Running Connect Thread\n");

    SC_PACKET SCpacket;

    int sockCount = 0;
    int retval;

    server_mgr.Initialize();

    while (true) {
        server_mgr.acceptClient();

        printf("player: %d\n", server_mgr.sockQueue.size());

        SCpacket.type = 10;
        SCpacket.id = sockCount;

        retval = send(server_mgr.sockQueue[sockCount], reinterpret_cast<char*>(&SCpacket), sizeof(SCpacket), 0);

        server_mgr.SetPlayer(sockCount);

        comm_thread.emplace_back(CommThread, server_mgr.sockQueue[sockCount]);

        sockCount++;

        if (sockCount == MAX_PLAYER)
            break;
    }

    printf("Exit ConnectThread\n");

    for (int i = 0; i < MAX_PLAYER; ++i)
        comm_thread[i].join();

    ExitThread(0);
}

void CommThread(SOCKET client_sock)
{
    printf("Running CommThread\n");

    /////////////////////////////////
    SOCKADDR_IN clientaddr;
    int addrlen;
    int retval;
    char ip_addr[40];

    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);
    /////////////////////////////////
    SC_PACKET SCpacket;
    CS_PACKET CSpacket;
    int playerID = 0;


    memset(&CSpacket, 0, sizeof(CSpacket));
    /////////////////////////////////

    while (1)
    {
        retval = WaitForSingleObject(Event, INFINITE);

        retval = recvn(client_sock, reinterpret_cast<char*>(&CSpacket), sizeof(CSpacket), 0);
        if (retval == SOCKET_ERROR) {
            server_mgr.err_display("recvn() err");
            server_mgr.SetHP(playerID, 0);
            server_mgr.SetPos(playerID, { 4000.f ,0.f ,4000.f });
            break;
        }

        playerID = CSpacket.id;
        SCpacket.id = playerID;

        server_mgr.keyInput(CSpacket);


        SCpacket = server_mgr.GetPacket(SCpacket);


        retval = send(client_sock, reinterpret_cast<char*>(&SCpacket), sizeof(SCpacket), 0);
        if (retval == SOCKET_ERROR) {
            server_mgr.err_display("send() err");
            server_mgr.SetHP(playerID, 0);
            server_mgr.SetPos(playerID, { 4000.f ,0.f ,4000.f });
            break;
        }

        SetEvent(Event);/*
        std::this_thread::sleep_for(std::chrono::milliseconds(16));*/
    }

    SetEvent(Event);

    closesocket(client_sock);

    printf("클라이언트 종료: IP 주소:%s, 포트 번호:%d\n",
        inet_ntop(AF_INET, &clientaddr.sin_addr, ip_addr, 40), ntohs(clientaddr.sin_port));

    printf("Exit CommThread\n");
}

void GameThread()
{
    while (true)
    {
        server_mgr.Update();
    }
}

/*
void TimerThreadFunc()
{
    while (true) {
        Sleep(1);
        time_point<system_clock> cur_time = system_clock::now();
        duration<float> elapsed_time = cur_time - prev_time;
        server_framework.Update(elapsed_time);
        server_framework.TimerSend(elapsed_time);
        prev_time = cur_time;
    }
}
*/

int main(int argc, char* argv[])
{
    Event = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (Event == NULL) return 1;

    thread connect_th{ ConnectThread };
    thread game_th{ GameThread };

    connect_th.join();
    game_th.join();
    

    printf("Run Server\n");

    while (1) {
        Sleep(3000);
        printf("X: %f, Y: %f, Z: %f", server_mgr.GetPos(0).x, server_mgr.GetPos(0).y, server_mgr.GetPos(0).z);
    }

    WSACleanup();
    return 0;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       