#include "LobbyMgr.h"
#include "iostream"
#include "MSWSock.h"
#include "WS2tcpip.h"

LobbyMgr::LobbyMgr():retval(0),addrlen(0),buf{},wsa(),sock(0),listen_sock(0),client_sock(0),serveraddr(),user_id{},db_buf{},user_time(0),henv(nullptr),hdbc(nullptr),retcode(0),szUser_Name{},dUser_time(0),dUser_level(0),ip_addr{},clientaddr(),CSpacket(), SCpacket()
{
    for (int i = 0; i < MAX_USERS; ++i) {
        arr_player[i].ready = false;
        arr_player[i].m_state = PLST_FREE;
    }

    for (int r_id = 0; r_id < MAX_ROOM; ++r_id) {
        for (int p_id = 0; p_id < MAX_NUM_PLAYER; ++p_id)
            //strcpy(arr_lobby[r_id].pl[p_id].name);
            ;
    }
}

LobbyMgr::~LobbyMgr()
{
}

void LobbyMgr::g_worker(HANDLE h_iocp, SOCKET l_socket)
{
    cout << "Running Worker Thread" << endl;

    while (true) {
        DWORD num_bytes;
        ULONG_PTR ikey;
        WSAOVERLAPPED* over;

        BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes,
            &ikey, &over, INFINITE);

        int key = static_cast<int>(ikey);
        if (FALSE == ret) {
            if (SERVERID == key) {
                //display_error("GQCS : ", WSAGetLastError());
                //exit(-1);
            }
            else {
                display_error("GQCS : ", WSAGetLastError());
                disconnect(key);
            }
        }
        if ((key != SERVERID) && (0 == num_bytes)) {
            //disconnect(key);
            continue;
        }
        EX_OVER* ex_over = reinterpret_cast<EX_OVER*>(over);

        switch (ex_over->m_op) {
        case OP_RECV: {
            unsigned char* packet_ptr = ex_over->m_packetbuf;
            int num_data = num_bytes + arr_player[key].m_prev_size;
            int packet_size = packet_ptr[0];

            while (num_data >= packet_size) {
                process_packet(key, packet_ptr);
                num_data -= packet_size;
                packet_ptr += packet_size;
                if (0 >= num_data) break;
                packet_size = packet_ptr[0];
            }
            arr_player[key].m_prev_size = num_data;
            if (0 != num_data)
                memcpy(ex_over->m_packetbuf, packet_ptr, num_data);
            do_recv(key);

            break;
        }
        case OP_SEND:
        {
            //
            delete ex_over;
            break;
        }
        case OP_ACCEPT:
        {
            //
            int c_id = get_new_player_id(ex_over->m_csocket);
            if (-1 != c_id) {
                arr_player[c_id].m_recv_over.m_op = OP_RECV;
                arr_player[c_id].m_prev_size = 0;
                CreateIoCompletionPort(
                    reinterpret_cast<HANDLE>(arr_player[c_id].m_socket), h_iocp, c_id, 0);

                //arr_player[c_id].CurPos.x -= 2000.f;
                //arr_player[c_id].CurPos.z -= 2000.f;

                
                sc_ingame_packet scpacket;
                scpacket.playerId = c_id;
                scpacket.type = SC_SET_ID_PACKET;
                do_recv(c_id);
            }
            else {
                closesocket(arr_player[c_id].m_socket);
            }

            memset(&ex_over->m_over, 0, sizeof(ex_over->m_over));
            SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            ex_over->m_csocket = c_socket;
            AcceptEx(l_socket, c_socket,
                ex_over->m_packetbuf, 0, 32, 32, NULL, &ex_over->m_over);

        }
            break;
        }
    }
}

void LobbyMgr::Initialize()
{
}

void LobbyMgr::acceptClient()
{
}

void LobbyMgr::disconnect(int p_id)
{
    {
        lock_guard <mutex> gl{ arr_player[p_id].m_slock };
        if (arr_player[p_id].m_state = PLST_FREE) return;
        closesocket(arr_player[p_id].m_socket);
        arr_player[p_id].m_state = PLST_FREE;
    }
    for (auto& pl : arr_player) {
        lock_guard<mutex> gl2{ pl.m_slock };
        if (PLST_INGAME == pl.m_state)
            cout << "remove: " << pl.id << endl;
            //send_remove_object(pl.id, p_id);
    }
}

void LobbyMgr::display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    cout << msg;
    wcout << lpMsgBuf << endl;
    LocalFree(lpMsgBuf);
}

void LobbyMgr::do_recv(int key)
{
    arr_player[key].m_recv_over.m_wsabuf[0].buf =
        reinterpret_cast<char*>(arr_player[key].m_recv_over.m_packetbuf)
        + arr_player[key].m_prev_size;
    arr_player[key].m_recv_over.m_wsabuf[0].len = MAX_BUFFER - arr_player[key].m_prev_size;
    memset(&arr_player[key].m_recv_over.m_over, 0, sizeof(arr_player[key].m_recv_over.m_over));
    DWORD r_flag = 0;
    int ret = WSARecv(arr_player[key].m_socket, arr_player[key].m_recv_over.m_wsabuf, 1,
        NULL, &r_flag, &arr_player[key].m_recv_over.m_over, NULL);
    if (0 != ret) {
        int err_no = WSAGetLastError();
        if (WSA_IO_PENDING != err_no) {
            display_error("WSARecv : ", WSAGetLastError());
            cout << "err_no: " << err_no << endl;
        }
    }

}

void LobbyMgr::do_send(int p_id, void* p)
{
    int p_size = reinterpret_cast<int*>(p)[0];
    int p_type = reinterpret_cast<int*>(p)[1];
    //cout << "To client [" << p_id << "] : ";
    //cout << "Packet [" << p_type << "]\n";
    EX_OVER* s_over = new EX_OVER;
    s_over->m_op = OP_SEND;
    memset(&s_over->m_over, 0, sizeof(s_over->m_over));
    memcpy(s_over->m_packetbuf, p, p_size);
    s_over->m_wsabuf[0].buf = reinterpret_cast<CHAR*>(s_over->m_packetbuf);
    s_over->m_wsabuf[0].len = p_size;
    int ret = WSASend(arr_player[p_id].m_socket, s_over->m_wsabuf, 1,
        NULL, 0, &s_over->m_over, 0);
    if (0 != ret) {
        int err_no = WSAGetLastError();
        if (WSA_IO_PENDING != err_no) {
            display_error("WSASend : ", WSAGetLastError());
            disconnect(p_id);
        }
    }
}

void LobbyMgr::process_packet(int p_id, unsigned char* p_buf)
{
    switch (arr_player[p_id].m_state) {
    case PLST_FREE:
        break;
    case PLST_CONNECTED: {
        cs_lobby_packet* cspacket = reinterpret_cast<cs_lobby_packet*>(p_buf);

        // 플레이어 닉네임을 받아서 저장.
        strcpy(arr_player[p_id].name, cspacket->name);
        int r_id = 0; // room id를 찾아야 함

        arr_player[p_id].r_id = r_id;
        arr_player[p_id].id = p_id;
        DB_get_time(arr_player[p_id].name);

        if (arr_lobby[r_id].pl.size() < MAX_NUM_PLAYER) {
            LOBBY_PLAYER_INFO info;
            strcpy(info.name, arr_player[p_id].name);
            info.ready = false;
            info.id = p_id;
            arr_lobby[r_id].pl.emplace_back(info);

            sc_lobby_packet packet;

            packet = get_packet(r_id);

            for (auto& au : arr_lobby[r_id].pl) {
                do_send(au.id, &packet);
            } // 다른 플레이어에게 접속을 알려야함. 미구현. 패킷에는 포함되어 있음.
        }

        arr_player[p_id].m_state = PLST_INLOBBY;
        break;
    }
    case PLST_INLOBBY: {
        cs_lobby_packet* cspacket = reinterpret_cast<cs_lobby_packet*>(p_buf);
        sc_lobby_packet scpacket;

        int r_id = arr_player[p_id].r_id;
        arr_lobby[r_id].pl[p_id].ready = cspacket->isReady;     
        arr_player[p_id].ready = cspacket->isReady;
        arr_player[p_id].wp_type = cspacket->weaponType;
        scpacket = get_packet(r_id);
        
        if (arr_lobby[r_id].pl.size() == MAX_NUM_PLAYER &&
            arr_lobby[r_id].pl[0].ready == true && arr_lobby[r_id].pl[1].ready == true
            && arr_lobby[r_id].pl[2].ready == true && arr_lobby[r_id].pl[3].ready == true) {
            cout << " all ready " << endl;
            scpacket.type = SC_LOBBY_TO_GAME_PACKET;

            int id[MAX_NUM_PLAYER] = { arr_lobby[r_id].pl[0].id, arr_lobby[r_id].pl[1].id,
            arr_lobby[r_id].pl[2].id, arr_lobby[r_id].pl[3].id };


            int count = 0;

            for (auto& au : arr_lobby[r_id].pl) {
                do_send(au.id, &scpacket);

                arr_player[au.id].m_state = PLST_INGAME;

                sc_ingame_packet set_id_packet;
                set_id_packet.playerId = count;
                set_id_packet.type = SC_SET_ID_PACKET;
                set_id_packet.size = sizeof(sc_ingame_packet);
                do_send(au.id, &set_id_packet);
                cout << "send set_id_packet: " << au.id << endl;

                ++count;
            }
            
            arr_game[r_id].InitGame(id);
            arr_game[r_id].startTime = chrono::system_clock::now();
        }
        else
            do_send(p_id, &scpacket);

        break;
    }
    case PLST_INGAME:
        sc_ingame_packet scpacket;

        int r_id = arr_player[p_id].r_id;

        arr_game[r_id].ProcessPacket(p_id, p_buf);

        scpacket = arr_game[r_id].GetPacket(scpacket);
        
        do_send(p_id, &scpacket);

        break;
    }
}

int LobbyMgr::get_new_player_id(SOCKET p_socket)
{
    for (int i = 0; i <= MAX_USERS; ++i) {
        lock_guard<mutex> lg{ arr_player[i].m_slock };
        if (PLST_FREE == arr_player[i].m_state) {
            arr_player[i].m_state = PLST_CONNECTED;
            arr_player[i].m_socket = p_socket;
            arr_player[i].name[0] = 0;
            return i;
        }
    }

    return -1;
}

int LobbyMgr::get_new_room_id(int p_id)
{
    return -1;
}

sc_lobby_packet LobbyMgr::get_packet(int r_id)
{
    sc_lobby_packet packet;

    for (auto& au : arr_lobby[r_id].pl) {
        strcpy(packet.infos[au.id].name, au.name);
        packet.size = sizeof(sc_lobby_packet);
        packet.type = SC_LOBBY_PACKET;
        packet.infos->isReady = arr_player[au.id].ready;
    }

    return packet;
}

void LobbyMgr::Update()
{
    while (true) {
        /*for (int i = 0; i < g_list.size(); ++i) {
            if (chrono::system_clock::now() - arr_game[i].cur_update_time > 16ms)
                arr_game[i].Update();
        }*/
        for (auto& p : arr_game)
        {
            if (!p.isRunning) continue;
            if (chrono::system_clock::now() - p.currentUpdateTime > 16ms)
                p.Update();
        }
    }
}

void LobbyMgr::DB_connect()
{
    // Allocate environment handle  
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLCHAR*)"veles_db_odbc", SQL_NTS, (SQLCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

                    cout << "ODBC connect success " << endl;
                }
                else
                    cout << "ODBC connect failed " << endl;
            }
        }
    }

}

void LobbyMgr::DB_update(char name[MAX_NUM_NAME], int time)
{
    sprintf_s(db_buf, sizeof(db_buf), "EXEC update_time %s, %d", user_id, user_time);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*) db_buf, SQL_NTS);

    cout << "retcode: " << retcode << endl;
}

int LobbyMgr::DB_get_time(char name[MAX_NUM_NAME])
{
    sprintf_s(db_buf, sizeof(db_buf), "EXEC select_time %s", name);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*) db_buf, SQL_NTS);
    if (retcode == SQL_ERROR) {
        cout << "select_time err" << endl;
    }

    int test = 140;
    sprintf_s(db_buf, sizeof(db_buf), "EXEC update_time %s, %d", name, test);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*) db_buf, SQL_NTS);
    if (retcode == SQL_ERROR) {
        cout << "update_time err" << endl;  
        cout << db_buf << endl;
    }


    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        cout << "Get_time Success: "  << endl;

        retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &dUser_time, 100, &cbID);
        retcode = SQLFetch(hstmt);

        cout << "Time: " << dUser_time << endl;
    }


    return dUser_time;
}

void LobbyMgr::err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void LobbyMgr::err_display(char* msg)
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
