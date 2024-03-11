#include "GameMgr.h"
#include "iostream"
#include <random>


GameMgr::GameMgr():
    currentZoneLevel_(0),
    roomId(0),
    playerIds{},
    mapData(std::make_unique<CHeightMapImage>(_T("mapImage/map1.raw"), MAP_WIDTH, MAP_HEIGHT, Vector3(SCALE_X, SCALE_Y, SCALE_Z))),
    currentUpdateTime(chrono::system_clock::now())
{
}


GameMgr::~GameMgr()
{
}

void GameMgr::InitGame(int id[4])
{
    float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f;
    zones_.reserve(5);
    currentZoneLevel_ = 0;
    lastStoneAttackTime_ = chrono::system_clock::now();
    roomId = arr_player[id[0]].r_id;

    //PLAYER
    {
        for (int i = 0; i < MAX_NUM_PLAYER; ++i)
        {
            playerIds[i] = id[i];
        }

        for (int i = 0; i != MAX_NUM_PLAYER; ++i)
        {
            int p_id = playerIds[i];
            arr_player[p_id].id = p_id;

            pos_x = 500 + p_id * ((2300.f - 1700.f) / MAX_NUM_PLAYER);
            pos_z = 4680.0f;
            pos_y = mapData->GetHeight(pos_x, pos_z) * mapData->GetScale().y + 100.f;

            Vector3 pos{pos_x, pos_y, pos_z};
            arr_player[i].InitPos = pos;
            arr_player[i].CurPos = pos;
            arr_player[i].PrevPos = pos;
            arr_player[i].status.hp = arr_player[i].status.maxhp = 100;

            switch (arr_player[i].wp_type)
            {
            case WeaponRifle:
                arr_player[i].status.attackSpeed = 0.15f;
                arr_player[i].status.maxAmmo = 30;
                arr_player[i].status.ammo = 30;
                arr_player[i].status.attackDamage = 7.f;
                break;
            case WeaponShotgun:
                arr_player[i].status.attackSpeed = 0.5f;
                arr_player[i].status.maxAmmo = 7;
                arr_player[i].status.ammo = 7;
                arr_player[i].status.attackDamage = 5.f;

                break;
            case WeaponSniper:
                arr_player[i].status.attackSpeed = 1.0f;
                arr_player[i].status.maxAmmo = 5;
                arr_player[i].status.ammo = 5;
                arr_player[i].status.attackDamage = 40.f;

                break;
            default:
                cout << "type error" << endl;
                break;
            }
            arr_player[i].state = NoneState;

            Vector3 look_vector = {pos_x - 2050.f, 0, pos_z - 1500.f};
            look_vector = look_vector.normalized();
            arr_player[i].pl_look = look_vector;
            arr_player[i].OOBB = BoundingOrientedBox(arr_player[i].CurPos,
                                                     XMFLOAT3(OBB_SCALE_PLAYER_X, OBB_SCALE_PLAYER_Y,
                                                              OBB_SCALE_PLAYER_Z), XMFLOAT4(0, 0, 0, 1));
            arr_player[i].cam_look = {1, 0, 0};

            for (int j = 0; j < ItemTypeCount; ++j)
            {
                auto item = static_cast<ITEM_TYPE>(j);
                arr_player[i].pl_items.insert({item, 0});
            }
        }
    }

    //NPC
    {
        npc_.reserve(MAX_NUM_OBJECT);

        NPC new_npc;
        pos_x = static_cast<float>(rand() % 2000 + 250);
        pos_z = static_cast<float>(rand() % 2000 + 250);
        new_npc.CurPos = {pos_x, mapData->GetHeight(pos_x, pos_z) * mapData->GetScale().y + 80, pos_z};
        new_npc.InitPos = new_npc.CurPos;
        new_npc.speed = 8;
        new_npc.state = NoneState;


        for (int i = 0; i < NUM_MAGMA_MONSTERS; ++i)
        {
            new_npc.mob = MAGMA;
            new_npc.OOBB.Extents = {OBB_SCALE_Magmaa_X, OBB_SCALE_Magmaa_Y, OBB_SCALE_Magmaa_Z};
            new_npc.hp = 200;
            new_npc.attackRange = 1300;
            new_npc.sight = 1500;
            new_npc.coolTime = 1000;
            npc_.emplace_back(new_npc);
        }
        for (int i = 0; i < NUM_GOLEM_MONSTER; ++i)
        {
            new_npc.mob = GOLEM;
            new_npc.OOBB.Extents = {OBB_SCALE_Golem_X, OBB_SCALE_Golem_Y, OBB_SCALE_Golem_Z};
            new_npc.hp = 3500;
            new_npc.attackRange = 300;
            new_npc.sight = 350;
            new_npc.speed = 10;
            new_npc.coolTime = 1800;
            npc_.emplace_back(new_npc);
        }
        for (int i = 0; i < NUM_ORGE_MONSTER; ++i)
        {
            new_npc.mob = OGRE;
            new_npc.OOBB.Extents = {OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z};
            new_npc.hp = 250;
            new_npc.attackRange = 150;
            new_npc.speed = 8;
            new_npc.sight = 500;
            new_npc.coolTime = 1000;
            npc_.emplace_back(new_npc);
        }

        for (int i = 0; i < MAX_NUM_OBJECT; ++i)
        {
            int rand = i % 4;
            switch (rand)
            {
            case 0:
                npc_[i].Lookvec = npc_[i].CurPos;
                npc_[i].Lookvec.x += 1;
                npc_[i].Lookvec = npc_[i].Lookvec.normalized();
                break;

            case 1:
                npc_[i].Lookvec = npc_[i].CurPos;
                npc_[i].Lookvec.x -= 1;
                npc_[i].Lookvec = npc_[i].Lookvec.normalized();
                break;

            case 2:
                npc_[i].Lookvec = npc_[i].CurPos;
                npc_[i].Lookvec.z += 1;
                npc_[i].Lookvec = npc_[i].Lookvec.normalized();
                break;

            case 3:
                npc_[i].Lookvec = npc_[i].CurPos;
                npc_[i].Lookvec.z -= 1;
                npc_[i].Lookvec = npc_[i].Lookvec.normalized();
                break;

            default:
                break;
            }
        }

        Vector3 mob_look;

        //중앙 방 2층 2마리
        npc_[0].CurPos.x = 1706.f;
        npc_[0].CurPos.z = 2045.f;
        npc_[0].zoneNum = 1;
        npc_[1].CurPos.x = 1705.f;
        npc_[1].CurPos.z = 3695.f;
        npc_[1].zoneNum = 1;

        //중앙방 출구후 방 1마리
        npc_[2].CurPos.x = 4790.f;
        npc_[2].CurPos.z = 1205.f;
        npc_[2].zoneNum = 2;
        mob_look = {1545.f, 0.f, 3296.f};

        for (int i = 0; i < 3; ++i)
        {
            npc_[i].Lookvec = mob_look - npc_[i].CurPos;
            npc_[i].Lookvec.y = 0.f;
            npc_[i].Lookvec = npc_[i].Lookvec.normalized();
        }
        //시작방 앞 2마리
        npc_[3].CurPos.x = 4675.15f;
        npc_[3].CurPos.z = 4515.6f;
        npc_[3].zoneNum = 0;
        npc_[4].CurPos.x = 4724.52f;
        npc_[4].CurPos.z = 4877.16f;
        npc_[4].zoneNum = 0;
        //중앙방 3마리
        npc_[5].CurPos.x = 4662.5f;
        npc_[5].CurPos.z = 2706.39f;
        npc_[5].zoneNum = 1;
        npc_[6].CurPos.x = 2006.f;
        npc_[6].CurPos.z = 2045.f;
        npc_[6].zoneNum = 1;
        npc_[7].CurPos.x = 1742.81;
        npc_[7].CurPos.z = 2972.67f;
        npc_[7].zoneNum = 1; //중앙 2층 한가운데

        //보스전방
        npc_[8].CurPos.x = 384;
        npc_[8].CurPos.z = 1209;
        npc_[8].zoneNum = 3;
        mob_look = {800.f, 0.f, 2671.f};

        for (int i = 3; i < 9; ++i)
        {
            npc_[i].Lookvec = mob_look - npc_[i].CurPos;
            npc_[i].Lookvec.y = 0.f;
            npc_[i].Lookvec = npc_[i].Lookvec.normalized();
        }

        //보스
        npc_[9].CurPos.x = 4760.f;
        npc_[9].CurPos.z = 575.f;
        npc_[9].zoneNum = 4;


        mob_look = {1545.f, 0.f, 3296.f};

        npc_[9].Lookvec = mob_look - npc_[9].CurPos;
        npc_[9].Lookvec.y = 0.f;
        npc_[9].Lookvec = npc_[9].Lookvec.normalized();

        npc_[9].PrevPos = npc_[9].CurPos;

        //시작방 앞 2마리
        npc_[10].CurPos.x = 3698.f;
        npc_[10].CurPos.z = 4907.26f;
        npc_[10].zoneNum = 0;
        npc_[11].CurPos.x = 3725.89f;
        npc_[11].CurPos.z = 4545.97f;
        npc_[11].zoneNum = 0;

        //중앙 방배치몹
        npc_[12].CurPos.x = 3298.f;
        npc_[12].CurPos.z = 2752.f;
        npc_[12].zoneNum = 1; //중앙방 맵 한가운데 왼쪽몹

        mob_look = {2029.f, 0.f, 1500.f};

        for (int i = 9; i < 13; ++i)
        {
            npc_[i].Lookvec = mob_look - npc_[i].CurPos;
            npc_[i].Lookvec.y = 0.f;
            npc_[i].Lookvec = npc_[i].Lookvec.normalized();
        }


        npc_[13].CurPos.x = 3255.0f;
        npc_[13].CurPos.z = 3703.09f;
        npc_[13].zoneNum = 1; //중앙방 한가운데 오른쪽몹
        npc_[14].CurPos.x = 4610.76f;
        npc_[14].CurPos.z = 2142.85f;
        npc_[14].zoneNum = 1; //중앙방 작은계단위

        //중앙방 출구 후 2마리
        npc_[15].CurPos.x = 4668.f;
        npc_[15].CurPos.z = 1640.f;
        npc_[15].zoneNum = 2; //출구 바라보고 왼쪽
        npc_[16].CurPos.x = 4240.f;
        npc_[16].CurPos.z = 1139.f;
        npc_[16].zoneNum = 2; //출구 바라보고 오른쪽

        //이상한놈들
        npc_[17].CurPos.x = 10036.f;
        npc_[17].CurPos.z = 3532.05f;
        npc_[17].zoneNum = 4;

        mob_look = {3256.f, 0.f, 2662.f};

        for (int i = 13; i < 18; ++i)
        {
            npc_[i].Lookvec = mob_look - npc_[i].CurPos;
            npc_[i].Lookvec.y = 0.f;
            npc_[i].Lookvec = npc_[i].Lookvec.normalized();
        }
        for (auto &p : npc_)
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
        interactions_.reserve(MAX_NUM_INTERACTION);

        std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<int> dis(0, 8);

        for (int i = 0; i < NUM_CHEST_OBJECT; ++i)
        {
            INTERACTION new_object;
            new_object.objectName = CHEST;
            new_object.OOBB.Extents = {OBB_SCALE_Chest_X, OBB_SCALE_Chest_Y, OBB_SCALE_Chest_Z};
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
            interactions_.emplace_back(new_object);
        }
        for (int i = 0; i < NUM_DOOR_OBJECT; ++i)
        {
            INTERACTION new_object;
            new_object.objectName = DOOR;
            new_object.OOBB.Extents = {OBB_SCALE_Door_X, OBB_SCALE_Door_Y, OBB_SCALE_Door_Z};
            new_object.zoneNum = 99;
            interactions_.emplace_back(new_object);
        }
        for (int i = 0; i < NUM_LEVER_OBJECT; ++i)
        {
            INTERACTION new_object;
            new_object.objectName = LEVER;
            new_object.OOBB.Extents = {OBB_SCALE_Lever_X, OBB_SCALE_Lever_Y, OBB_SCALE_Lever_Z};
            new_object.zoneNum = 99;
            interactions_.emplace_back(new_object);
        }

        INTERACTION new_object;
        new_object.objectName = MUD;
        new_object.OOBB.Extents = {0, 0, 0};
        interactions_.emplace_back(new_object);

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

        //  4699.98 4273.8
        //  3933.29 1773.41
        //  708.688 1029.33

        //상자
        interactions_[0].Pos = GetPositionToHeightMap(4915.81, 4840, 40); //시작방
        interactions_[1].Pos = GetPositionToHeightMap(660.232, 4105.35, 40); //마그마 미로방
        interactions_[2].Pos = GetPositionToHeightMap(860, 275, 40); //보스방앞
        interactions_[3].Pos = GetPositionToHeightMap(3555, 2000, 40); //중앙 방 내부 상자
        interactions_[4].Pos = GetPositionToHeightMap(1350, 1208, 40); //중앙 방 출구후 뒤 상자
        interactions_[5].Pos = GetPositionToHeightMap(1350, 4120, 40);
        //문                 
        interactions_[6].Pos = GetPositionToHeightMap(1165, 4700, 75); //시작방
        interactions_[7].Pos = GetPositionToHeightMap(1170, 1450, 75); //마그마방 직전
        interactions_[7].zoneNum = 2;
        interactions_[8].Pos = GetPositionToHeightMap(1165, 450, 75); //보스방 입구
        interactions_[9].Pos = GetPositionToHeightMap(3935, 1815, 90); //중앙방 출구
        interactions_[9].zoneNum = 1;
        interactions_[10].Pos = GetPositionToHeightMap(710, 1060, 70); //보스방 전전방
        interactions_[10].zoneNum = 3;
        interactions_[11].Pos = GetPositionToHeightMap(4700, 4242, 60); //중앙방 입구
        interactions_[11].zoneNum = 0;
        //레버
        //interaction[11].Pos = Vector3(1630, 540, 2680);

        interactions_[12].Pos = Vector3(360, 1475, 4250);

        Matrix4x4 world;
        Quaternion qua = Quaternion::AngleAxis(90, Vector3(0, 1, 0));
        world = Matrix4x4::Rotate(qua);
        interactions_[4].OOBB.Transform(interactions_[4].OOBB, XMLoadFloat4x4(&world));
        interactions_[6].OOBB.Transform(interactions_[6].OOBB, XMLoadFloat4x4(&world));
        interactions_[7].OOBB.Transform(interactions_[7].OOBB, XMLoadFloat4x4(&world));
        interactions_[8].OOBB.Transform(interactions_[8].OOBB, XMLoadFloat4x4(&world));
        for (auto &p : interactions_)
        {
            world = Matrix4x4::identity;
            world._41 = p.Pos.x, world._42 = p.Pos.y, world._43 = p.Pos.z;
            p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
            p.Lookvec = Vector3(-1, 0, 0);
            p.state = NoneState;
            p.interactEnable = false;
        }
        interactions_[4].Lookvec = Vector3(0, 0, 1);
    }

    rangeAttacks_.reserve(20);

    for (int i = 0; i < 20; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);

        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(20, 20, 20);
        ra.activeEnable = false;

        rangeAttacks_.push_back(ra);
    }

    stoneAttacks_.reserve(10);

    for (int i = 0; i < 5; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(50, 37, 75);
        ra.activeEnable = false;
        stoneAttacks_.push_back(ra);
    }
    for (int i = 0; i < 5; ++i)
    {
        RangeAttack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(30, 22.5, 37);
        ra.activeEnable = false;
        stoneAttacks_.push_back(ra);
    }
    // WALL
    {
        constexpr int x_objects = 20;
        constexpr int y_objects = 3;
        constexpr int z_objects = 7;

        constexpr float wall_size = 250;
        float x_pos, y_pos, z_pos;
        float height = 1000;

        for (int i = 0, x = 0; x < x_objects; x++)
        {
            for (int z = 0; z < z_objects; z++)
            {
                for (int y = 0; y < y_objects; y++)
                {
                    Structure structure;
                    structure.type = wall;

                    if (z == 0)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = z * wall_size * 3 + 100;
                        y_pos = height + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);

                        x_pos = z * wall_size * 3 + 100;
                        z_pos = x * wall_size + 200;
                        y_pos = height + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);
                    }

                    if (z == z_objects - 1)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 5045.f;
                        y_pos = height + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);


                        x_pos = 5045.f;
                        z_pos = x * wall_size + 200;
                        y_pos = height + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);
                    }

                    if ((x != 1 && x != 5 && x != x_objects - 2) || y != 0)
                    {
                        x_pos = 1170;
                        z_pos = x * wall_size + 200;
                        y_pos = height + 55 + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);
                    }

                    if ((x != 15 || y != 0))
                    {
                        if (x != 1 && x != 2 || y == 2)
                        {
                            x_pos = x * wall_size + 200;
                            z_pos = 1815;
                            y_pos = height - 50 + wall_size * (0.5f + y);
                            structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                            structure.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                            structure.OOBB = BoundingOrientedBox(structure.center, structure.extend,
                                                                 XMFLOAT4(0, 0, 0, 1));
                            structures_.push_back(structure);
                        }
                    }

                    if ((x != x_objects - 2 || y == 2))
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 4245.f;
                        y_pos = height + 40 + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);
                    }
                    if (x != 2 || y != 0)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 1060;
                        y_pos = height + 50 + wall_size * (0.5f + y);
                        structure.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        structure.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        structure.OOBB = BoundingOrientedBox(structure.center, structure.extend, XMFLOAT4(0, 0, 0, 1));
                        structures_.push_back(structure);
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
        structures_.push_back(st);

        //계단 난간

        xPos = 2410;
        yPos = 1050;
        zPos = 2300;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(270, 300, 40));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structures_.push_back(st);

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
        structures_.push_back(st);
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
        structures_.push_back(st);

        //입구에서 출구바라보고 계단오른쪽난간
        xPos = 4276.01;
        yPos = 1050;
        zPos = 4365;
        st.center = (XMFLOAT3(xPos, yPos, zPos));
        st.extend = (XMFLOAT3(40, 100, 120));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structures_.push_back(st);
    }

    //중앙방 출구 후 방 계단난간
    {
        Structure st;

        st.type = fence;

        st.center = (XMFLOAT3(3550, 1050, 1564));
        st.extend = (XMFLOAT3(40, 100, 200));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structures_.push_back(st);
    }

    BoundingOrientedBox LAVA;
    LAVA.Center = {665.265, 1260, 3328.36};
    LAVA.Extents = {800, 1.f, 800};
    lavaZones_.emplace_back(LAVA);

    ZONE zone;
    zone.isClear = false;
    //시작방
    zone.monsterID = {3, 4, 10, 11};
    zones_.push_back(zone);
    //중앙방
    zone.monsterID = {0, 1, 5, 6, 7, 12, 13, 14};
    zones_.push_back(zone);
    zone.monsterID = {2, 15, 16};
    zones_.push_back(zone);
    //안개방?
    zone.monsterID = {8};
    zones_.push_back(zone);
    //보스방
    zone.monsterID = {9};
    zones_.push_back(zone);

    isRunning = true;
}

void GameMgr::Update()
{
    for (int n_id = 0; n_id < MAX_NUM_OBJECT; ++n_id)
    {
        switch (npc_[n_id].state)
        {
        case NoneState:
            for (auto &p : arr_player)
            {
                if (find(zones_.at(currentZoneLevel_).monsterID.begin(), zones_.at(currentZoneLevel_).monsterID.end(),
                         n_id) == zones_.at(currentZoneLevel_).monsterID.end())
                    continue;
                if (p.status.hp <= 0)
                    continue;
                if (Vector3::Distance(npc_[n_id].CurPos, p.CurPos) < npc_[n_id].sight)
                {
                    npc_[n_id].destPl = p.id;
                    npc_[n_id].state = Hit;
                }
            }
            break;
        case Hit:
            TracePlayer(n_id);
            break;

        case Attacking:
            AttackPlayer(n_id);
            break;

        case Dead:
        {
            if (chrono::system_clock::now() - npc_[n_id].time_death > chrono::milliseconds(2000))
            {
                npc_[n_id].CurPos.x = -1000.f;
                npc_[n_id].CurPos.z = -1000.f;
                npc_[n_id].PrevPos.x = -1000.f;
                npc_[n_id].PrevPos.z = -1000.f;
            }
            break;
        }
        default:
            break;
        }

        //gravity
        if (npc_[n_id].CurPos.y > mapData->GetHeight(npc_[n_id].CurPos.x, npc_[n_id].CurPos.z) * mapData->GetScale().y +
            80)
            npc_[n_id].CurPos.y -= 5;
        else
            npc_[n_id].CurPos.y = mapData->GetHeight(npc_[n_id].CurPos.x, npc_[n_id].CurPos.z) * mapData->GetScale().y +
                80;


        if (npc_[n_id].CurPos != npc_[n_id].PrevPos)
        {
            Vector3 Lookvec = npc_[n_id].CurPos - npc_[n_id].PrevPos;
            Lookvec.y = 0.f;
            npc_[n_id].Lookvec = Lookvec.normalized();
        }

        XMFLOAT4X4 danwi
            (
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                npc_[n_id].CurPos.x, npc_[n_id].CurPos.y, npc_[n_id].CurPos.z, 1
                );


        npc_[n_id].OOBB.Transform(npc_[n_id].OOBB, XMLoadFloat4x4(&danwi));

        switch (npc_[n_id].mob)
        {
        case MAGMA:
            npc_[n_id].OOBB.Extents = {OBB_SCALE_Magmaa_X, OBB_SCALE_Magmaa_Y, OBB_SCALE_Magmaa_Z};
            break;
        case GOLEM:
            npc_[n_id].OOBB.Extents = {OBB_SCALE_Golem_X, OBB_SCALE_Golem_Y, OBB_SCALE_Golem_Z};
            break;
        case OGRE:
            npc_[n_id].OOBB.Extents = {OBB_SCALE_Orge_X, OBB_SCALE_Orge_Y, OBB_SCALE_Orge_Z};
            break;
        }

        // init lookvec
        if (npc_[n_id].Lookvec.x == 0.f && npc_[n_id].Lookvec.y == 0.f && npc_[n_id].Lookvec.z == 0.f)
            npc_[n_id].Lookvec = {1, 0, 0};


        CheckNPCCollision(n_id);

        npc_[n_id].PrevPos = npc_[n_id].CurPos;
    }

    for (auto &ra : rangeAttacks_)
    {
        if (!ra.activeEnable)
            continue;
        ra.pos = ra.pos + ra.look * ra.speed;
        ra.OOBB.Center = ra.pos;
        if (chrono::system_clock::now() - ra.liveTime > chrono::seconds(3))
        {
            ra.activeEnable = false;
            continue;
        }
        for (auto &p : arr_player)
        {
            if (ra.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].status.block < Mathf::RandF(0, 100))
                    arr_player[p.id].status.hp = Mathf::Max(arr_player[p.id].status.hp - 10, 0);
                ra.activeEnable = false;

                cout << "range Attack" << endl;
                break;
            }
        }
        for (auto &p : structures_)
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
    if (currentZoneLevel_ == 4 && chrono::system_clock::now() - lastStoneAttackTime_ > chrono::milliseconds(500))
    {
        for (auto &st : stoneAttacks_)
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
        lastStoneAttackTime_ = chrono::system_clock::now();
    }

    for (auto &st : stoneAttacks_)
    {
        if (!st.activeEnable)
            continue;
        st.pos.y -= st.speed;
        st.OOBB.Center = st.pos;
        if (chrono::system_clock::now() - st.liveTime > chrono::seconds(4))
        {
            st.activeEnable = false;
            continue;
        }
        for (auto &p : arr_player)
        {
            if (st.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].status.block < Mathf::RandF(0, 100))
                    arr_player[p.id].status.hp = Mathf::Max(arr_player[p.id].status.hp - 20, 0);
                st.activeEnable = false;
                break;
            }
        }
    }

    if (isSlowed_ && chrono::system_clock::now() - slowEffectEndTime > chrono::seconds(20))
        isSlowed_ = false;

    if (npc_[9].state == Dead && chrono::system_clock::now() - npc_[9].time_death > chrono::seconds(4))
    {
        if (isEnding == false)
        {
            isEnding = true;
            endTime = chrono::system_clock::now();
        }
    }

    currentUpdateTime = chrono::system_clock::now();
}


void GameMgr::TracePlayer(int n_id)
{
    int p_id = npc_[n_id].destPl;
    if (arr_player[p_id].status.hp <= 0)
    {
        npc_[n_id].state = NoneState;
        return;
    }
    Vector3 player_pos = arr_player[p_id].CurPos;
    player_pos.y -= 20;
    float dis = Vector3::Distance(npc_[n_id].CurPos, player_pos);
    float speed = npc_[n_id].speed;
    if (isSlowed_)
        speed *= 0.5;

    if (npc_[n_id].attackRange < dis)
        npc_[n_id].CurPos = Vector3::MoveTowards(npc_[n_id].CurPos, player_pos, speed);
    else
    {
        npc_[n_id].state = Attacking;
        npc_[n_id].isAttack = true;
        npc_[n_id].time_last_attack = chrono::system_clock::now();
    }
}


void GameMgr::AttackPlayer(int n_id)
{
    int p_id = npc_[n_id].destPl;

    if (0 < arr_player[p_id].status.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack >
        chrono::milliseconds(npc_[n_id].coolTime) && npc_[n_id].isAttack)
    {
        if (npc_[n_id].mob == OGRE)
        {
            if (arr_player[p_id].status.block < Mathf::RandF(0, 100))
            {
                arr_player[p_id].m_slock.lock();
                arr_player[p_id].status.hp = Mathf::Max(arr_player[p_id].status.hp - 10, 0);
                arr_player[p_id].m_slock.unlock();
            }
        }
        else if (npc_[n_id].mob == MAGMA)
        {
            for (auto &p : rangeAttacks_)
            {
                Matrix4x4 world = Matrix4x4::identity;
                if (!p.activeEnable)
                {
                    p.pos = npc_[n_id].CurPos + npc_[n_id].Lookvec * 30;
                    world._41 = p.pos.x;
                    world._42 = p.pos.y;
                    world._43 = p.pos.z;
                    p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
                    p.speed = 25;
                    p.look = (arr_player[p_id].CurPos - npc_[n_id].CurPos).normalized();
                    p.activeEnable = true;
                    p.liveTime = chrono::system_clock::now();
                    break;
                }
            }
        }
        else
        {
            for (auto &p : arr_player)
            {
                if (npc_[n_id].CurPos.x - 400 < p.CurPos.x && npc_[n_id].CurPos.x + 400 > p.CurPos.x
                    && npc_[n_id].CurPos.z - 400 < p.CurPos.z && npc_[n_id].CurPos.z + 400 > p.CurPos.z)
                {
                    if (p.status.block < Mathf::RandF(0, 100))
                    {
                        p.m_slock.lock();
                        p.status.hp = Mathf::Max(p.status.hp - 30, 0);
                        p.m_slock.unlock();
                    }
                }
            }
        }
        npc_[n_id].attackPacketEnable = true;
        npc_[n_id].isAttack = false;
    }
    if (0 < arr_player[p_id].status.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack >
        chrono::milliseconds(npc_[n_id].coolTime + 200))
    {
        npc_[n_id].attackPacketEnable = false;
    }
    if (0 < arr_player[p_id].status.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack >
        chrono::milliseconds(2500))
    {
        npc_[n_id].state = Hit;
    }
    if (arr_player[p_id].status.hp <= 0)
    {
        npc_[n_id].state = NoneState;
        npc_[n_id].attackPacketEnable = false;
    }
}

//버그가 있음
void GameMgr::CheckPlayerDead(const int p_id)
{
    if (arr_player[p_id].status.hp <= 0 && arr_player[p_id].state != Dead)
    {\
        arr_player[p_id].m_slock.lock();
        arr_player[p_id].timeDead = chrono::system_clock::now();
        arr_player[p_id].state = Dead;
        arr_player[p_id].CurPos = arr_player[p_id].InitPos;
        arr_player[p_id].m_slock.unlock();
    }


    if (arr_player[p_id].state == Dead && chrono::system_clock::now() - arr_player[p_id].timeDead >
        chrono::milliseconds(2000))
    {
        player_status ps;
        arr_player[p_id].m_slock.lock();
        const int hp = arr_player[p_id].status.maxhp;
        arr_player[p_id].status = ps;
        arr_player[p_id].status.maxhp = arr_player[p_id].status.hp = Mathf::Min(hp, 100);
        switch (arr_player[p_id].wp_type)
        {
        case WeaponRifle:
            arr_player[p_id].status.attackSpeed = 0.15f;
            arr_player[p_id].status.maxAmmo = 30;
            arr_player[p_id].status.ammo = 30;
            arr_player[p_id].status.attackDamage = 7.f;
            break;
        case WeaponShotgun:
            arr_player[p_id].status.attackSpeed = 0.5f;
            arr_player[p_id].status.maxAmmo = 7;
            arr_player[p_id].status.ammo = 7;
            arr_player[p_id].status.attackDamage = 5.f;
            break;
        case WeaponSniper:
            arr_player[p_id].status.attackSpeed = 1.0f;
            arr_player[p_id].status.maxAmmo = 5;
            arr_player[p_id].status.ammo = 5;
            arr_player[p_id].status.attackDamage = 50.f;
            break;
        default:
            cout << "type error" << endl;
            break;
        }
        arr_player[p_id].activeItem = ItemEmpty;
        arr_player[p_id].state = NoneState;
        arr_player[p_id].m_slock.unlock();
    }
}

void GameMgr::ProcessKeyInput(cs_ingame_packet cspacket)
{
}

void GameMgr::ProcessPacket(int p_id, unsigned char *p_buf)
{
    const cs_ingame_packet *cs_packet = reinterpret_cast<cs_ingame_packet *>(p_buf);

    for (int i = 0; i < 5; ++i)
    {
        arr_player[p_id].bullet[i].isInUse = false;
        arr_player[p_id].bullet[i].type = NoneType;
    }
    arr_player[p_id].currentItem = ItemEmpty;


    if (arr_player[p_id].state != Dead)
    {
        if (cs_packet->input.Key_W == true)
        {
            arr_player[p_id].state = Moving;
            arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z + cs_packet->lookDirection.z * arr_player[p_id].status
                .speed;
            arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x + cs_packet->lookDirection.x * arr_player[p_id].status
                .speed;
        }
        if (cs_packet->input.Key_S == true)
        {
            arr_player[p_id].state = Moving;
            arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z - cs_packet->lookDirection.z * arr_player[p_id].status
                .speed;
            arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x - cs_packet->lookDirection.x * arr_player[p_id].status
                .speed;
        }
        if (cs_packet->input.Key_A == true)
        {
            arr_player[p_id].state = Moving;
            arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z + cs_packet->lookDirection.x * arr_player[p_id].status
                .speed;
            arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x - cs_packet->lookDirection.z * arr_player[p_id].status
                .speed;
        }
        if (cs_packet->input.Key_D == true)
        {
            arr_player[p_id].state = Moving;
            arr_player[p_id].CurPos.z = arr_player[p_id].CurPos.z - cs_packet->lookDirection.x * arr_player[p_id].status
                .speed;
            arr_player[p_id].CurPos.x = arr_player[p_id].CurPos.x + cs_packet->lookDirection.z * arr_player[p_id].status
                .speed;
        }
        if (!(cs_packet->input.Key_W || cs_packet->input.Key_S || cs_packet->input.Key_A || cs_packet->input.Key_D))
            arr_player[p_id].state =
                NoneState;
        if (cs_packet->input.Key_Q == true)
        {
            if (arr_player[p_id].activeItem == ItemMaxHpUp)
            {
                arr_player[p_id].status.hp = Mathf::Min(arr_player[p_id].status.hp + 20, arr_player[p_id].status.maxhp);
            }
            else if (arr_player[p_id].activeItem == ItemMonsterSlow)
            {
                isSlowed_ = true;
                slowEffectEndTime = chrono::system_clock::now();
            }
            arr_player[p_id].activeItem = ItemEmpty;
        }
        if (cs_packet->input.Key_B == true)
        {
            arr_player[p_id].CurPos = Vector3(487, 0, 589);
            arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) *
                mapData->GetScale().y + 100;
            currentZoneLevel_ = 4;
        }
        if (cs_packet->input.Key_N == true)
        {
            arr_player[p_id].CurPos = Vector3(557.519, 0, 1202.554);
            arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) *
                mapData->GetScale().y + 100;
            currentZoneLevel_ = 3;
        }
        if (cs_packet->input.Key_M == true)
        {
            arr_player[p_id].status.attackDamage = 500;
        }
    }
    if (cs_packet->input.Key_E == true)
    {
        PickInteractionObject(p_id);
    }

    if (cs_packet->input.Key_R == true)
    {
        if (!arr_player[p_id].reloadEnable && arr_player[p_id].status.ammo < arr_player[p_id].status.maxAmmo)
        {
            arr_player[p_id].reloadEnable = true;
            arr_player[p_id].timeReload = chrono::system_clock::now();
        }
    }

    //아이템 처리
    if (cs_packet->item.doSend)
    {
        const int chest_id = cs_packet->item.chestId;
        const int item_id = cs_packet->item.itemId;
        if (!interactions_[chest_id].item.at(item_id).getEnable)
        {
            arr_player[p_id].pl_items.find(interactions_[chest_id].item.at(item_id).item)->second += 1;
            interactions_[chest_id].item.at(item_id).getEnable = true;
            SetItem(p_id, interactions_[chest_id].item.at(item_id).item);
        }
    }

    if (cs_packet->type == CS_SHOOT_PACKET && arr_player[p_id].state != Dead)
    {
        if (arr_player[p_id].status.ammo > 0 && !arr_player[p_id].reloadEnable)
        {
            arr_player[p_id].state = Attacking;
            arr_player[p_id].status.ammo--;
            if (arr_player[p_id].wp_type == WeaponShotgun)
                FindCollideObjectShotGun(p_id);
            else
                FindCollideObject(p_id);
        }

    }
    if (arr_player[p_id].status.ammo == 0)
    {
        if (!arr_player[p_id].reloadEnable)
        {
            arr_player[p_id].reloadEnable = true;
            arr_player[p_id].timeReload = chrono::system_clock::now();
        }
    }
    if (chrono::system_clock::now() - arr_player[p_id].timeReload > chrono::milliseconds(2200) && arr_player[p_id].
        reloadEnable)
    {
        arr_player[p_id].status.ammo = arr_player[p_id].status.maxAmmo;
        arr_player[p_id].reloadEnable = false;
    }
    // gravity
    if (arr_player[p_id].CurPos.y > mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->
        GetScale().y + 100
        && arr_player[p_id].status.hp >= 0) // ���� �߻�
        arr_player[p_id].CurPos.y -= 7;
    else
        arr_player[p_id].CurPos.y = mapData->GetHeight(arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.z) * mapData->
            GetScale().y + 100;

    arr_player[p_id].pl_look = cs_packet->lookDirection;
    arr_player[p_id].cam_look = cs_packet->cameraLook;

    XMFLOAT4X4 danwi
        (
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            arr_player[p_id].CurPos.x, arr_player[p_id].CurPos.y, arr_player[p_id].CurPos.z, 1
            );

    arr_player[p_id].OOBB.Transform(arr_player[p_id].OOBB, XMLoadFloat4x4(&danwi));
    arr_player[p_id].OOBB.Extents.x = OBB_SCALE_PLAYER_X;
    arr_player[p_id].OOBB.Extents.y = OBB_SCALE_PLAYER_Y;
    arr_player[p_id].OOBB.Extents.z = OBB_SCALE_PLAYER_Z;

    CheckPlayerCollision(p_id);
    CheckPlayerDead(p_id);

    arr_player[p_id].PrevPos = arr_player[p_id].CurPos;
}


void GameMgr::CheckPlayerCollision(const int id) const
{
    for (int i = 0; i < structures_.size(); ++i)
    {
        const ContainmentType contain_type = arr_player[id].OOBB.Contains(structures_[i].OOBB);

        switch (contain_type)
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

    for (auto &p : interactions_)
    {
        if (p.objectName == DOOR && p.interactEnable == true)
            continue;
        const ContainmentType contain_type = arr_player[id].OOBB.Contains(p.OOBB);
        switch (contain_type)
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

    for (int i = 0; i < lavaZones_.size(); ++i)
    {
        const ContainmentType contain_type = lavaZones_[i].Contains(arr_player[id].OOBB);

        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            if (arr_player[id].status.hp > 0)
                arr_player[id].status.hp -= 1;
            break;
        case CONTAINS:
            arr_player[id].CurPos = arr_player[id].PrevPos;
            if (arr_player[id].status.hp > 0)
                arr_player[id].status.hp -= 1;
            break;

        default:
            break;
        }
    }

}

void GameMgr::CheckNPCCollision(int id)

{
    for (int i = 0; i < structures_.size(); ++i)
    {
        const ContainmentType contain_type = npc_[id].OOBB.Contains(structures_[i].OOBB);

        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
        {
            /*         npc[id].state = none;*/
            npc_[id].CurPos = npc_[id].PrevPos;
            Vector3 look_vec = npc_[id].InitPos - npc_[id].CurPos;
            npc_[id].Lookvec = look_vec.normalized();
            break;
        }
        case CONTAINS:
        {
            //npc[id].state = none;
            npc_[id].CurPos = npc_[id].PrevPos;
            Vector3 look_vec = npc_[id].InitPos - npc_[id].CurPos;
            npc_[id].Lookvec = look_vec.normalized();
            break;
        }
        default:
            break;
        }
    }

}

bool GameMgr::CollideObjectByRayIntersection(const BoundingOrientedBox &objectBoundingBox, const Vector3 &position,
                                             const Vector3 &direction, float *distance)
{
    const XMVECTOR ray_origin = XMLoadFloat3(&position);
    const XMVECTOR ray_direction = XMLoadFloat3(&direction);
    return objectBoundingBox.Intersects(ray_origin, ray_direction, *distance);
} // ���� �߻�

void GameMgr::PickInteractionObject(const int p_id)
{
    bool isIntersected = false;
    float hit_distance = 75.f, nearest_hit_distance = 150.f;
    ObjectType object_type = NoneType;
    int object_id = 0;
    Vector3 camera_pos = arr_player[p_id].CurPos;

    camera_pos.y += 20;
    Matrix4x4 collide_position = Matrix4x4::identity;


    for (int i = 0; i < MAX_NUM_INTERACTION; ++i) // monster check
    {
        isIntersected = CollideObjectByRayIntersection(interactions_[i].OOBB, camera_pos, arr_player[p_id].cam_look,
                                                       &hit_distance);
        if (isIntersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = StaticType;
            object_id = i;
        }
    }
    if (object_type == StaticType)
    {
        if (interactions_[object_id].interactEnable == false)
        {
            bool isOpen = true;
            if (interactions_[object_id].zoneNum != 99)
            {
                for (auto p : zones_.at(interactions_[object_id].zoneNum).monsterID)
                {
                    if (npc_[p].hp > 0)
                        isOpen = false;
                }
                if (interactions_[object_id].zoneNum == 3 && !interactions_[12].interactEnable)
                    isOpen = false;
                if (isOpen)
                    currentZoneLevel_ = ++interactions_[object_id].zoneNum;
            }
            interactions_[object_id].interactEnable = isOpen;
        }
    }
}

void GameMgr::FindCollideObject(int p_id)
{
    Vector3 camera_pos = arr_player[p_id].CurPos;
    camera_pos.y += 20;
    bool isIntersected = false;
    float hit_distance = FLT_MAX, nearest_hit_distance = FLT_MAX;
    ObjectType object_type = NoneType;
    int object_id = 0;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0))
        arr_player[p_id].cam_look = Vector3(0, 1, 0);
    arr_player[p_id].bullet[0].isInUse = true;

    Matrix4x4 collide_position = Matrix4x4::identity; // identity: 4x4 danwi
    for (int i = 0; i < MAX_NUM_OBJECT; ++i) // monster check
    {
        if (npc_[i].hp <= 0)
            continue;
        isIntersected = CollideObjectByRayIntersection(npc_[i].OOBB, camera_pos, arr_player[p_id].cam_look,
                                                       &hit_distance);
        if (isIntersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = NPCType;
            object_id = i;
        }
    }

    for (int i = 0; i < MAX_NUM_INTERACTION; ++i) // monster check
    {
        if (interactions_[i].objectName == DOOR && interactions_[i].interactEnable == true)
            continue;
        isIntersected = CollideObjectByRayIntersection(interactions_[i].OOBB, camera_pos, arr_player[p_id].cam_look,
                                                       &hit_distance);
        if (isIntersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = StaticType;
            object_id = i;
        }
    }

    for (int i = 0; i < structures_.size(); ++i)
    {
        if (structures_[i].type == fence)
            continue;
        isIntersected = CollideObjectByRayIntersection(structures_[i].OOBB, camera_pos, arr_player[p_id].cam_look,
                                                       &hit_distance);
        if (isIntersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = StaticType;
            object_id = i;
        }
    }

    ////////////////////////////////////////
    if (NoneType != object_type)
    {
        float fDistance = static_cast<float>(pow(nearest_hit_distance, 2));
        float fSumLookPos = static_cast<float>(pow(arr_player[p_id].cam_look.x, 2)) + static_cast<float>(
            pow(arr_player[p_id].cam_look.y, 2)) + static_cast<float>(pow(arr_player[p_id].cam_look.z, 2));
        float fFinal = fDistance / fSumLookPos;
        // return collision CurPos (burn effects)
        collide_position._41 = camera_pos.x + arr_player[p_id].cam_look.x * (sqrt(fFinal) - 2);
        collide_position._42 = camera_pos.y + arr_player[p_id].cam_look.y * (sqrt(fFinal) - 2);
        collide_position._43 = camera_pos.z + arr_player[p_id].cam_look.z * (sqrt(fFinal) - 2);

        arr_player[p_id].bullet[0].position.x = collide_position._41;
        arr_player[p_id].bullet[0].position.y = collide_position._42;
        arr_player[p_id].bullet[0].position.z = collide_position._43;
    }

    switch (object_type)
    {
    case NoneType:
        arr_player[p_id].bullet[0].isInUse = false;
        break;
    case NPCType:
        if (npc_[object_id].state != Dead)
        {
            arr_player[p_id].bullet[0].type = NPCType;
            for (int p : zones_.at(npc_[object_id].zoneNum).monsterID)
            {
                if (npc_[p].state != NoneState)
                    continue;
                npc_[p].destPl = p_id;
                npc_[p].state = Hit;
            }
            if (npc_[object_id].destPl != p_id)
            {
                if (Vector3::Distance(npc_[object_id].CurPos, arr_player[p_id].CurPos) < Vector3::Distance(
                    npc_[object_id].CurPos, arr_player[npc_[object_id].destPl].CurPos))
                {
                    npc_[object_id].destPl = p_id;
                    npc_[object_id].state = Hit;
                }
            }

            if (0 < npc_[object_id].hp)
            {
                //몬스터 즉사 아이템 처리
                if (arr_player[p_id].status.instantDeath > npc_[object_id].hp && npc_[object_id].mob != GOLEM)
                    npc_[object_id].hp = 0;
                else
                {
                    float damage = arr_player[p_id].status.attackDamage;
                    //보스 데미지 증가 아이템처리
                    if (npc_[object_id].mob == GOLEM)
                        damage = damage + damage * arr_player[p_id].status.bossDamage * 0.01;
                    npc_[object_id].hp -= damage;
                    npc_[object_id].hp = Mathf::Max(npc_[object_id].hp, 0);
                }

                if (npc_[object_id].hp == 0)
                {
                    arr_player[p_id].status.hp = Mathf::Min(
                        arr_player[p_id].status.hp + arr_player[p_id].status.killMaxHp, arr_player[p_id].status.maxhp);
                    npc_[object_id].state = Dead;
                    npc_[object_id].time_death = chrono::system_clock::now();
                }
            }
            else
            {
                npc_[object_id].state = Dead;
                npc_[object_id].time_death = chrono::system_clock::now();
            }
        }
        break;
    case StaticType:
        arr_player[p_id].bullet[0].type = StaticType;
    //
        break;
    case PlayerType:
        //
        break;
    }
}

void GameMgr::FindCollideObjectShotGun(int p_id)
{
    Vector3 camera_pos = arr_player[p_id].CurPos;
    camera_pos.y += 20;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0))
        arr_player[p_id].cam_look = Vector3(0, 1, 0);

    for (int j = 0; j < 5; ++j)
    {
        bool isIntersected = false;
        float hit_distance = FLT_MAX, nearest_hit_distance = FLT_MAX;
        ObjectType object_type = NoneType;
        int object_id = 0;

        Vector3 camera_look = arr_player[p_id].cam_look;
        camera_look.x += Mathf::RandF(-0.15f, 0.15f);
        camera_look.y += Mathf::RandF(-0.15f, 0.15f);
        camera_look.z += Mathf::RandF(-0.15f, 0.15f);
        camera_look = camera_look.normalized();
        arr_player[p_id].bullet[j].isInUse = true;
        Matrix4x4 collide_position = Matrix4x4::identity; // identity: 4x4 danwi
        for (int i = 0; i < MAX_NUM_OBJECT; ++i) // monster check
        {
            if (npc_[i].hp <= 0)
                continue;
            isIntersected = CollideObjectByRayIntersection(npc_[i].OOBB, camera_pos, camera_look, &hit_distance);
            if (isIntersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = NPCType;
                object_id = i;
            }
        }

        for (int i = 0; i < MAX_NUM_INTERACTION; ++i) // monster check
        {
            if (interactions_[i].objectName == DOOR && interactions_[i].interactEnable == true)
                continue;
            isIntersected = CollideObjectByRayIntersection(interactions_[i].OOBB, camera_pos, camera_look,
                                                           &hit_distance);
            if (isIntersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = StaticType;
                object_id = i;
            }
        }

        for (int i = 0; i < structures_.size(); ++i)
        {
            if (structures_[i].type == fence)
                continue;
            isIntersected = CollideObjectByRayIntersection(structures_[i].OOBB, camera_pos, camera_look, &hit_distance);
            if (isIntersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = StaticType;
                object_id = i;
            }
        }

        ////////////////////////////////////////
        if (NoneType != object_type)
        {
            const float distance = static_cast<float>(pow(nearest_hit_distance, 2));
            const float sum_look_pos = static_cast<float>(pow(camera_look.x, 2)) + static_cast<float>(
                pow(camera_look.y, 2)) + static_cast<float>(pow(camera_look.z, 2));
            const float final = distance / sum_look_pos;
            // return collision CurPos (burn effects)
            collide_position._41 = camera_pos.x + camera_look.x * (sqrt(final) - 2);
            collide_position._42 = camera_pos.y + camera_look.y * (sqrt(final) - 2);
            collide_position._43 = camera_pos.z + camera_look.z * (sqrt(final) - 2);

            arr_player[p_id].bullet[j].position.x = collide_position._41;
            arr_player[p_id].bullet[j].position.y = collide_position._42;
            arr_player[p_id].bullet[j].position.z = collide_position._43;
        }

        switch (object_type)
        {
        case NoneType:
            arr_player[p_id].bullet[j].isInUse = false;
            break;
        case NPCType:
            if (npc_[object_id].state != Dead)
            {
                arr_player[p_id].bullet[j].type = NPCType;
                for (int p : zones_.at(npc_[object_id].zoneNum).monsterID)
                {
                    if (npc_[p].state != NoneState)
                        continue;
                    npc_[p].destPl = p_id;
                    npc_[p].state = Hit;
                }
                if (npc_[object_id].destPl != p_id)
                {
                    if (Vector3::Distance(npc_[object_id].CurPos, arr_player[p_id].CurPos) < Vector3::Distance(
                        npc_[object_id].CurPos, arr_player[npc_[object_id].destPl].CurPos))
                    {
                        npc_[object_id].destPl = p_id;
                        npc_[object_id].state = Hit;
                    }
                }

                if (0 < npc_[object_id].hp)
                {
                    //몬스터 즉사 아이템 처리
                    if (arr_player[p_id].status.instantDeath > npc_[object_id].hp && npc_[object_id].mob != GOLEM)
                        npc_[object_id].hp = 0;
                    else
                    {
                        float damage = arr_player[p_id].status.attackDamage;
                        //보스 데미지 증가 아이템처리
                        if (npc_[object_id].mob == GOLEM)
                            damage = damage + damage * arr_player[p_id].status.bossDamage * 0.01;
                        npc_[object_id].hp -= damage;
                        npc_[object_id].hp = Mathf::Max(npc_[object_id].hp, 0);
                    }

                    if (npc_[object_id].hp == 0)
                    {
                        arr_player[p_id].status.hp = Mathf::Min(
                            arr_player[p_id].status.hp + arr_player[p_id].status.killMaxHp,
                            arr_player[p_id].status.maxhp);
                        npc_[object_id].state = Dead;
                        npc_[object_id].time_death = chrono::system_clock::now();
                    }
                }
                else
                {
                    npc_[object_id].state = Dead;
                    npc_[object_id].time_death = chrono::system_clock::now();
                }
            }
            break;
        case StaticType:
            arr_player[p_id].bullet[j].type = StaticType;
        //
            break;
        case PlayerType:
            //
            break;
        default: ;
        }
    }
}

void GameMgr::CheckinteractionObject(int p_id)
{
}

void GameMgr::SetHP(const int id, const int hp)
{
    arr_player[id].status.hp = hp;
}


void GameMgr::SetItem(const int id, const ITEM_TYPE item)
{
    arr_player[id].currentItem = item;
    switch (item)
    {
    case ItemDamageUpMaxHpDown:
        //데미지 증가 최대체력 감소 아이템처리
        arr_player[id].status.maxhp /= 2;
        arr_player[id].status.hp = Mathf::Min(arr_player[id].status.maxhp, arr_player[id].status.hp);
        arr_player[id].status.attackDamage *= 2;
        break;
    case ItemBlockDamage:
        arr_player[id].status.block += 10;
        break;
    case ItemBossDamageUp:
        arr_player[id].status.bossDamage += 20;
        break;
    case ItemKillMaxHpUp:
        arr_player[id].status.killMaxHp += 1;
        break;
    case ItemInstantDeath:
        if (arr_player[id].status.instantDeath == 0)
            arr_player[id].status.instantDeath += 10;
        else
            arr_player[id].status.instantDeath += 5;
        break;
    case ItemMonsterSlow:
        arr_player[id].activeItem = item;
    //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ItemPlayerSpeedUp:
        //플레이어 스피드업 아이템처리
        arr_player[id].status.speed *= 1.2;
        break;
    case ItemMaxHpUp:
        //최대 체력 증가 아이템 처리
        arr_player[id].activeItem = item;
    //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ItemAttackSpeedUp:
        arr_player[id].status.attackSpeed /= 1.2f;
        break;
    default:
        break;
    }
}

Vector3 GameMgr::GetPositionToHeightMap(const float x, const float z, const float addy) const
{
    Vector3 pos;
    pos.x = x;
    pos.z = z;
    pos.y = mapData->GetHeight(x, z) * mapData->GetScale().y + addy;
    return pos;
}


sc_ingame_packet GameMgr::GetPacket(sc_ingame_packet packet) const
{
    sc_ingame_packet sc_packet;
    if (isEnding)
    {
        sc_packet.type = SC_GAME_TO_ENDING_PACKET;
        chrono::duration<int> t = chrono::duration_cast<chrono::seconds>(endTime - startTime);
        sc_packet.playTime = t.count();
    }
    else
        sc_packet.type = SC_INGAME_PACKET;
    sc_packet.size = sizeof(sc_ingame_packet);
    sc_packet.playerId = 0;


    for (int i = 0; i < MAX_NUM_PLAYER; ++i)
    {
        const int id = playerIds[i];

        sc_packet.players[i].position = arr_player[id].CurPos;
        sc_packet.players[i].lookDirection = arr_player[id].pl_look;
        sc_packet.players[i].cameraLook = arr_player[id].cam_look;
        sc_packet.players[i].state = arr_player[id].state;
        sc_packet.players[i].id = i;
        sc_packet.players[i].zoneNumber = currentZoneLevel_;
        sc_packet.players[i].status.healthPoints = arr_player[id].status.hp;
        sc_packet.players[i].status.maxHealthPoints = arr_player[id].status.maxhp;
        sc_packet.players[i].status.attackSpeed = arr_player[id].status.attackSpeed;
        sc_packet.players[i].isReloading = arr_player[id].reloadEnable;
        sc_packet.players[i].ammoCount = arr_player[id].status.ammo;
        memset(sc_packet.players[i].bullet, NULL, sizeof(Bullet) * 5);
        memcpy(sc_packet.players[i].bullet, arr_player[id].bullet, sizeof(Bullet) * 5);
        sc_packet.players[i].currentItem = arr_player[id].currentItem;
    }

    for (int i = 0; i < MAX_NUM_OBJECT; ++i) // 오류 발생
    {
        sc_packet.npcs[i].id = i;
        sc_packet.npcs[i].position = npc_[i].CurPos;
        sc_packet.npcs[i].lookDirection = npc_[i].Lookvec;
        sc_packet.npcs[i].healthPoints = npc_[i].hp;
        sc_packet.npcs[i].isAttack = npc_[i].attackPacketEnable;
        sc_packet.npcs[i].state = npc_[i].state;
    }

    for (int i = 0; i < MAX_NUM_INTERACTION; ++i)
    {
        sc_packet.interactions[i].position = interactions_[i].Pos;
        sc_packet.interactions[i].lookDirection = interactions_[i].Lookvec;
        if (interactions_[i].item.size() != 0)
        {
            sc_packet.interactions[i].items[0].itemType = interactions_[i].item.at(0).item;
            sc_packet.interactions[i].items[0].isAlive = !interactions_[i].item.at(0).getEnable;
            sc_packet.interactions[i].items[1].itemType = interactions_[i].item.at(1).item;
            sc_packet.interactions[i].items[1].isAlive = !interactions_[i].item.at(1).getEnable;
            sc_packet.interactions[i].items[2].itemType = interactions_[i].item.at(2).item;
            sc_packet.interactions[i].items[2].isAlive = !interactions_[i].item.at(2).getEnable;
            sc_packet.interactions[i].items[3].itemType = interactions_[i].item.at(3).item;
            sc_packet.interactions[i].items[3].isAlive = !interactions_[i].item.at(3).getEnable;
        }
        sc_packet.interactions[i].isInteracting = interactions_[i].interactEnable;
        sc_packet.interactions[i].state = interactions_[i].state;
    }

    for (int i = 0; i < stoneAttacks_.size(); ++i)
    {
        sc_packet.stones[i].isActive = stoneAttacks_[i].activeEnable;
        sc_packet.stones[i].position = stoneAttacks_[i].pos;
    }

    for (int i = 0; i < 20; ++i)
    {
        sc_packet.attacks[i].isActive = rangeAttacks_[i].activeEnable;
        sc_packet.attacks[i].position = rangeAttacks_[i].pos;
    }

    return sc_packet;
}

XMFLOAT3 GameMgr::GetPlayerPosition(const int id)
{
    if (id < 0 || id >= MAX_NUM_PLAYER)
        return {0, 0, 0};
    return arr_player[id].CurPos;
}

XMFLOAT3 GameMgr::GetPlayerLook(const int id)
{
    if (id < 0 || id >= MAX_NUM_PLAYER)
        return {0, 0, 0};
    return arr_player[id].pl_look;
}

XMFLOAT3 GameMgr::GetPlayerCameraLook(const int id)
{
    if (id < 0 || id >= MAX_NUM_PLAYER)
        return {0, 0, 0};
    return arr_player[id].cam_look;
}

float GameMgr::GetPlayerHP(const int id)
{
    if (id < 0 || id >= MAX_NUM_PLAYER)
        return 0;
    return arr_player[id].status.hp;
}

XMFLOAT3 GameMgr::GetNPCPosition(const int id)
{
    if (id < 0 || id >= MAX_NUM_OBJECT)
        return {0, 0, 0};
    return npc_[id].CurPos;
}
