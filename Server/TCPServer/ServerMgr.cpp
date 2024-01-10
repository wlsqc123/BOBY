#include "stdafx.h"
#include "ServerMgr.h"

ServerMgr::ServerMgr()
{
    mapData = new CHeightMapImage(_T("mapImage/terrain3.raw"), 513, 513, Vector3(7.5f, 10.0f, 7.5f));
    //mapData = new CHeightMapImage( _T("mapImage/terrain5.raw"), 513, 513, Vector3(20.f, 5.0f, 20.f));

    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;


    //PLAYER
    {
        for (int i = 0; i < MAX_PLAYER; ++i) {
            posX = float(1700 + (i * ((2300 - 1700) / MAX_PLAYER)));
            posZ = 2050.f;
            posY = mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 90.f;
            Vector3 pos{ posX, posY,  posZ };
            player[i].InitPos = pos;

            pos.x += 2000.f; pos.z += 2000.f;
            player[i].CurPos = pos;
            player[i].PrevPos = pos;
            player[i].hp = 100;
            player[i].speed = PLAYER_SPEED;
            player[i].state = dead;

            Vector3 lookvec = { posX - 2050.f, 0, posZ - 1500.f };
            lookvec = lookvec.normalized();
            player[i].look = lookvec;
            player[i].OOBB = BoundingOrientedBox(player[i].CurPos, XMFLOAT3(OBB_SCALE_PLAYER_X, OBB_SCALE_PLAYER_Y, OBB_SCALE_PLAYER_Z), XMFLOAT4(0, 0, 0, 1));
            player[i].bullet.in_use = false;
            player[i].bullet.type = type_none;
            player[i].bullet.ammo = 30;
            player[i].camera_look = { 0, 1, 0 };

            player[i].state = none;
        }
    }

    //NPC
    {
        npc.reserve(MAX_OBJECT);

        for (int i = 0; i < MAGMAMONSTER_NUM; ++i) {
            NPC new_npc;
            new_npc.mob = MAGMA;
            new_npc.OOBB.Extents = { OBB_SCALE_Magmaa_X, OBB_SCALE_Magmaa_Y, OBB_SCALE_Magmaa_Z };
            posX = float(rand() % 2000 + 250);
            posZ = float(rand() % 2000 + 250);
            new_npc.CurPos = { posX, mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 80, posZ  };
            new_npc.InitPos = new_npc.CurPos;
            new_npc.speed = (float(rand() % 10) + 1.0f) / 10.0f;
            new_npc.hp = 300;
            npc.emplace_back(new_npc);
        }
        for (int i = 0; i < GOLEMMONSTER_NUM; ++i) {
            NPC new_npc;
            new_npc.mob = GOLEM;
            new_npc.OOBB.Extents = { OBB_SCALE_Golem_X, OBB_SCALE_Golem_Y, OBB_SCALE_Golem_Z };
            posX = 750.f;
            posZ = 600.f;
            new_npc.CurPos = { posX, mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 80, posZ };
            new_npc.InitPos = new_npc.CurPos;
            new_npc.speed = (float(rand() % 10) + 1.0f) / 10.0f;
            new_npc.hp = 3000;
            npc.emplace_back(new_npc);
        }
        for (int i = 0; i < ORGEMONSTER_NUM; ++i) {
            NPC new_npc;
            new_npc.mob = ORGE;
            new_npc.OOBB.Extents = { OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z };
            posX = float(rand() % 2000 + 250);
            posZ = float(rand() % 2000 + 250);
            new_npc.CurPos = { posX, mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 80, posZ };
            new_npc.InitPos = new_npc.CurPos;
            new_npc.speed = (float(rand() % 10) + 1.0f) / 10.0f;
            new_npc.hp = 100;
            npc.emplace_back(new_npc);
        }

        for (int i = 0; i < CHESTOBJECT_NUM; ++i) {
            NPC new_npc;
            new_npc.mob = CHEST;
            new_npc.OOBB.Extents = { OBB_SCALE_Chest_X, OBB_SCALE_Chest_Y, OBB_SCALE_Chest_Z };
            posX = float(rand() % 2000 + 250);
            posZ = float(rand() % 2000 + 250);
            new_npc.CurPos = { posX, mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 80, posZ };
            new_npc.InitPos = new_npc.CurPos;
            new_npc.speed = 0.0;
            new_npc.hp = 100;
            npc.emplace_back(new_npc);
        }

        for (int i = 0; i < MAX_OBJECT; ++i) {
            int rand = i % 4;
            switch (rand)
            {
            case 0: 
                npc[i].Lookvec = npc[i].CurPos;
                npc[i].Lookvec.x += 1;
                npc[i].Lookvec.normalized();
                  break;
            case 1: 
                npc[i].Lookvec = npc[i].CurPos;
                npc[i].Lookvec.x -= 1;
                npc[i].Lookvec.normalized();
                break;
            case 2: 
                npc[i].Lookvec = npc[i].CurPos;
                npc[i].Lookvec.z += 1;
                npc[i].Lookvec.normalized();
                break;
            case 3: 
                npc[i].Lookvec = npc[i].CurPos;
                npc[i].Lookvec.z -= 1;
                npc[i].Lookvec.normalized();
                break;
            default: 
                break;
            }
        }

        //// for test
        //for (int i = 0; i < MAX_OBJECT; ++i) {
        //    npc[i].hp = 100;
        //    npc[i].state = none;
        //}

        Vector3 mob_look;

        npc[0].CurPos.x = 553.f; npc[0].CurPos.z = 3511.f;
        npc[1].CurPos.x = 633.f; npc[1].CurPos.z = 3275.f;
        npc[2].CurPos.x = 700.f; npc[2].CurPos.z = 2850.f;
        
        mob_look = { 1545.f, 0.f, 3296.f };

        for (int i = 0; i < 3; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();

            npc[i].PrevPos = npc[i].CurPos;
        }
        //
        npc[3].CurPos.x = 1223.f; npc[3].CurPos.z = 1881.f;
        npc[4].CurPos.x = 1006.f; npc[4].CurPos.z = 1730.f;
        npc[5].CurPos.x = 747.f; npc[5].CurPos.z = 1724.f;
        npc[6].CurPos.x = 444.f; npc[6].CurPos.z = 1887.f;
        npc[7].CurPos.x = 203.f; npc[7].CurPos.z = 2100.f;
        npc[8].CurPos.x = 783.f; npc[8].CurPos.z = 2010.f;

        mob_look = { 800.f, 0.f, 2671.f };

        for (int i = 3; i < 8; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();

            npc[i].PrevPos = npc[i].CurPos;
        }

        //
        npc[9].CurPos.x = 764.f; npc[9].CurPos.z = 500.f;
        
        mob_look = { 1545.f, 0.f, 3296.f };

        npc[9].Lookvec = mob_look - npc[9].CurPos;
        npc[9].Lookvec.y = 0.f;
        npc[9].Lookvec = npc[9].Lookvec.normalized();

        npc[9].PrevPos = npc[9].CurPos;


        //
        npc[10].CurPos.x = 2356.f; npc[10].CurPos.z = 532.f;
        npc[11].CurPos.x = 2025.f; npc[11].CurPos.z = 514.f;
        npc[12].CurPos.x = 1786.f; npc[12].CurPos.z = 733.f;

        mob_look = { 2029.f, 0.f, 1500.f };

        for (int i = 10; i < 12; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();

            npc[i].PrevPos = npc[i].CurPos;
        }

        //
        npc[13].CurPos.x = 2866.f; npc[13].CurPos.z = 3181.f;
        npc[14].CurPos.x = 2945.f; npc[14].CurPos.z = 3424.f;
        npc[15].CurPos.x = 3141.f; npc[15].CurPos.z = 3499.f;
        npc[16].CurPos.x = 3415.f; npc[16].CurPos.z = 3402.f;
        npc[17].CurPos.x = 3597.f; npc[17].CurPos.z = 3273.f;

        mob_look = { 3256.f, 0.f, 2662.f };

        for (int i = 13; i < 17; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();

            npc[i].PrevPos = npc[i].CurPos;
        }

        //



    }

    // WALL
    {
        int xObjects = 16;
        int yObjects = 4;
        int zObjects = 6;
       /* int xObjects = 1;
        int yObjects = 1;
        int zObjects = 1;*/
        float wallSize = 250;
        float xPos, yPos, zPos;
        float height = 400;

        for (int i = 0, x = 0; x < xObjects; x++)
        {
            for (int z = 0; z < zObjects; z++)
            {
                for (int y = 0; y < yObjects; y++)
                {
                    Structure st;
                    if (z == 0 || z == zObjects - 1)
                    {
                        if (z != 0 && z != zObjects - 1 && x % 3 == 1 && y == 0) continue;
                        xPos = x * wallSize + 30;
                        zPos = z * wallSize * 3 + 30;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);

                        xPos = z * wallSize * 3 + 30;
                        zPos = x * wallSize + 30;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }


                    if (z == zObjects - 4)
                    {
                        if ((x == xObjects - 3 && y == 0))
                            continue;

                        xPos = z * wallSize * 3 + 7;
                        zPos = x * wallSize + 30;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);

                        if (x >= 11)
                            continue;
                        if ((x == 3 && y == 0))
                            continue;
                        if ((x == 8 && y == 2))
                            continue;

                        xPos = x * wallSize + 30;
                        zPos = z * wallSize * 3;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }


                    if (z == zObjects - 3)
                    {
                        if ((x == xObjects - 3 && y == 0))
                            continue;
                        if ((x == 3 && y == 0))
                            continue;

                        xPos = x * wallSize + 30;
                        zPos = z * wallSize * 3.5 + 50;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                        ++i;


                        if ((x == 0 && y == 2))
                            continue;
                        if ((x == 1 && y == 2))
                            continue;

                        xPos = z * wallSize * 3.5 + 40;
                        zPos = x * wallSize + 30;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                        ++i;

                    }
                }
            }
        }
    }

    BoundingOrientedBox LAVA;
    LAVA.Center = { 2070.f, 350.f, 2930.f };
    LAVA.Extents = { 250.f, 1.f, 250.f };
    map_lava.emplace_back(LAVA);

    LAVA.Center = { 2070.f, 350.f, 3580.f };
    LAVA.Extents = { 250.f, 1.f, 250.f };
    map_lava.emplace_back(LAVA);

    LAVA.Center = { 2100.f, 700.f, 2100.f };
    LAVA.Extents = { 600.f, 1.f, 600.f };
    map_lava.emplace_back(LAVA);


    memset(&CSpacket, 0, sizeof(CSpacket));



}

ServerMgr::~ServerMgr()
{
}

void ServerMgr::Initialize()
{
    // wsastartup()
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        err_quit("wsastartup()");

    // socket()
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");


    // bind()
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    printf("Initialized\n");
}

void ServerMgr::acceptClient()
{
    addrlen = sizeof(clientaddr);
    client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
    if (client_sock == INVALID_SOCKET) {
        err_quit("accept()");
    }

    //inet_ntop(AF_INET, &clientaddr.sin_addr, ip_addr, 40);

    printf("\n[TCP IP] NEW CLIENT: IP= %s, PORT= %d\n",
        ip_addr, ntohs(clientaddr.sin_port));

    sockQueue.emplace_back(client_sock);
}

void ServerMgr::RecvPacket(SOCKET s)
{
    retval = recvn(s, reinterpret_cast<char *>(&CSpacket), sizeof(CSpacket), 0);
    if (retval == SOCKET_ERROR)
    {
        //err_quit("recv()");
    }
}

void ServerMgr::SendPacket(SOCKET s)
{
    for (int i = 0; i < MAX_PLAYER; ++i)
    {
        SCpacket.player->id = i;
        SCpacket.player->pos = player[i].CurPos;
        SCpacket.player->look = player[i].look;

        retval = send(s, reinterpret_cast<char*>(&SCpacket), sizeof(SCpacket), 0);
        if (retval == WSAEWOULDBLOCK)
        {
            err_display("send WOULDBLOCK\n");
            //err_quit("send()");
        }
    }

}

void ServerMgr::Update()
{
    for (int n_id = 0; n_id < MAX_OBJECT; ++n_id) {
        switch (npc[n_id].state)
        {
        case none:
            //npc[n_id].CurPos.z = npc[n_id].CurPos.z + (npc[n_id].Lookvec.z * npc[n_id].speed);
            //npc[n_id].CurPos.x = npc[n_id].CurPos.x + (npc[n_id].Lookvec.x * npc[n_id].speed);
            break;

        case hit:
            TracePlayer(n_id);
            break;

        case attack:
            AttackPlayer(n_id);
            break;

        case dead:
            {
                if ( chrono::system_clock::now() - npc[n_id].timeDeath > chrono::milliseconds(2000))
                npc[n_id].CurPos.x = -1000.f;
                npc[n_id].CurPos.z = -1000.f;
                npc[n_id].PrevPos.x = -1000.f;
                npc[n_id].PrevPos.z = -1000.f;
            }
            break;
        }

        //gravity
        if (npc[n_id].CurPos.y > mapData->GetHeight(npc[n_id].CurPos.x, npc[n_id].CurPos.z) * mapData->GetScale().y + 80)
            npc[n_id].CurPos.y -= 5;
        else
            npc[n_id].CurPos.y = mapData->GetHeight(npc[n_id].CurPos.x, npc[n_id].CurPos.z) * mapData->GetScale().y + 80;


        if (npc[n_id].CurPos != npc[n_id].PrevPos) {
            Vector3 Lookvec = npc[n_id].CurPos - npc[n_id].PrevPos;
            Lookvec.y = 0.f;
            npc[n_id].Lookvec = Lookvec.normalized();
        }

        // modify using ._41(?)
        XMFLOAT4X4 danwi
        (
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            npc[n_id].CurPos.x, npc[n_id].CurPos.y, npc[n_id].CurPos.z, 1
        );


        npc[n_id].OOBB.Transform(npc[n_id].OOBB, DirectX::XMLoadFloat4x4(&danwi));

        switch (npc[n_id].mob) 
        {
        case MAGMA:
            npc[n_id].OOBB.Extents = { OBB_SCALE_Magmaa_X, OBB_SCALE_Magmaa_Y, OBB_SCALE_Magmaa_Z };
            break;
        case GOLEM:
            npc[n_id].OOBB.Extents = { OBB_SCALE_Golem_X, OBB_SCALE_Golem_Y, OBB_SCALE_Golem_Z };
            break;
        case ORGE:
            npc[n_id].OOBB.Extents = { OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z };
            break;
        case CHEST:
            npc[n_id].OOBB.Extents = { OBB_SCALE_Chest_X, OBB_SCALE_Chest_Y, OBB_SCALE_Chest_Z };
            break;
        }

        // init lookvec
        if (npc[n_id].Lookvec.x == 0.f && npc[n_id].Lookvec.y == 0.f && npc[n_id].Lookvec.z == 0.f)
            npc[n_id].Lookvec = { 1, 0, 0 };

        
        NPC_CollCheck(n_id);       

        npc[n_id].PrevPos = npc[n_id].CurPos;
    }

    
}

void ServerMgr::TracePlayer(int n_id)
{
    int p_id = npc[n_id].destPl;
    float dis = Vector3::Distance(npc[n_id].CurPos, player[p_id].CurPos);

    if (100.f < dis)
    {
        npc[n_id].CurPos = Vector3::MoveTowards(npc[n_id].CurPos, player[npc[n_id].destPl].CurPos, 10);
        npc[n_id].CurPos.y = mapData->GetHeight(npc[n_id].CurPos.x, npc[n_id].CurPos.z) * mapData->GetScale().y + 80;
    }
    else 
        npc[n_id].state = attack;


    if (player[p_id].hp == 0)
        npc[n_id].state = none;
}

void ServerMgr::AttackPlayer(int n_id)
{
    int p_id = npc[n_id].destPl;

    player[p_id].state = hit;
    npc[n_id].speed = 0;

    if (0 < player[p_id].hp && chrono::system_clock::now() - npc[n_id].timeLastAttack > chrono::milliseconds(1000)) {
        player[p_id].hp -= 10;
        npc[n_id].state = hit;
        npc[n_id].timeLastAttack = chrono::system_clock::now();
    }
}

void ServerMgr::CheckPlayerDead(int p_id)
{
    if (player[p_id].hp <= 0 && player[p_id].state != dead) {
        player[p_id].timeDead = chrono::system_clock::now();
        player[p_id].state = dead;
        player[p_id].CurPos = player[p_id].PrevPos;
    }


    if (player[p_id].state == dead && chrono::system_clock::now() - player[p_id].timeDead > chrono::milliseconds(3000)) {
        player[p_id].hp = 100;
        player[p_id].state = none;
        player[p_id].CurPos = player[p_id].InitPos;
    }
}


void ServerMgr::keyInput(CS_PACKET cspacket)
{
    player[cspacket.id].bullet.in_use = false;
    player[cspacket.id].bullet.type = type_none;

    if (cspacket.input.Key_W == true) {
        player[cspacket.id].state = ::move;
        player[cspacket.id].CurPos.z = player[cspacket.id].CurPos.z + cspacket.look.z * player[cspacket.id].speed;
        player[cspacket.id].CurPos.x = player[cspacket.id].CurPos.x + cspacket.look.x * player[cspacket.id].speed;
    }
    if (cspacket.input.Key_S == true) {
        player[cspacket.id].state = ::move;
        player[cspacket.id].CurPos.z = player[cspacket.id].CurPos.z - cspacket.look.z * player[cspacket.id].speed;
        player[cspacket.id].CurPos.x = player[cspacket.id].CurPos.x - cspacket.look.x * player[cspacket.id].speed;
    }
    if (cspacket.input.Key_A == true) {
        player[cspacket.id].state = ::move;
        player[cspacket.id].CurPos.z = player[cspacket.id].CurPos.z + cspacket.look.x * player[cspacket.id].speed;
        player[cspacket.id].CurPos.x = player[cspacket.id].CurPos.x - cspacket.look.z * player[cspacket.id].speed;
    }
    if (cspacket.input.Key_D == true) {
        player[cspacket.id].state = ::move;
        player[cspacket.id].CurPos.z = player[cspacket.id].CurPos.z - cspacket.look.x * player[cspacket.id].speed;
        player[cspacket.id].CurPos.x = player[cspacket.id].CurPos.x + cspacket.look.z * player[cspacket.id].speed;
    }

    if (cspacket.type == 1 && player[cspacket.id].state != dead) {
        if (player[cspacket.id].bullet.ammo > 0) {
            player[cspacket.id].state = attack;
            player[cspacket.id].bullet.in_use = true;
            player[cspacket.id].bullet.ammo--;
            PickObjectByCameraLookVector(cspacket.id);
        }
        
        if (player[cspacket.id].bullet.ammo == 0)
        {
            if (player[cspacket.id].state == none) {
                player[cspacket.id].state = reload;
                player[cspacket.id].timeReload = chrono::system_clock::now();
            }

            if (player[cspacket.id].state == reload && chrono::system_clock::now() - player[cspacket.id].timeReload > chrono::milliseconds(2200));
            player[cspacket.id].bullet.ammo = 30;
        }

        cout << "x: " << player[cspacket.id].CurPos.x << " y: " << player[cspacket.id].CurPos.y << " z: " << player[cspacket.id].CurPos.z << endl;
        cout << "look.x: " << player[cspacket.id].look.x << " look.y: " << player[cspacket.id].look.y << " look.z: " << player[cspacket.id].look.z << endl << endl;
    }

    // gravity
    if (player[cspacket.id].CurPos.y > mapData->GetHeight(player[cspacket.id].CurPos.x, player[cspacket.id].CurPos.z) * mapData->GetScale().y + 90
        && player[cspacket.id].hp >= 0)
        player[cspacket.id].CurPos.y -= 10;
    else    
        player[cspacket.id].CurPos.y = mapData->GetHeight(player[cspacket.id].CurPos.x, player[cspacket.id].CurPos.z) * mapData->GetScale().y + 90;

    player[cspacket.id].look = cspacket.look;
    player[cspacket.id].camera_look = cspacket.cameraLook;
        
    XMFLOAT4X4 danwi
    (
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        player[cspacket.id].CurPos.x, player[cspacket.id].CurPos.y, player[cspacket.id].CurPos.z, 1
    );  

    player[cspacket.id].OOBB.Transform(player[cspacket.id].OOBB, DirectX::XMLoadFloat4x4(&danwi));
    player[cspacket.id].OOBB.Extents.x = OBB_SCALE_PLAYER_X;
    player[cspacket.id].OOBB.Extents.y = OBB_SCALE_PLAYER_Y;
    player[cspacket.id].OOBB.Extents.z = OBB_SCALE_PLAYER_Z;

    Player_CollCheck(cspacket.id);
    CheckPlayerDead(cspacket.id);

    player[cspacket.id].PrevPos = player[cspacket.id].CurPos;
}

void ServerMgr::ProcessPacket()
{
}

void ServerMgr::Player_CollCheck(int id)
{
    //for (int i = 0; i < MAX_OBJECT; ++i) {
    //    ContainmentType containType = player[id].OOBB.Contains(npc[i].OOBB);
    //
    //    switch (containType)
    //    {
    //    case DISJOINT:
    //        break;
    //    case INTERSECTS:
    //        player[id].CurPos = player[id].PrevPos;
    //        break;
    //
    //    case CONTAINS:
    //        player[id].CurPos = player[id].PrevPos;
    //        break;
    //
    //    default:
    //        break;
    //    }
    //}

    for (int i = 0; i < structure.size(); ++i) {
        ContainmentType containType = player[id].OOBB.Contains(structure[i].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            player[id].CurPos = player[id].PrevPos;
            break;
        case CONTAINS:
            player[id].CurPos = player[id].PrevPos;
            break;

        default:
            break;
        }
    }

    for (int i = 0; i < map_lava.size(); ++i)
    {
        ContainmentType containType = map_lava[i].Contains(player[id].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            player[id].CurPos = player[id].PrevPos;
            if (player[id].hp > 0)
                player[id].hp -= 1;
            break;
        case CONTAINS:
            player[id].CurPos = player[id].PrevPos;
            if (player[id].hp > 0)
                player[id].hp -= 1;
            break;

        default:
            break;
        }
    }
}

void ServerMgr::NPC_CollCheck(int id)
{
    //for (int i = 0; i < MAX_PLAYER; ++i) {
    //    //if (sqrt((npc[id].CurPos.x - player[i].CurPos.x) * (npc[id].CurPos.x - player[i].CurPos.x) 
    //    //    + (npc[id].CurPos.y - player[i].CurPos.y) * (npc[id].CurPos.y - player[i].CurPos.y)
    //    //    + (npc[id].CurPos.z - player[i].CurPos.z) * (npc[id].CurPos.z - player[i].CurPos.z)) < 200.f)
    //    {
    //        ContainmentType containType = npc[id].OOBB.Contains(player[i].OOBB);
    //        switch (containType)
    //        {
    //        case DISJOINT:
    //            break;
    //        case INTERSECTS:
    //            npc[id].state = none;
    //            npc[id].CurPos = npc[id].PrevPos;
    //            npc[id].speed *= -1;
    //            break;
    //        case CONTAINS:
    //            npc[id].state = none;
    //            npc[id].CurPos = npc[id].PrevPos;
    //            npc[id].speed *= -1;
    //            break;
    //        default:
    //            break;
    //        }
    //    }
    //}

    for (int i = 0; i < structure.size(); ++i) {
        ContainmentType containType = npc[id].OOBB.Contains(structure[i].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
        {
            npc[id].state = none;
            npc[id].CurPos = npc[id].PrevPos;
            Vector3 lookvec = npc[id].InitPos - npc[id].CurPos;
            npc[id].Lookvec = lookvec.normalized();
            break;
        }
        case CONTAINS:
        {
            npc[id].state = none;
            npc[id].CurPos = npc[id].PrevPos;
            Vector3 lookvec = npc[id].InitPos - npc[id].CurPos;
            npc[id].Lookvec = lookvec.normalized();
            break;
        }
        default:
            break;
        }
    }
}

bool ServerMgr::CollideObjectByRayIntersection(BoundingOrientedBox objectBoundingBox, Vector3& position, Vector3& direction, float* distance)
{
    XMVECTOR xmRayOrigin = XMLoadFloat3(&position);
    XMVECTOR xmRayDirection = XMLoadFloat3(&direction);
    return objectBoundingBox.Intersects(xmRayOrigin, xmRayDirection, *distance);
} // error (call stack)

void ServerMgr::PickObjectByCameraLookVector(int p_id)
{
    bool			isIntersected = false;
    float			fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
    obj_type        object_type;
    object_type =   type_none;
    int             object_id;
    Vector3         camera_pos = player[p_id].CurPos;

    camera_pos.y += 20;

    Matrix4x4		matColidePosition = Matrix4x4::identity; // identity: 4x4 danwi
    for (int i = 0; i < MAX_OBJECT; ++i) // monster check
    {
        isIntersected = CollideObjectByRayIntersection(npc[i].OOBB, camera_pos, player[p_id].camera_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_npc;
            object_id = i;
        }
    }

    for (int i = 0; i < structure.size(); ++i)
    {
        isIntersected = CollideObjectByRayIntersection(structure[i].OOBB, camera_pos, player[p_id].camera_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_wall;
            object_id = i;
        }
    }

    ////////////////////////////////////////
    if (type_none != object_type)
    {
        float fDistance = static_cast<float>(pow(fNearestHitDistance, 2));
        float fSumLookPos = static_cast<float>(pow(player[p_id].camera_look.x, 2)) + static_cast<float>(pow(player[p_id].camera_look.y, 2)) + static_cast<float>(pow(player[p_id].camera_look.z, 2));
        float fFinal = fDistance / fSumLookPos;
        // return collision CurPos (burn effects)
        matColidePosition._41 = camera_pos.x + player[p_id].camera_look.x * (sqrt(fFinal) - 2);
        matColidePosition._42 = camera_pos.y + player[p_id].camera_look.y * (sqrt(fFinal) - 2);
        matColidePosition._43 = camera_pos.z + player[p_id].camera_look.z * (sqrt(fFinal) - 2);

        player[p_id].bullet.pos.x = matColidePosition._41;
        player[p_id].bullet.pos.y = matColidePosition._42;
        player[p_id].bullet.pos.z = matColidePosition._43;
    }

    switch (object_type)
    {
    case type_none:
        player[p_id].bullet.in_use = 0;
        break;
    case type_npc:
        if (npc[object_id].mob == MAGMA || npc[object_id].mob == GOLEM || npc[object_id].mob == ORGE)
        {
            if (npc[object_id].state != dead)
            {
                player[p_id].bullet.type = type_npc;
                npc[object_id].destPl = p_id;
                npc[object_id].state = hit;

                if (0 < npc[object_id].hp)
                    npc[object_id].hp -= 10;
                else
                {
                    npc[object_id].state = dead;
                    npc[object_id].timeDeath = chrono::system_clock::now();
                }
            }
        }

        if (npc[object_id].mob == CHEST)
            player[p_id].bullet.type = type_wall;
            break;
    case type_wall:
        player[p_id].bullet.type = type_wall;
        //
        break;
    case type_player:
        //
        break;
    }
}

void ServerMgr::SetHP(int id, int hp)
{
    player[id].hp = hp;
}

void ServerMgr::SetPos(int id, XMFLOAT3 pos)
{
    player[id].CurPos = pos;
}



SC_PACKET ServerMgr::GetPacket(SC_PACKET scpacket)
{

    for (int i = 0; i < MAX_PLAYER; ++i)
    {
        scpacket.player[i].id = i;
        scpacket.player[i].pos = player[i].CurPos;
        scpacket.player[i].look = player[i].look;
        scpacket.player[i].cameraLook = player[i].camera_look;
        scpacket.player[i].state = player[i].state;
        scpacket.player[i].hp = player[i].hp;
        scpacket.player[i].bullet.in_use = player[i].bullet.in_use;
        scpacket.player[i].bullet.type = player[i].bullet.type;
        scpacket.player[i].bullet.ammo = player[i].bullet.ammo;
        scpacket.player[i].bullet.pos = player[i].bullet.pos;
        scpacket.player[i].bullet.type = player[i].bullet.type;
    }

    for (int i = 0; i < MAX_OBJECT; ++i)
    {
        scpacket.npc[i].pos = npc[i].CurPos;
        scpacket.npc[i].look = npc[i].Lookvec;
        scpacket.npc[i].hp = npc[i].hp;
        scpacket.npc[i].state = npc[i].state;
    }

    return scpacket;
}





void ServerMgr::err_quit(char* msg)
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
