#include "GameMgr.h"
#include <random>

GameMgr::GameMgr()
{
    mapData = new CHeightMapImage( _T("mapImage/map1.raw"), 513, 513, Vector3(10.0f, 25.0f, 10.0f));
}

GameMgr::~GameMgr()
{
}

void GameMgr::InitGame(int id[4])
{
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    gameZones.reserve(5);
    zoneLevel = 0;
    stoneTime = chrono::system_clock::now();
    r_id = arr_player[id[0]].r_id;

    //PLAYER
    {
        for (int i = 0; i < MAX_PLAYER; ++i) {
            pl_list[i] = id[i];
        }

        for (int i = 0; i != MAX_PLAYER; ++i) {
            int p_id = pl_list[i];

            arr_player[p_id].id = p_id;


            posX = float(500 + (p_id * ((2300 - 1700) / MAX_PLAYER)));
            posZ = 4680.0f;

            //posX = float(557.519 + (p_id * ((2300 - 1700) / MAX_PLAYER)));
            //posZ = 1202.554;

            posY = mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 100.f;

            Vector3 pos{ posX, posY,  posZ };
            arr_player[i].InitPos = pos;
            arr_player[i].CurPos = pos;
            arr_player[i].PrevPos = pos;
            arr_player[i].ps.hp = arr_player[i].ps.maxhp = 100;
            //라이플
            switch (arr_player[i].wp_type)
            {
            case WEAPON_RIFLE:
                arr_player[i].ps.attackSpeed = 0.15f;
                arr_player[i].ps.maxAmmo = 30;
                arr_player[i].ps.ammo = 30;
                arr_player[i].ps.attackDamage = 7.f;
                break;
            case WEAPON_SHOTGUN:
                arr_player[i].ps.attackSpeed = 0.5f;
                arr_player[i].ps.maxAmmo = 7;
                arr_player[i].ps.ammo = 7;
                arr_player[i].ps.attackDamage = 5.f;

                break;
            case WEAPON_SNIPER:
                arr_player[i].ps.attackSpeed = 1.0f;
                arr_player[i].ps.maxAmmo = 5;
                arr_player[i].ps.ammo = 5;
                arr_player[i].ps.attackDamage = 40.f;

                break;
            default:
                cout << "type error" << endl;
                break;
            }
            arr_player[i].state = none;

            Vector3 lookvec = { posX - 2050.f, 0, posZ - 1500.f };
            lookvec = lookvec.normalized();
            arr_player[i].pl_look = lookvec;
            arr_player[i].OOBB = BoundingOrientedBox(arr_player[i].CurPos, XMFLOAT3(OBB_SCALE_PLAYER_X, OBB_SCALE_PLAYER_Y, OBB_SCALE_PLAYER_Z), XMFLOAT4(0, 0, 0, 1));
            arr_player[i].cam_look = { 1,0,0 };


            for (int j = 0; j < 9; ++j)
            {
                ITEM_TYPE item = static_cast<ITEM_TYPE>(j);
                arr_player[i].pl_items.insert({ item, 0 });
            }
        }
    }

    //NPC
    {
        npc.reserve(MAX_OBJECT);

        NPC new_npc;
        posX = float(rand() % 2000 + 250);
        posZ = float(rand() % 2000 + 250);
        new_npc.CurPos = { posX, mapData->GetHeight(posX, posZ) * mapData->GetScale().y + 80, posZ };
        new_npc.InitPos = new_npc.CurPos;
        new_npc.speed = 8;
        new_npc.state = none;


        for (int i = 0; i < MAGMAMONSTER_NUM; ++i) {
            new_npc.mob = MAGMA;
            new_npc.OOBB.Extents = { OBB_SCALE_Magmaa_X, OBB_SCALE_Magmaa_Y, OBB_SCALE_Magmaa_Z };
            new_npc.hp = 200;
            new_npc.attackRange = 1300;
            new_npc.sight = 1500;
            new_npc.coolTime = 1000;
            npc.emplace_back(new_npc);
        }
        for (int i = 0; i < GOLEMMONSTER_NUM; ++i) {
            new_npc.mob = GOLEM;
            new_npc.OOBB.Extents = { OBB_SCALE_Golem_X, OBB_SCALE_Golem_Y, OBB_SCALE_Golem_Z };
            new_npc.hp = 3500;
            new_npc.attackRange = 300;
            new_npc.sight = 350;
            new_npc.speed = 10;
            new_npc.coolTime = 1800;
            npc.emplace_back(new_npc);
        }
        for (int i = 0; i < ORGEMONSTER_NUM; ++i) {
            new_npc.mob = OGRE;
            new_npc.OOBB.Extents = { OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z };
            new_npc.hp = 250;
            new_npc.attackRange = 150;
            new_npc.speed = 8;
            new_npc.sight = 500;
            new_npc.coolTime = 1000;
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

        Vector3 mob_look;

        //중앙 방 2층 2마리
        npc[0].CurPos.x = 1706.f; npc[0].CurPos.z = 2045.f; npc[0].zoneNum = 1;
        npc[1].CurPos.x = 1705.f; npc[1].CurPos.z = 3695.f; npc[1].zoneNum = 1;

        //중앙방 출구후 방 1마리
        npc[2].CurPos.x = 4790.f; npc[2].CurPos.z = 1205.f; npc[2].zoneNum = 2;
        mob_look = { 1545.f, 0.f, 3296.f };

        for (int i = 0; i < 3; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();
        }
        //시작방 앞 2마리
        npc[3].CurPos.x = 4675.15f; npc[3].CurPos.z = 4515.6f; npc[3].zoneNum = 0;
        npc[4].CurPos.x = 4724.52f; npc[4].CurPos.z = 4877.16f; npc[4].zoneNum = 0;
        //중앙방 3마리
        npc[5].CurPos.x = 4662.5f;  npc[5].CurPos.z = 2706.39f; npc[5].zoneNum = 1;
        npc[6].CurPos.x = 2006.f;  npc[6].CurPos.z = 2045.f; npc[6].zoneNum = 1;
        npc[7].CurPos.x = 1742.81; npc[7].CurPos.z = 2972.67f; npc[7].zoneNum = 1;     //중앙 2층 한가운데

        //보스전방
        npc[8].CurPos.x = 384;  npc[8].CurPos.z = 1209; npc[8].zoneNum = 3;
        mob_look = { 800.f, 0.f, 2671.f };

        for (int i = 3; i < 9; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();
        }

        //보스
        npc[9].CurPos.x = 4760.f; npc[9].CurPos.z = 575.f; npc[9].zoneNum = 4;


        mob_look = { 1545.f, 0.f, 3296.f };

        npc[9].Lookvec = mob_look - npc[9].CurPos;
        npc[9].Lookvec.y = 0.f;
        npc[9].Lookvec = npc[9].Lookvec.normalized();

        npc[9].PrevPos = npc[9].CurPos;

        //시작방 앞 2마리
        npc[10].CurPos.x = 3698.f; npc[10].CurPos.z = 4907.26f;   npc[10].zoneNum = 0;
        npc[11].CurPos.x = 3725.89f; npc[11].CurPos.z = 4545.97f;   npc[11].zoneNum = 0;

        //중앙 방배치몹
        npc[12].CurPos.x = 3298.f; npc[12].CurPos.z = 2752.f;   npc[12].zoneNum = 1;    //중앙방 맵 한가운데 왼쪽몹

        mob_look = { 2029.f, 0.f, 1500.f };

        for (int i = 9; i < 13; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();
        }


        npc[13].CurPos.x = 3255.0f;  npc[13].CurPos.z = 3703.09f; npc[13].zoneNum = 1;       //중앙방 한가운데 오른쪽몹
        npc[14].CurPos.x = 4610.76f; npc[14].CurPos.z = 2142.85f; npc[14].zoneNum = 1;      //중앙방 작은계단위

        //중앙방 출구 후 2마리
        npc[15].CurPos.x = 4668.f; npc[15].CurPos.z = 1640.f; npc[15].zoneNum = 2;          //출구 바라보고 왼쪽
        npc[16].CurPos.x = 4240.f; npc[16].CurPos.z = 1139.f; npc[16].zoneNum = 2;        //출구 바라보고 오른쪽

        //이상한놈들
        npc[17].CurPos.x = 10036.f; npc[17].CurPos.z = 3532.05f; npc[17].zoneNum = 4;

        mob_look = { 3256.f, 0.f, 2662.f };

        for (int i = 13; i < 18; ++i)
        {
            npc[i].Lookvec = mob_look - npc[i].CurPos;
            npc[i].Lookvec.y = 0.f;
            npc[i].Lookvec = npc[i].Lookvec.normalized();
        }
        for (auto& p : npc)
        {
            p.PrevPos = p.CurPos = GetPositionToHeightMap(p.CurPos.x, p.CurPos.z, 80);
            Matrix4x4 temp = Matrix4x4::identity;
            temp = temp.Translate(p.CurPos);
            p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&temp));
        }
        //

    }

    ///static object
    {
        interaction.reserve(MAX_INTRACTION);

        std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<int> dis(0, 8);

        for (int i = 0; i < CHESTOBJECT_NUM; ++i) {
            INTERACTION new_object;
            new_object.objectName = CHEST;
            new_object.OOBB.Extents = { OBB_SCALE_Chest_X, OBB_SCALE_Chest_Y, OBB_SCALE_Chest_Z };
            item_status item;
            item.getEnable = false;
            item.item = static_cast<ITEM_TYPE>(dis(gen));
            new_object.item.push_back(item);
            item.item = static_cast<ITEM_TYPE>(dis(gen));
            new_object.item.push_back(item);
            item.item = static_cast<ITEM_TYPE>(dis(gen));
            new_object.item.push_back(item);
            item.item = static_cast<ITEM_TYPE>(dis(gen));
            new_object.item.push_back(item);
            new_object.zoneNum = 99;
            interaction.emplace_back(new_object);
        }
        for (int i = 0; i < DOOROBJECT_NUM; ++i) {
            INTERACTION new_object;
            new_object.objectName = DOOR;
            new_object.OOBB.Extents = { OBB_SCALE_Door_X, OBB_SCALE_Door_Y, OBB_SCALE_Door_Z };
            new_object.zoneNum = 99;
            interaction.emplace_back(new_object);
        }
        for (int i = 0; i < LEVEROBJECT_NUM; ++i) {
            INTERACTION new_object;
            new_object.objectName = LEVER;
            new_object.OOBB.Extents = { OBB_SCALE_Lever_X, OBB_SCALE_Lever_Y, OBB_SCALE_Lever_Z };
            new_object.zoneNum = 99;
            interaction.emplace_back(new_object);
        }

        INTERACTION new_object;
        new_object.objectName = MUD;
        new_object.OOBB.Extents = { 0, 0, 0 };
        interaction.emplace_back(new_object);

        /*new_object;
        new_object.objectName = MUD;
        new_object.OOBB.Extents = { 0, 0, 0 };
        interaction.emplace_back(new_object);*/
        /*
            INTERACTION new_object;
            new_object.objectName = ROCKS;
            new_object.OOBB.Extents = { 0, 0, 0 };
            interaction.emplace_back(new_object);

            new_object.objectName = ROCKS;
            new_object.OOBB.Extents = { 0, 0, 0 };
            interaction.emplace_back(new_object);

            INTERACTION new_object;
            new_object.objectName = MUD;
            new_object.OOBB.Extents = { 0, 0, 0 };
            interaction.emplace_back(new_object);

        for (int i = 0; i < 3; ++i) {
             INTERACTION new_object;
             new_object.objectName = STONE;
             new_object.OOBB.Extents = { 0, 0, 0 };
             interaction.emplace_back(new_object);
            }*/

            //4699.98 4273.8
            //3933.29 1773.41
            //  708.688 1029.33

            //상자
        interaction[0].Pos = GetPositionToHeightMap(4915.81, 4840, 40);      //시작방
        interaction[1].Pos = GetPositionToHeightMap(660.232, 4105.35, 40);      //마그마 미로방
        interaction[2].Pos = GetPositionToHeightMap(860, 275, 40);              //보스방앞
        interaction[3].Pos = GetPositionToHeightMap(3555, 2000, 40);            //중앙 방 내부 상자
        interaction[4].Pos = GetPositionToHeightMap(1350, 1208, 40);            //중앙 방 출구후 뒤 상자
        interaction[5].Pos = GetPositionToHeightMap(1350, 4120, 40);
        //문                 
        interaction[6].Pos = GetPositionToHeightMap(1165, 4700, 75);        //시작방
        interaction[7].Pos = GetPositionToHeightMap(1170, 1450, 75);       //마그마방 직전
        interaction[7].zoneNum = 2;
        interaction[8].Pos = GetPositionToHeightMap(1165, 450, 75);      //보스방 입구
        interaction[9].Pos = GetPositionToHeightMap(3935, 1815, 90);           //중앙방 출구
        interaction[9].zoneNum = 1;
        interaction[10].Pos = GetPositionToHeightMap(710, 1060, 70);                   //보스방 전전방
        interaction[10].zoneNum = 3;
        interaction[11].Pos = GetPositionToHeightMap(4700, 4242, 60);                   //중앙방 입구
        interaction[11].zoneNum = 0;
        //레버
        //interaction[11].Pos = Vector3(1630, 540, 2680);

        interaction[12].Pos = Vector3(360, 1475, 4250);

        Matrix4x4 world;
        Quaternion qua = Quaternion::AngleAxis(90, Vector3(0, 1, 0));
        world = Matrix4x4::Rotate(qua);
        interaction[4].OOBB.Transform(interaction[4].OOBB, XMLoadFloat4x4(&world));
        interaction[6].OOBB.Transform(interaction[6].OOBB, XMLoadFloat4x4(&world));
        interaction[7].OOBB.Transform(interaction[7].OOBB, XMLoadFloat4x4(&world));
        interaction[8].OOBB.Transform(interaction[8].OOBB, XMLoadFloat4x4(&world));
        for (auto& p : interaction)
        {
            world = Matrix4x4::identity;
            world._41 = p.Pos.x, world._42 = p.Pos.y, world._43 = p.Pos.z;
            p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
            p.Lookvec = Vector3(-1, 0, 0);
            p.state = none;
            p.interactEnable = false;
        }
        interaction[4].Lookvec = Vector3(0, 0, 1);
    }

    rangeAttack.reserve(20);

    for (int i = 0; i < 20; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);

        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(20, 20, 20);
        ra.activeEnable = false;

        rangeAttack.push_back(ra);
    }

    stoneAttack.reserve(10);

    for (int i = 0; i < 5; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(50, 37, 75);
        ra.activeEnable = false;
        stoneAttack.push_back(ra);
    }
    for (int i = 0; i < 5; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(30, 22.5, 37);
        ra.activeEnable = false;
        stoneAttack.push_back(ra);
    }
    // WALL
    {
        int xObjects = 20;
        int yObjects = 3;
        int zObjects = 7;

        float wallSize = 250;
        float xPos, yPos, zPos;
        float height = 1000;

        for (int i = 0, x = 0; x < xObjects; x++)
        {
            for (int z = 0; z < zObjects; z++)
            {
                for (int y = 0; y < yObjects; y++)
                {
                    Structure st;
                    st.type = wall;

                    if (z == 0)
                    {
                        xPos = x * wallSize + 200;
                        zPos = z * wallSize * 3 + 100;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);

                        xPos = z * wallSize * 3 + 100;
                        zPos = x * wallSize + 200;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }

                    if (z == zObjects - 1)
                    {
                        xPos = x * wallSize + 200;
                        zPos = 5045.f;;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);


                        xPos = 5045.f;
                        zPos = x * wallSize + 200;
                        yPos = height + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }

                    if ((x != 1 && x != 5 && x != xObjects - 2) || y != 0)
                    {
                        xPos = 1170;
                        zPos = x * wallSize + 200;
                        yPos = height + 55 + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 20, wallSize / 2, wallSize / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }

                    if ((x != 15 || y != 0))
                    {
                        if (x != 1 && x != 2 || y == 2)
                        {
                            xPos = x * wallSize + 200;
                            zPos = 1815;
                            yPos = height - 50 + wallSize * (0.5f + y);
                            st.center = (XMFLOAT3(xPos, yPos, zPos));
                            st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                            st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                            structure.push_back(st);
                        }
                    }

                    if ((x != xObjects - 2 || y == 2))
                    {
                        xPos = x * wallSize + 200;
                        zPos = 4245.f;
                        yPos = height + 40 + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }
                    if (x != 2 || y != 0)
                    {
                        xPos = x * wallSize + 200;
                        zPos = 1060;
                        yPos = height + 50 + wallSize * (0.5f + y);
                        st.center = (XMFLOAT3(xPos, yPos, zPos));
                        st.extend = (XMFLOAT3(wallSize / 2, wallSize / 2, wallSize / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure.push_back(st);
                    }
                }
            }
        }
    }

    //중앙방 큰 2층 난간
    {
        //2층위 난간
        float xPos, yPos, zPos;
        float height = 1000;
        Structure st;

        st.type = fence;
        xPos = 2140;
        yPos = 1050;
        zPos = 3255;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(80, 400, 990));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);

        //계단 난간

        xPos = 2410;
        yPos = 1050;
        zPos = 2300;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(270, 300, 40));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);

    }

    //중앙방 작은 층 난간
    {
        float xPos, yPos, zPos;
        float height = 1000;
        Structure st;

        st.type = fence;

        xPos = 4255.58;
        yPos = 1050;
        zPos = 2805;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(40, 110, 1020));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);
    }

    //시작방 난간
    {
        //입구에서 출구바라보고 계단왼쪽난간
        float xPos, yPos, zPos;
        float height = 1000;
        Structure st;

        st.type = fence;
        xPos = 4276.01;
        yPos = 1050;
        zPos = 4900.77;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(40, 100, 120));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);

        //입구에서 출구바라보고 계단오른쪽난간
        xPos = 4276.01;
        yPos = 1050;
        zPos = 4365;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(40, 100, 120));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);
    }

    //중앙방 출구 후 방 계단난간
    {
        Structure st;

        st.type = fence;

        st.center = (XMFLOAT3(3550, 1050, 1564));
        st.extend = (XMFLOAT3(40, 100, 200));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure.push_back(st);
    }

    BoundingOrientedBox LAVA;
    LAVA.Center = { 665.265, 1260, 3328.36 };
    LAVA.Extents = { 800, 1.f, 800 };
    map_lava.emplace_back(LAVA);

    ZONE zone;
    zone.isClear = false;
    //시작방
    zone.monsterID = { 3,4,10,11 };
    gameZones.push_back(zone);
    //중앙방
    zone.monsterID = { 0,1,5,6,7,12,13,14 };
    gameZones.push_back(zone);
    zone.monsterID = { 2, 15, 16 };
    gameZones.push_back(zone);
    //안개방?
    zone.monsterID = { 8 };
    gameZones.push_back(zone);
    //보스방
    zone.monsterID = { 9 };
    gameZones.push_back(zone);

    isRunning = true;
}



void GameMgr::Update()
{
    for (int n_id = 0; n_id < MAX_OBJECT; ++n_id) {
        switch (npc[n_id].state)
        {
        case none:
            for (auto& p : arr_player)
            {
                if (find(gameZones.at(zoneLevel).monsterID.begin(), gameZones.at(zoneLevel).monsterID.end(), n_id) == gameZones.at(zoneLevel).monsterID.end()) continue;
                if (p.ps.hp <= 0) continue;
                if (Vector3::Distance(npc[n_id].CurPos, p.CurPos) < npc[n_id].sight)
                {
                    npc[n_id].destPl = p.id;
                    npc[n_id].state = hit;
                }
            }
            break;
        case hit:
            TracePlayer(n_id);
            break;

        case attack:
            AttackPlayer(n_id);
            break;

        case dead:
        {
            if (chrono::system_clock::now() - npc[n_id].timeDeath > chrono::milliseconds(2000)) {
                npc[n_id].CurPos.x = -1000.f;
            npc[n_id].CurPos.z = -1000.f;
            npc[n_id].PrevPos.x = -1000.f;
            npc[n_id].PrevPos.z = -1000.f;
            }
            break;
        }
        default:
            break;
        }

        //gravity
        if (npc[n_id].CurPos.y > mapData->GetHeight(npc[n_id].CurPos.x, npc[n_id].CurPos.z)* mapData->GetScale().y + 80)
            npc[n_id].CurPos.y -= 5;
        else
            npc[n_id].CurPos.y = mapData->GetHeight(npc[n_id].CurPos.x, npc[n_id].CurPos.z) * mapData->GetScale().y + 80;


        if (npc[n_id].CurPos != npc[n_id].PrevPos) {
            Vector3 Lookvec = npc[n_id].CurPos - npc[n_id].PrevPos;
            Lookvec.y = 0.f;
            npc[n_id].Lookvec = Lookvec.normalized();
        }

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
        case OGRE:
            npc[n_id].OOBB.Extents = { OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z };
            break;
        }

        // init lookvec
        if (npc[n_id].Lookvec.x == 0.f && npc[n_id].Lookvec.y == 0.f && npc[n_id].Lookvec.z == 0.f)
            npc[n_id].Lookvec = { 1, 0, 0 };


        NPC_CollCheck(n_id);

        npc[n_id].PrevPos = npc[n_id].CurPos;
    }

    for (auto& ra : rangeAttack)
    {
        if (!ra.activeEnable) continue;
        ra.pos = ra.pos + ra.look * ra.speed;
        ra.OOBB.Center = ra.pos;
        if (chrono::system_clock::now() - ra.liveTime > chrono::seconds(3))
        {
            ra.activeEnable = false;
            continue;
        }
        for (auto& p : arr_player)
        {
            if (ra.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].ps.block < Mathf::RandF(0, 100))
                    arr_player[p.id].ps.hp = Mathf::Max(arr_player[p.id].ps.hp - 10, 0);
                ra.activeEnable = false;

                cout << "range Attack" << endl;
                break;
            }
        }
        for (auto& p : structure)
        {
            if (ra.OOBB.Intersects(p.OOBB))
            {
                if (p.type == wall)
                {
                    ra.activeEnable = false;
                    break;
                }
            }
        }
           
    }
    if (zoneLevel == 4 && chrono::system_clock::now() - stoneTime > chrono::milliseconds(500))
    {
        for (auto& st : stoneAttack)
        {
            if (!st.activeEnable)
            {
                st.pos = Vector3(Mathf::RandF(1250.f, 4800.f), 1800, Mathf::RandF(200.f, 900.f));
                st.OOBB.Center = st.pos;
                st.activeEnable = true;
                st.liveTime = chrono::system_clock::now();
                break;
            }
        }
        stoneTime = chrono::system_clock::now();
    }

    for (auto& st : stoneAttack)
    {
        if (!st.activeEnable) continue;
        st.pos.y -= st.speed;
        st.OOBB.Center = st.pos;
        if (chrono::system_clock::now() - st.liveTime > chrono::seconds(4))
        {
            st.activeEnable = false;
            continue;
        }
        for (auto& p : arr_player)
        {
            if (st.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].ps.block < Mathf::RandF(0, 100))
                    arr_player[p.id].ps.hp = Mathf::Max(arr_player[p.id].ps.hp - 20, 0);
                st.activeEnable = false;
                break;
            }
        }
    }

    if (isSlow && chrono::system_clock::now() - slowTime > chrono::seconds(20))
        isSlow = false;

    if (npc[9].state == dead && chrono::system_clock::now() - npc[9].timeDeath > chrono::seconds(4))
    {
        if (isEnding == false) {
            isEnding = true;
            e_time = chrono::system_clock::now();
        }
    }

    cur_update_time = chrono::system_clock::now();
}


void GameMgr::TracePlayer(int n_id)
{
    int p_id = npc[n_id].destPl;
    if (arr_player[p_id].ps.hp <= 0)
    {
        npc[n_id].state = none;
        return;
    }
    Vector3 player_pos = arr_player[p_id].CurPos;
    player_pos.y -= 20;
    float dis = Vector3::Distance(npc[n_id].CurPos, player_pos);
    float speed = npc[n_id].speed;
    if (isSlow) speed *= 0.5;

    if (npc[n_id].attackRange < dis)    
        npc[n_id].CurPos = Vector3::MoveTowards(npc[n_id].CurPos, player_pos, speed);
    else
    {
        npc[n_id].state = attack;
        npc[n_id].isAttack = true;
        npc[n_id].timeLastAttack = chrono::system_clock::now();
    }
}


void GameMgr::AttackPlayer(int n_id)
{
    int p_id = npc[n_id].destPl;

    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc[n_id].timeLastAttack > chrono::milliseconds(npc[n_id].coolTime) && npc[n_id].isAttack)
    {
        if (npc[n_id].mob == OGRE)
        {
            if (arr_player[p_id].ps.block < Mathf::RandF(0, 100))
            {
                arr_player[p_id].m_slock.lock();
                arr_player[p_id].ps.hp = Mathf::Max(arr_player[p_id].ps.hp - 10, 0);
                arr_player[p_id].m_slock.unlock(); 
            }
        }
        else if(npc[n_id].mob == MAGMA)
        {
            for (auto& p : rangeAttack)
            {
                Matrix4x4 world = Matrix4x4::identity;
                if (!p.activeEnable)
                {
                    p.pos = npc[n_id].CurPos + npc[n_id].Lookvec * 30;
                    world._41 = p.pos.x; world._42 = p.pos.y; world._43 = p.pos.z;
                    p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
                    p.speed = 25;
                    p.look =  (arr_player[p_id].CurPos- npc[n_id].CurPos).normalized();
                    p.activeEnable = true;
                    p.liveTime = chrono::system_clock::now();
                    break;
                }
            }
        }
        else
        {      
            for (auto& p : arr_player)
            {
                if (npc[n_id].CurPos.x - 400 < p.CurPos.x && npc[n_id].CurPos.x + 400 > p.CurPos.x
                    && npc[n_id].CurPos.z - 400 < p.CurPos.z && npc[n_id].CurPos.z + 400 > p.CurPos.z)
                {
                    if (p.ps.block < Mathf::RandF(0, 100))
                    {
                        p.m_slock.lock();
                        p.ps.hp = Mathf::Max(p.ps.hp - 30, 0);
                        p.m_slock.unlock();
                    }
                }
            }
        }
        npc[n_id].attackPacketEnable = true;
        npc[n_id].isAttack = false;
    } 
    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc[n_id].timeLastAttack > chrono::milliseconds(npc[n_id].coolTime + 200))
    {
        npc[n_id].attackPacketEnable = false;
    }
    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc[n_id].timeLastAttack > chrono::milliseconds(2500))
    {
        npc[n_id].state = hit;
    }
    if (arr_player[p_id].ps.hp <= 0)
    {
        npc[n_id].state = none;
        npc[n_id].attackPacketEnable = false;
    }
}

//버그가 있음
void GameMgr::CheckPlayerDead(int p_id)
{
    if (arr_player[p_id].ps.hp <= 0 && arr_player[p_id].state != dead) {\
        arr_player[p_id].m_slock.lock();
        arr_player[p_id].timeDead = chrono::system_clock::now();
        arr_player[p_id].state = dead;
        arr_player[p_id].CurPos = arr_player[p_id].InitPos;
        arr_player[p_id].m_slock.unlock();
    }


    if (arr_player[p_id].state == dead && chrono::system_clock::now() - arr_player[p_id].timeDead > chrono::milliseconds(2000)) {
        player_status ps;
        arr_player[p_id].m_slock.lock();
        int hp = arr_player[p_id].ps.maxhp;
        arr_player[p_id].ps = ps;
        arr_player[p_id].ps.maxhp = arr_player[p_id].ps.hp = Mathf::Min(hp, 100);
        switch (arr_player[p_id].wp_type)
        {
        case WEAPON_RIFLE:
            arr_player[p_id].ps.attackSpeed = 0.15f;
            arr_player[p_id].ps.maxAmmo = 30;
            arr_player[p_id].ps.ammo = 30;
            arr_player[p_id].ps.attackDamage = 7.f;
            break;
        case WEAPON_SHOTGUN:
            arr_player[p_id].ps.attackSpeed = 0.5f;
            arr_player[p_id].ps.maxAmmo = 7;
            arr_player[p_id].ps.ammo = 7;
            arr_player[p_id].ps.attackDamage = 5.f;
            break;
        case WEAPON_SNIPER:
            arr_player[p_id].ps.attackSpeed = 1.0f;
            arr_player[p_id].ps.maxAmmo = 5;
            arr_player[p_id].ps.ammo = 5;
            arr_player[p_id].ps.attackDamage = 50.f;
            break;
        default:
            cout << "type error" << endl;
            break;
        }
        arr_player[p_id].activeItem = ITEM_EMPTY;
        arr_player[p_id].state = none;
        arr_player[p_id].m_slock.unlock();
    }
}

void GameMgr::keyInput(cs_ingame_packet cspacket)
{
}

void GameMgr::process_packet(int p_id, unsigned char* p_buf)
{
    cs_ingame_packet* cspacket = reinterpret_cast<cs_ingame_packet*>(p_buf);

        for (int i = 0; i < 5; ++i)
        {
            arr_player[p_id].bullet[i].in_use = false;
            arr_player[p_id].bullet[i].type = type_none;
        }
        arr_player[p_id].currentItem = ITEM_EMPTY;


        if (arr_player[p_id].state != dead)
        {
            if (cspacket->input.Key_W == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z + cspacket->look.z * arr_player[p_id].ps.speed;
                arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x + cspacket->look.x * arr_player[p_id].ps.speed;
            }
            if (cspacket->input.Key_S == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z - cspacket->look.z * arr_player[p_id].ps.speed;
                arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x - cspacket->look.x * arr_player[p_id].ps.speed;
            }
            if (cspacket->input.Key_A == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z + cspacket->look.x * arr_player[p_id].ps.speed;
                arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x - cspacket->look.z * arr_player[p_id].ps.speed;
            }
            if (cspacket->input.Key_D == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z - cspacket->look.x * arr_player[p_id].ps.speed;
                arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x + cspacket->look.z * arr_player[p_id].ps.speed;
            }
            if (!(cspacket->input.Key_W || cspacket->input.Key_S || cspacket->input.Key_A || cspacket->input.Key_D)) arr_player[p_id].state = ::none;
            if (cspacket->input.Key_Q == true) {
                if (arr_player[p_id].activeItem == ITEM_MAXHPUP)
                {
                    arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + 20, arr_player[p_id].ps.maxhp);
                }
                else if (arr_player[p_id].activeItem == ITEM_MONSTER_SLOW)
                {
                    isSlow = true;  
                    slowTime = chrono::system_clock::now();
                }
                arr_player[p_id].activeItem = ITEM_EMPTY;
            }
            if (cspacket->input.Key_B == true) {
                arr_player[p_id].CurPos = Vector3(487, 0, 589);
                arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->GetScale().y + 100;
                zoneLevel = 4;
            }
            if (cspacket->input.Key_N == true) {
                arr_player[p_id].CurPos = Vector3(557.519, 0, 1202.554);
                arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->GetScale().y + 100;
                zoneLevel = 3;
            }
            if (cspacket->input.Key_M == true) {
                arr_player[p_id].ps.attackDamage = 500;
            }
        }
        if (cspacket->input.Key_E == true) {
            PickInteractionObject(p_id);
        }

        if (cspacket->input.Key_R == true) {
            if(!arr_player[p_id].reloadEnable && arr_player[p_id].ps.ammo < arr_player[p_id].ps.maxAmmo)
            {
                arr_player[p_id].reloadEnable = true;
                arr_player[p_id].timeReload = chrono::system_clock::now();
            }
        }

        //아이템 처리
        if (cspacket->item.doSend)
        {
            int chestid = cspacket->item.chestId;
            int itemid = cspacket->item.itemId;
            if(!interaction[chestid].item.at(itemid).getEnable)
            { 
                arr_player[p_id].pl_items.find(interaction[chestid].item.at(itemid).item)->second += 1;
                interaction[chestid].item.at(itemid).getEnable = true;
                SetItem(p_id, interaction[chestid].item.at(itemid).item);
            }
        }

        if (cspacket->type == CS_SHOOT_PACKET && arr_player[p_id].state != dead) {
            if (arr_player[p_id].ps.ammo > 0 && !arr_player[p_id].reloadEnable) {
                arr_player[p_id].state = attack;
                arr_player[p_id].ps.ammo--;
                if (arr_player[p_id].wp_type == WEAPON_SHOTGUN)
                    FindCollideObjectShotGun(p_id);
                else
                    FindCollideObject(p_id);
            }

        }
        if (arr_player[p_id].ps.ammo == 0)
        {
            if (!arr_player[p_id].reloadEnable)
            {
                arr_player[p_id].reloadEnable = true;
                arr_player[p_id].timeReload = chrono::system_clock::now();
            }
        }
        if (chrono::system_clock::now() - arr_player[p_id].timeReload > chrono::milliseconds(2200) && arr_player[p_id].reloadEnable)
        {
            arr_player[p_id].ps.ammo = arr_player[p_id].ps.maxAmmo;
            arr_player[p_id].reloadEnable = false;
        }
    // gravity
    if (arr_player[p_id].CurPos.y > mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->GetScale().y + 100
        && arr_player[p_id].ps.hp >= 0) // ���� �߻�
        arr_player[p_id].CurPos.y -= 7;
    else
        arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->GetScale().y + 100;

    arr_player[p_id].pl_look = cspacket->look;
    arr_player[p_id].cam_look = cspacket->cameraLook;

    XMFLOAT4X4 danwi
    (
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.y, arr_player[p_id].CurPos.z, 1
    );

    arr_player[p_id].OOBB.Transform(arr_player[p_id].OOBB, DirectX::XMLoadFloat4x4(&danwi));
    arr_player[p_id].OOBB.Extents.x = OBB_SCALE_PLAYER_X;
    arr_player[p_id].OOBB.Extents.y = OBB_SCALE_PLAYER_Y;
    arr_player[p_id].OOBB.Extents.z = OBB_SCALE_PLAYER_Z;

    Player_CollCheck(p_id);
    CheckPlayerDead(p_id);

    arr_player[p_id].PrevPos = arr_player[p_id].CurPos;
}


void GameMgr::Player_CollCheck(int id)
{
    for (int i = 0; i < structure.size(); ++i) {
        ContainmentType containType = arr_player[id].OOBB.Contains(structure[i].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            break;
        case CONTAINS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            break;

        default:
            break;
        }
    }

    for (auto& p : interaction)
    {
        if (p.objectName == DOOR && p.interactEnable == true) continue;
        ContainmentType containType = arr_player[id].OOBB.Contains(p.OOBB);
        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            break;
        case CONTAINS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            break;
        default:
            break;
        }
    }

    for (int i = 0; i < map_lava.size(); ++i)
    {
        ContainmentType containType = map_lava[i].Contains(arr_player[id].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            if (arr_player[id].ps.hp > 0)
                arr_player[id].ps.hp -= 1;
            break;
        case CONTAINS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            if (arr_player[id].ps.hp > 0)
                arr_player[id].ps.hp -= 1;
            break;

        default:
            break;
        }
    }

}

void GameMgr::NPC_CollCheck(int id)

{
    for (int i = 0; i < structure.size(); ++i) {
        ContainmentType containType = npc[id].OOBB.Contains(structure[i].OOBB);

        switch (containType)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
        {
   /*         npc[id].state = none;*/
            npc[id].CurPos = npc[id].PrevPos;
            Vector3 lookvec = npc[id].InitPos - npc[id].CurPos;
            npc[id].Lookvec = lookvec.normalized();
            break;
        }
        case CONTAINS:
        {
            //npc[id].state = none;
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

bool GameMgr::CollideObjectByRayIntersection(BoundingOrientedBox objectBoundingBox, Vector3& position, Vector3& direction, float* distance)
{
    XMVECTOR xmRayOrigin = XMLoadFloat3(&position);
    XMVECTOR xmRayDirection = XMLoadFloat3(&direction);
    return objectBoundingBox.Intersects(xmRayOrigin, xmRayDirection, *distance);
} // ���� �߻�

void GameMgr::PickInteractionObject(int p_id)
{
    bool			isIntersected = false;
    float			fHitDistance = 75.f, fNearestHitDistance = 150.f;
    obj_type        object_type;
    object_type = type_none;
    int             object_id;
    Vector3         camera_pos = arr_player[p_id].CurPos;

    camera_pos.y += 20;
    Matrix4x4		matColidePosition = Matrix4x4::identity;


    for (int i = 0; i < MAX_INTRACTION; ++i) // monster check
    {
        isIntersected = CollideObjectByRayIntersection(interaction[i].OOBB, camera_pos, arr_player[p_id].cam_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_static;
            object_id = i;
        }
    }
    if (object_type == type_static)
    {
        if (interaction[object_id].interactEnable == false)
        {
            bool isOpen = true;
            if (interaction[object_id].zoneNum != 99)
            {
                for (auto p : gameZones.at(interaction[object_id].zoneNum).monsterID)
                {
                    if (npc[p].hp > 0) isOpen = false;
                }
                if (interaction[object_id].zoneNum == 3 && !interaction[12].interactEnable) isOpen = false;
                if (isOpen) zoneLevel = ++interaction[object_id].zoneNum;
            }
            interaction[object_id].interactEnable = isOpen;
        }
    }
}

void GameMgr::FindCollideObject(int p_id)
{
    Vector3         camera_pos = arr_player[p_id].CurPos;
    camera_pos.y += 20;
    bool			isIntersected = false;
    float			fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
    obj_type        object_type;
    object_type = type_none;
    int             object_id;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0)) arr_player[p_id].cam_look = Vector3(0, 1, 0);
    arr_player[p_id].bullet[0].in_use = true;

    Matrix4x4		matColidePosition = Matrix4x4::identity; // identity: 4x4 danwi
    for (int i = 0; i < MAX_OBJECT; ++i) // monster check
    {
        if (npc[i].hp <= 0) continue;
        isIntersected = CollideObjectByRayIntersection(npc[i].OOBB, camera_pos, arr_player[p_id].cam_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_npc;
            object_id = i;
        }
    }

    for (int i = 0; i < MAX_INTRACTION; ++i) // monster check
    {
        if (interaction[i].objectName == DOOR && interaction[i].interactEnable == true) continue;
        isIntersected = CollideObjectByRayIntersection(interaction[i].OOBB, camera_pos, arr_player[p_id].cam_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_static;
            object_id = i;
        }
    }

    for (int i = 0; i < structure.size(); ++i)
    {
        if (structure[i].type == fence) continue;
        isIntersected = CollideObjectByRayIntersection(structure[i].OOBB, camera_pos, arr_player[p_id].cam_look, &fHitDistance);
        if (isIntersected && (fHitDistance < fNearestHitDistance))
        {
            fNearestHitDistance = fHitDistance;
            object_type = type_static;
            object_id = i;
        }
    }

    ////////////////////////////////////////
    if (type_none != object_type)
    {
        float fDistance = static_cast<float>(pow(fNearestHitDistance, 2));
        float fSumLookPos = static_cast<float>(pow(arr_player[p_id].cam_look.x, 2)) + static_cast<float>(pow(arr_player[p_id].cam_look.y, 2)) + static_cast<float>(pow(arr_player[p_id].cam_look.z, 2));
        float fFinal = fDistance / fSumLookPos;
        // return collision CurPos (burn effects)
        matColidePosition._41 = camera_pos.x + arr_player[p_id].cam_look.x * (sqrt(fFinal) - 2);
        matColidePosition._42 = camera_pos.y + arr_player[p_id].cam_look.y * (sqrt(fFinal) - 2);
        matColidePosition._43 = camera_pos.z + arr_player[p_id].cam_look.z * (sqrt(fFinal) - 2);

        arr_player[p_id].bullet[0].pos.x = matColidePosition._41;
        arr_player[p_id].bullet[0].pos.y = matColidePosition._42;
        arr_player[p_id].bullet[0].pos.z = matColidePosition._43;
    }

    switch (object_type)
    {
    case type_none:
        arr_player[p_id].bullet[0].in_use = false;
        break;
    case type_npc:
        if (npc[object_id].state != dead)
        {
            arr_player[p_id].bullet[0].type = type_npc;
            for (int p : gameZones.at(npc[object_id].zoneNum).monsterID)
            {
                if (npc[p].state != none) continue;
                npc[p].destPl = p_id;
                npc[p].state = hit;
            }
            if (npc[object_id].destPl != p_id)
            {
                if (Vector3::Distance(npc[object_id].CurPos, arr_player[p_id].CurPos) < Vector3::Distance(npc[object_id].CurPos, arr_player[npc[object_id].destPl].CurPos))
                {
                    npc[object_id].destPl = p_id;
                    npc[object_id].state = hit;
                }
            }

            if (0 < npc[object_id].hp)
            {
                //몬스터 즉사 아이템 처리
                if (arr_player[p_id].ps.instantDeath > npc[object_id].hp && npc[object_id].mob != GOLEM)
                    npc[object_id].hp = 0;
                else
                {
                    float damage = arr_player[p_id].ps.attackDamage;
                    //보스 데미지 증가 아이템처리
                    if (npc[object_id].mob == GOLEM)
                        damage = damage + damage * arr_player[p_id].ps.bossDamage * 0.01;
                    npc[object_id].hp -= damage;
                    npc[object_id].hp = Mathf::Max(npc[object_id].hp, 0);
                }

                if (npc[object_id].hp == 0)
                {
                    arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + arr_player[p_id].ps.killMaxHp, arr_player[p_id].ps.maxhp);
                    npc[object_id].state = dead;
                    npc[object_id].timeDeath = chrono::system_clock::now();
                }
            }
            else
            {
                npc[object_id].state = dead;
                npc[object_id].timeDeath = chrono::system_clock::now();
            }
        }
        break;
    case type_static:
        arr_player[p_id].bullet[0].type = type_static;
        //
        break;
    case type_player:
        //
        break;
    }
}

void GameMgr::FindCollideObjectShotGun(int p_id)
{
    Vector3         camera_pos = arr_player[p_id].CurPos;
    camera_pos.y += 20;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0)) arr_player[p_id].cam_look = Vector3(0, 1, 0);

    for (int j = 0; j < 5; ++j)
    {
        bool			isIntersected = false;
        float			fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
        obj_type        object_type;
        object_type = type_none;
        int             object_id; 

        Vector3 camera_look = arr_player[p_id].cam_look;
        camera_look.x += Mathf::RandF(-0.15f, 0.15f);
        camera_look.y += Mathf::RandF(-0.15f, 0.15f);
        camera_look.z += Mathf::RandF(-0.15f, 0.15f);
        camera_look = camera_look.normalized();
        arr_player[p_id].bullet[j].in_use = true;
        Matrix4x4		matColidePosition = Matrix4x4::identity; // identity: 4x4 danwi
        for (int i = 0; i < MAX_OBJECT; ++i) // monster check
        {
            if (npc[i].hp <= 0) continue;
            isIntersected = CollideObjectByRayIntersection(npc[i].OOBB, camera_pos, camera_look, &fHitDistance);
            if (isIntersected && (fHitDistance < fNearestHitDistance))
            {
                fNearestHitDistance = fHitDistance;
                object_type = type_npc;
                object_id = i;
            }
        }

        for (int i = 0; i < MAX_INTRACTION; ++i) // monster check
        {
            if (interaction[i].objectName == DOOR && interaction[i].interactEnable == true) continue;
            isIntersected = CollideObjectByRayIntersection(interaction[i].OOBB, camera_pos, camera_look, &fHitDistance);
            if (isIntersected && (fHitDistance < fNearestHitDistance))
            {
                fNearestHitDistance = fHitDistance;
                object_type = type_static;
                object_id = i;
            }
        }

        for (int i = 0; i < structure.size(); ++i)
        {
            if (structure[i].type == fence) continue;
            isIntersected = CollideObjectByRayIntersection(structure[i].OOBB, camera_pos, camera_look, &fHitDistance);
            if (isIntersected && (fHitDistance < fNearestHitDistance))
            {
                fNearestHitDistance = fHitDistance;
                object_type = type_static;
                object_id = i;
            }
        }

        ////////////////////////////////////////
        if (type_none != object_type)
        {
            float fDistance = static_cast<float>(pow(fNearestHitDistance, 2));
            float fSumLookPos = static_cast<float>(pow(camera_look.x, 2)) + static_cast<float>(pow(camera_look.y, 2)) + static_cast<float>(pow(camera_look.z, 2));
            float fFinal = fDistance / fSumLookPos;
            // return collision CurPos (burn effects)
            matColidePosition._41 = camera_pos.x + camera_look.x * (sqrt(fFinal) - 2);
            matColidePosition._42 = camera_pos.y + camera_look.y * (sqrt(fFinal) - 2);
            matColidePosition._43 = camera_pos.z + camera_look.z * (sqrt(fFinal) - 2);

            arr_player[p_id].bullet[j].pos.x = matColidePosition._41;
            arr_player[p_id].bullet[j].pos.y = matColidePosition._42;
            arr_player[p_id].bullet[j].pos.z = matColidePosition._43;
        }

        switch (object_type)
        {
        case type_none:
            arr_player[p_id].bullet[j].in_use = false;
            break;
        case type_npc:
            if (npc[object_id].state != dead)
            {
                arr_player[p_id].bullet[j].type = type_npc;
                for (int p : gameZones.at(npc[object_id].zoneNum).monsterID)
                {
                    if (npc[p].state != none) continue;
                    npc[p].destPl = p_id;
                    npc[p].state = hit;
                }
                if (npc[object_id].destPl != p_id)
                {
                    if (Vector3::Distance(npc[object_id].CurPos, arr_player[p_id].CurPos) < Vector3::Distance(npc[object_id].CurPos, arr_player[npc[object_id].destPl].CurPos))
                    {
                        npc[object_id].destPl = p_id;
                        npc[object_id].state = hit;
                    }
                }

                if (0 < npc[object_id].hp)
                {
                    //몬스터 즉사 아이템 처리
                    if (arr_player[p_id].ps.instantDeath > npc[object_id].hp&& npc[object_id].mob != GOLEM)
                        npc[object_id].hp = 0;
                    else
                    {
                        float damage = arr_player[p_id].ps.attackDamage;
                        //보스 데미지 증가 아이템처리
                        if (npc[object_id].mob == GOLEM)
                            damage = damage + damage * arr_player[p_id].ps.bossDamage * 0.01;
                        npc[object_id].hp -= damage;
                        npc[object_id].hp = Mathf::Max(npc[object_id].hp, 0);
                    }

                    if (npc[object_id].hp == 0)
                    {
                        arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + arr_player[p_id].ps.killMaxHp, arr_player[p_id].ps.maxhp);
                        npc[object_id].state = dead;
                        npc[object_id].timeDeath = chrono::system_clock::now();
                    }
                }
                else
                {
                    npc[object_id].state = dead;
                    npc[object_id].timeDeath = chrono::system_clock::now();
                }
            }
            break;
        case type_static:
            arr_player[p_id].bullet[j].type = type_static;
            //
            break;
        case type_player:
            //
            break;
        }
    }
}

void GameMgr::CheckinteractionObject(int p_id)
{
}

void GameMgr::SetHP(int id, int hp)
{
    arr_player[id].ps.hp = hp;
}


void GameMgr::SetItem(int id, ITEM_TYPE item)
{
    arr_player[id].currentItem = item;
    switch (item)
    {
    case ITEM_TYPE::ITEM_DAMAGEUP_MAXHPDOWN:
        //데미지 증가 최대체력 감소 아이템처리
        arr_player[id].ps.maxhp /= 2;
        arr_player[id].ps.hp = Mathf::Min(arr_player[id].ps.maxhp, arr_player[id].ps.hp);
        arr_player[id].ps.attackDamage *= 2;
        break;
    case ITEM_TYPE::ITEM_BLOCK_DAMAGE:
        arr_player[id].ps.block += 10;
        break;
    case ITEM_TYPE::ITEM_BOSS_DAMAGEUP:
        arr_player[id].ps.bossDamage += 20;
        break;
    case ITEM_TYPE::ITEM_KILL_MAXHPUP:
        arr_player[id].ps.killMaxHp += 1;
        break;
    case ITEM_TYPE::ITEM_INSTANT_DEATH:
        if (arr_player[id].ps.instantDeath == 0)
            arr_player[id].ps.instantDeath += 10;
        else
            arr_player[id].ps.instantDeath += 5;
        break;
    case ITEM_TYPE::ITEM_MONSTER_SLOW:
        arr_player[id].activeItem = item;
        //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ITEM_TYPE::ITEM_PLAYER_SPEEDUP:
        //플레이어 스피드업 아이템처리
        arr_player[id].ps.speed *= 1.2;
        break;
    case ITEM_TYPE::ITEM_MAXHPUP:
        //최대 체력 증가 아이템 처리
        arr_player[id].activeItem = item;
        //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ITEM_TYPE::ITEM_ATTACK_SPEEDUP:
        arr_player[id].ps.attackSpeed /= 1.2f;
        break;
    }
}

Vector3 GameMgr::GetPositionToHeightMap(float x, float z, float addy)
{
    Vector3 pos;
    pos.x = x;
    pos.z = z;
    pos.y = mapData->GetHeight(x, z) * mapData->GetScale().y + addy;
    return pos;
}



sc_ingame_packet GameMgr::GetPacket(sc_ingame_packet packet)
{
    sc_ingame_packet scpacket;
    if (isEnding) {
        scpacket.type = SC_GAME_TO_ENDING_PACKET;
        chrono::duration<int> t = chrono::duration_cast<chrono::seconds> (e_time - s_time);
        scpacket.play_time = t.count();
    }
    else
        scpacket.type = SC_INGAME_PACKET;
    scpacket.size = sizeof(sc_ingame_packet);
    //cout << scpacket.size << endl;
    scpacket.id = 0;


    for (int i = 0; i < MAX_PLAYER; ++i)
    {
        int id = pl_list[i];

        scpacket.player[i].pos = arr_player[id].CurPos;
        scpacket.player[i].look = arr_player[id].pl_look;
        scpacket.player[i].cameraLook = arr_player[id].cam_look;
        scpacket.player[i].state = arr_player[id].state;
        scpacket.player[i].id = i;
        scpacket.player[i].zoneNum = zoneLevel;
        scpacket.player[i].ps.hp = arr_player[id].ps.hp;
        scpacket.player[i].ps.maxHp = arr_player[id].ps.maxhp;
        scpacket.player[i].ps.attackSpeed = arr_player[id].ps.attackSpeed;
        scpacket.player[i].reloadEnable = arr_player[id].reloadEnable;
        scpacket.player[i].ammo = arr_player[id].ps.ammo;
        memset(scpacket.player[i].bullet, NULL, sizeof(Bullet) * 5);
        memcpy(scpacket.player[i].bullet, arr_player[id].bullet, sizeof(Bullet) * 5);
        scpacket.player[i].currentItem = arr_player[id].currentItem;
    }

    for (int i = 0; i < MAX_OBJECT; ++i) // 오류 발생
    {
        scpacket.npc[i].id = i;
        scpacket.npc[i].pos = npc[i].CurPos;
        scpacket.npc[i].look = npc[i].Lookvec;
        scpacket.npc[i].hp = npc[i].hp;
        scpacket.npc[i].attackEnable = npc[i].attackPacketEnable;
        scpacket.npc[i].state = npc[i].state;
    }

    for (int i = 0; i < MAX_INTRACTION; ++i)
    {
        scpacket.interaction[i].pos = interaction[i].Pos;
        scpacket.interaction[i].look = interaction[i].Lookvec;
        if (interaction[i].item.size() != 0)
        {
            scpacket.interaction[i].item[0].itemType = interaction[i].item.at(0).item;
            scpacket.interaction[i].item[0].isAlive = !interaction[i].item.at(0).getEnable;
            scpacket.interaction[i].item[1].itemType = interaction[i].item.at(1).item;
            scpacket.interaction[i].item[1].isAlive = !interaction[i].item.at(1).getEnable;
            scpacket.interaction[i].item[2].itemType = interaction[i].item.at(2).item;
            scpacket.interaction[i].item[2].isAlive = !interaction[i].item.at(2).getEnable;
            scpacket.interaction[i].item[3].itemType = interaction[i].item.at(3).item;
            scpacket.interaction[i].item[3].isAlive = !interaction[i].item.at(3).getEnable;
        }
        scpacket.interaction[i].interactEnable = interaction[i].interactEnable;
        scpacket.interaction[i].state = interaction[i].state;
    }

    for (int i = 0; i < stoneAttack.size(); ++i)
    {
        scpacket.stone[i].activeEnable = stoneAttack[i].activeEnable;
        scpacket.stone[i].pos = stoneAttack[i].pos;
    }

    for (int i = 0; i < 20; ++i)
    {
        scpacket.attack[i].activeEnable = rangeAttack[i].activeEnable;
        scpacket.attack[i].pos = rangeAttack[i].pos;
    }

    return scpacket;
}
