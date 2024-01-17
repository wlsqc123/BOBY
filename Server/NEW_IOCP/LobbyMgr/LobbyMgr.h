#pragma once
#include "../GameMgr/GameMgr.h"
#include <sqlext.h>

//#define SERVERIP	"211.252.22.159"
//#define SERVERPORT	6112
#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000

#define BUFSIZE		512

#define SERVERID 0

using namespace std;

class LobbyMgr
{
	//Server
private:
	int retval;
	int addrlen;
	char buf[BUFSIZE + 1];

	WSADATA		wsa;
	SOCKET		sock;
	SOCKET		listen_sock;
	SOCKET		client_sock;
	SOCKADDR_IN	serveraddr;

	vector<int> g_list;

	//DB
private:
	char user_id[max_name];
	char db_buf[100];
	int user_time;

	// Connect to DB
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szUser_Name[max_name];
	SQLINTEGER dUser_time, dUser_level;

	SQLLEN cbID = 0;

	SQLCHAR* OutConnStr = (SQLCHAR*)malloc(255);
	SQLSMALLINT* OutConnStrLen = (SQLSMALLINT*)malloc(255);

public:
	LobbyMgr();
	~LobbyMgr();

	static void err_quit(const char* msg);
	static void err_display(char* msg);

	char								ip_addr[40];
	SOCKADDR_IN							clientaddr;

	cs_ingame_packet							CSpacket;
	sc_ingame_packet							SCpacket;

	mutex rm_l;
	



	chrono::system_clock::time_point	deltaTime;
	chrono::system_clock::time_point	currentTime;


	static void Initialize();
	static void acceptClient();
	static void disconnect(int p_id);
	static void display_error(const char* msg, int err_no);

	void g_worker(HANDLE h_iocp, SOCKET l_socket);

	static void do_recv(int key);
	static void do_send(int p_id, void* p);
	void process_packet(int p_id, unsigned char* p_buf);
	static int get_new_player_id(SOCKET p_socket);
	static int get_new_room_id(int p_id);
	static sc_lobby_packet get_packet(int r_id);

	void Update();

	array<GameMgr, 100> arr_game;

public:
	void	db_connect();
	void	DB_update(char name[max_name], int time);
	int		DB_get_time(char name[max_name]);
};

