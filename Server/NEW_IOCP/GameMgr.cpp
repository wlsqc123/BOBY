#include "GameMgr.h"
#include <random>
#include <tchar.h>

GameMgr::GameMgr(): zone_level_(0), r_id(0), pl_list{}
{
    map_data = new CHeightMapImage(_T("mapImage/map1.raw"), 513, 513, Vector3(10.0f, 25.0f, 10.0f));
}

GameMgr::~GameMgr()
{
}

void GameMgr::init_game(int id[4])
{
    float pos_x = 0.0f;
    float pos_z = 0.0f;
    
    game_zones_.reserve(5);
    zone_level_ = 0;
    stone_time_ = chrono::system_clock::now();
    r_id = arr_player[id[0]].r_id;

    //PLAYER
    {
        float posY = 0.0f;
        for (int i = 0; i < max_player; ++i) {
            pl_list[i] = id[i];
        }

        for (int p_id : pl_list)
        {
            arr_player[p_id].id = p_id;

            pos_x = static_cast<float>(500 + (p_id * ((2300 - 1700) / max_player)));
            pos_z = 4680.0f;

            //posX = float(557.519 + (p_id * ((2300 - 1700) / MAX_PLAYER)));
            //posZ = 1202.554;

            posY = map_data->GetHeight(pos_x, pos_z) * map_data->GetScale().y + 100.f;

            Vector3 pos{ pos_x, posY,  pos_z };
            arr_player[p_id].init_pos = pos;
            arr_player[p_id].cur_pos = pos;
            arr_player[p_id].prev_pos = pos;
            arr_player[p_id].ps.hp = arr_player[p_id].ps.max_hp = 100;
            //라이플
            switch (arr_player[p_id].wp_type)
            {
            case WEAPON_RIFLE:
                arr_player[p_id].ps.attack_speed = 0.15f;
                arr_player[p_id].ps.max_ammo = 30;
                arr_player[p_id].ps.ammo = 30;
                arr_player[p_id].ps.attack_damage = 7.f;
                break;
            case WEAPON_SHOTGUN:
                arr_player[p_id].ps.attack_speed = 0.5f;
                arr_player[p_id].ps.max_ammo = 7;
                arr_player[p_id].ps.ammo = 7;
                arr_player[p_id].ps.attack_damage = 5.f;

                break;
            case WEAPON_SNIPER:
                arr_player[p_id].ps.attack_speed = 1.0f;
                arr_player[p_id].ps.max_ammo = 5;
                arr_player[p_id].ps.ammo = 5;
                arr_player[p_id].ps.attack_damage = 40.f;

                break;
            default:
                cout << "type error" << endl;
                break;
            }
            arr_player[p_id].state = none;

            Vector3 look_vector;
            look_vector = {pos_x - 2050.f, 0, pos_z - 1500.f};
            look_vector = look_vector.normalized();
            arr_player[p_id].pl_look = look_vector;
            arr_player[p_id].OOBB = BoundingOrientedBox(arr_player[p_id].cur_pos, XMFLOAT3(obb_scale_player_x, obb_scale_player_y, obb_scale_player_z), XMFLOAT4(0, 0, 0, 1));
            arr_player[p_id].cam_look = { 1,0,0 };


            for (int j = 0; j < 9; ++j)
            {
                auto item = static_cast<ITEM_TYPE>(j);
                arr_player[p_id].pl_items.insert({ item, 0 });
            }
        }
    }

    //NPC
    {
        npc_.reserve(max_object);

        NPC new_npc;
        pos_x = static_cast<float>(rand() % 2000 + 250);
        pos_z = static_cast<float>(rand() % 2000 + 250);
        new_npc.cur_pos = { pos_x, map_data->GetHeight(pos_x, pos_z) * map_data->GetScale().y + 80, pos_z };
        new_npc.init_pos = new_npc.cur_pos;
        new_npc.speed = 8;
        new_npc.state = none;


        for (int i = 0; i < magmamonster_num; ++i) {
            new_npc.mob = magma;
            new_npc.OOBB.Extents = { obb_scale_magmaa_x, obb_scale_magmaa_y, obb_scale_magmaa_z };
            new_npc.hp = 200;
            new_npc.attack_range = 1300;
            new_npc.sight = 1500;
            new_npc.cool_time = 1000;
            npc_.emplace_back(new_npc);
        }
        for (int i = 0; i < golemmonster_num; ++i) {
            new_npc.mob = golem;
            new_npc.OOBB.Extents = { obb_scale_golem_x, obb_scale_golem_y, obb_scale_golem_z };
            new_npc.hp = 3500;
            new_npc.attack_range = 300;
            new_npc.sight = 350;
            new_npc.speed = 10;
            new_npc.cool_time = 1800;
            npc_.emplace_back(new_npc);
        }
        for (int i = 0; i < orgemonster_num; ++i) {
            new_npc.mob = ogre;
            new_npc.OOBB.Extents = { obb_scale_orge_x, obb_scale_orge_y, obb_scale_orge_z };
            new_npc.hp = 250;
            new_npc.attack_range = 150;
            new_npc.speed = 8;
            new_npc.sight = 500;
            new_npc.cool_time = 1000;
            npc_.emplace_back(new_npc);
        }

        for (int i = 0; i < max_object; ++i) {
            int rand = i % 4;
            switch (rand)
            {
            case 0:
                npc_[i].look_vec = npc_[i].cur_pos;
                npc_[i].look_vec.x += 1;
                npc_[i].look_vec = npc_[i].look_vec.normalized();
                break;
            case 1:
                npc_[i].look_vec = npc_[i].cur_pos;
                npc_[i].look_vec.x -= 1;
                npc_[i].look_vec = npc_[i].look_vec.normalized();

                break;
            case 2:
                npc_[i].look_vec = npc_[i].cur_pos;
                npc_[i].look_vec.z += 1;
                npc_[i].look_vec = npc_[i].look_vec.normalized();
                break;
            case 3:
                npc_[i].look_vec = npc_[i].cur_pos;
                npc_[i].look_vec.z -= 1;
                npc_[i].look_vec = npc_[i].look_vec.normalized();
                break;
            default:
                break;
            }
        }

        Vector3 mob_look;

        //중앙 방 2층 2마리
        npc_[0].cur_pos.x = 1706.f; npc_[0].cur_pos.z = 2045.f; npc_[0].zone_num = 1;
        npc_[1].cur_pos.x = 1705.f; npc_[1].cur_pos.z = 3695.f; npc_[1].zone_num = 1;

        //중앙방 출구후 방 1마리
        npc_[2].cur_pos.x = 4790.f; npc_[2].cur_pos.z = 1205.f; npc_[2].zone_num = 2;
        mob_look = { 1545.f, 0.f, 3296.f };

        for (int i = 0; i < 3; ++i)
        {
            npc_[i].look_vec = mob_look - npc_[i].cur_pos;
            npc_[i].look_vec.y = 0.f;
            npc_[i].look_vec = npc_[i].look_vec.normalized();
        }
        //시작방 앞 2마리
        npc_[3].cur_pos.x = 4675.15f; npc_[3].cur_pos.z = 4515.6f; npc_[3].zone_num = 0;
        npc_[4].cur_pos.x = 4724.52f; npc_[4].cur_pos.z = 4877.16f; npc_[4].zone_num = 0;
        //중앙방 3마리
        npc_[5].cur_pos.x = 4662.5f;  npc_[5].cur_pos.z = 2706.39f; npc_[5].zone_num = 1;
        npc_[6].cur_pos.x = 2006.f;  npc_[6].cur_pos.z = 2045.f; npc_[6].zone_num = 1;
        npc_[7].cur_pos.x = 1742.81; npc_[7].cur_pos.z = 2972.67f; npc_[7].zone_num = 1;     //중앙 2층 한가운데

        //보스전방
        npc_[8].cur_pos.x = 384;  npc_[8].cur_pos.z = 1209; npc_[8].zone_num = 3;
        mob_look = { 800.f, 0.f, 2671.f };

        for (int i = 3; i < 9; ++i)
        {
            npc_[i].look_vec = mob_look - npc_[i].cur_pos;
            npc_[i].look_vec.y = 0.f;
            npc_[i].look_vec = npc_[i].look_vec.normalized();
        }

        //보스
        npc_[9].cur_pos.x = 4760.f; npc_[9].cur_pos.z = 575.f; npc_[9].zone_num = 4;

        mob_look = { 1545.f, 0.f, 3296.f };

        npc_[9].look_vec = mob_look - npc_[9].cur_pos;
        npc_[9].look_vec.y = 0.f;
        npc_[9].look_vec = npc_[9].look_vec.normalized();

        npc_[9].prev_pos = npc_[9].cur_pos;

        //시작방 앞 2마리
        npc_[10].cur_pos.x = 3698.f; npc_[10].cur_pos.z = 4907.26f;   npc_[10].zone_num = 0;
        npc_[11].cur_pos.x = 3725.89f; npc_[11].cur_pos.z = 4545.97f;   npc_[11].zone_num = 0;

        //중앙 방배치몹
        npc_[12].cur_pos.x = 3298.f; npc_[12].cur_pos.z = 2752.f;   npc_[12].zone_num = 1;    //중앙방 맵 한가운데 왼쪽몹

        mob_look = { 2029.f, 0.f, 1500.f };

        for (int i = 9; i < 13; ++i)
        {
            npc_[i].look_vec = mob_look - npc_[i].cur_pos;
            npc_[i].look_vec.y = 0.f;
            npc_[i].look_vec = npc_[i].look_vec.normalized();
        }


        npc_[13].cur_pos.x = 3255.0f;  npc_[13].cur_pos.z = 3703.09f; npc_[13].zone_num = 1;       //중앙방 한가운데 오른쪽몹
        npc_[14].cur_pos.x = 4610.76f; npc_[14].cur_pos.z = 2142.85f; npc_[14].zone_num = 1;      //중앙방 작은계단위

        //중앙방 출구 후 2마리
        npc_[15].cur_pos.x = 4668.f; npc_[15].cur_pos.z = 1640.f; npc_[15].zone_num = 2;          //출구 바라보고 왼쪽
        npc_[16].cur_pos.x = 4240.f; npc_[16].cur_pos.z = 1139.f; npc_[16].zone_num = 2;        //출구 바라보고 오른쪽

        //이상한놈들
        npc_[17].cur_pos.x = 10036.f; npc_[17].cur_pos.z = 3532.05f; npc_[17].zone_num = 4;

        mob_look = { 3256.f, 0.f, 2662.f };

        for (int i = 13; i < 18; ++i)
        {
            npc_[i].look_vec = mob_look - npc_[i].cur_pos;
            npc_[i].look_vec.y = 0.f;
            npc_[i].look_vec = npc_[i].look_vec.normalized();
        }
        for (auto& p : npc_)
        {
            p.prev_pos = p.cur_pos = get_position_to_height_map(p.cur_pos.x, p.cur_pos.z, 80);
            Matrix4x4 temp = Matrix4x4::identity;
            temp = temp.Translate(p.cur_pos);
            p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&temp));
        }
        //

    }

    ///static object
    {
        interaction_.reserve(max_intraction);

        std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<int> dis(0, 8);

        for (int i = 0; i < chestobject_num; ++i) {
            interaction new_object;
            new_object.object_name = chest;
            new_object.OOBB.Extents = { obb_scale_chest_x, obb_scale_chest_y, obb_scale_chest_z };
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
            new_object.zone_num = 99;
            interaction_.emplace_back(new_object);
        }
        for (int i = 0; i < doorobject_num; ++i) {
            interaction new_object;
            new_object.object_name = door;
            new_object.OOBB.Extents = { obb_scale_door_x, obb_scale_door_y, obb_scale_door_z };
            new_object.zone_num = 99;
            interaction_.emplace_back(new_object);
        }
        for (int i = 0; i < leverobject_num; ++i) {
            interaction new_object;
            new_object.object_name = lever;
            new_object.OOBB.Extents = { obb_scale_lever_x, obb_scale_lever_y, obb_scale_lever_z };
            new_object.zone_num = 99;
            interaction_.emplace_back(new_object);
        }

        interaction new_object;
        new_object.object_name = mud;
        new_object.OOBB.Extents = { 0, 0, 0 };
        interaction_.emplace_back(new_object);

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
        
        interaction_[0].pos = get_position_to_height_map(4915.81, 4840, 40);      //시작방
        interaction_[1].pos = get_position_to_height_map(660.232, 4105.35, 40);      //마그마 미로방
        interaction_[2].pos = get_position_to_height_map(860, 275, 40);              //보스방앞
        interaction_[3].pos = get_position_to_height_map(3555, 2000, 40);            //중앙 방 내부 상자
        interaction_[4].pos = get_position_to_height_map(1350, 1208, 40);            //중앙 방 출구후 뒤 상자
        interaction_[5].pos = get_position_to_height_map(1350, 4120, 40);
        
        //문                 
        interaction_[6].pos = get_position_to_height_map(1165, 4700, 75);        //시작방
        interaction_[7].pos = get_position_to_height_map(1170, 1450, 75);       //마그마방 직전
        interaction_[7].zone_num = 2;
        interaction_[8].pos = get_position_to_height_map(1165, 450, 75);      //보스방 입구
        interaction_[9].pos = get_position_to_height_map(3935, 1815, 90);           //중앙방 출구
        interaction_[9].zone_num = 1;
        interaction_[10].pos = get_position_to_height_map(710, 1060, 70);                   //보스방 전전방
        interaction_[10].zone_num = 3;
        interaction_[11].pos = get_position_to_height_map(4700, 4242, 60);                   //중앙방 입구
        interaction_[11].zone_num = 0;
        
        //레버
        //interaction[11].Pos = Vector3(1630, 540, 2680);

        interaction_[12].pos = Vector3(360, 1475, 4250);

        Matrix4x4 world;
        Quaternion qua = Quaternion::AngleAxis(90, Vector3(0, 1, 0));
        world = Matrix4x4::Rotate(qua);
        interaction_[4].OOBB.Transform(interaction_[4].OOBB, XMLoadFloat4x4(&world));
        interaction_[6].OOBB.Transform(interaction_[6].OOBB, XMLoadFloat4x4(&world));
        interaction_[7].OOBB.Transform(interaction_[7].OOBB, XMLoadFloat4x4(&world));
        interaction_[8].OOBB.Transform(interaction_[8].OOBB, XMLoadFloat4x4(&world));
        for (auto& p : interaction_)
        {
            world = Matrix4x4::identity;
            world._41 = p.pos.x, world._42 = p.pos.y, world._43 = p.pos.z;
            p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
            p.lookvec = Vector3(-1, 0, 0);
            p.state = none;
            p.interact_enable = false;
        }
        interaction_[4].lookvec = Vector3(0, 0, 1);
    }

    range_attack_.reserve(20);

    for (int i = 0; i < 20; ++i)
    {
        range_attack ra;
        ra.pos = Vector3(0, 0, 0);

        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(20, 20, 20);
        ra.active_enable = false;

        range_attack_.push_back(ra);
    }

    stone_attack_.reserve(10);

    for (int i = 0; i < 5; ++i)
    {
        range_attack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(50, 37, 75);
        ra.active_enable = false;
        stone_attack_.push_back(ra);
    }
    for (int i = 0; i < 5; ++i)
    {
        range_attack ra;
        ra.pos = Vector3(0, 0, 0);
        ra.speed = 10;
        ra.OOBB.Center = ra.pos;
        ra.OOBB.Extents = Vector3(30, 22.5, 37);
        ra.active_enable = false;
        stone_attack_.push_back(ra);
    }
    // WALL
    {
        int x_objects = 20;
        int y_objects = 3;
        int z_objects = 7;

        float wall_size = 250;
        float x_pos, y_pos, z_pos;
        float height = 1000;

        for (int x = 0; x < x_objects; x++)
        {
            for (int z = 0; z < z_objects; z++)
            {
                for (int y = 0; y < y_objects; y++)
                {
                    structure st;
                    st.type = wall;

                    if (z == 0)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = z * wall_size * 3 + 100;
                        y_pos = height + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);

                        x_pos = z * wall_size * 3 + 100;
                        z_pos = x * wall_size + 200;
                        y_pos = height + wall_size * (0.5f + y);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);
                    }

                    if (z == z_objects - 1)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 5045.f;;
                        y_pos = height + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);


                        x_pos = 5045.f;
                        z_pos = x * wall_size + 200;
                        y_pos = height + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);
                    }

                    if ((x != 1 && x != 5 && x != x_objects - 2) || y != 0)
                    {
                        x_pos = 1170;
                        z_pos = x * wall_size + 200;
                        y_pos = height + 55 + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 20, wall_size / 2, wall_size / 2));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);
                    }

                    if ((x != 15 || y != 0))
                    {
                        if (x != 1 && x != 2 || y == 2)
                        {
                            x_pos = x * wall_size + 200;
                            z_pos = 1815;
                            y_pos = height - 50 + wall_size * (y + 0.5f);
                            st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                            st.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                            st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                            structure_.push_back(st);
                        }
                    }

                    if ((x != x_objects - 2 || y == 2))
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 4245.f;
                        y_pos = height + 40 + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);
                    }
                    if (x != 2 || y != 0)
                    {
                        x_pos = x * wall_size + 200;
                        z_pos = 1060;
                        y_pos = height + 50 + wall_size * (y + 0.5f);
                        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
                        st.extend = (XMFLOAT3(wall_size / 2, wall_size / 2, wall_size / 20));
                        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
                        structure_.push_back(st);
                    }
                }
            }
        }
    }

    //중앙방 큰 2층 난간
    {
        //2층위 난간
        float x_pos, y_pos, z_pos;
        // float height = 1000;
        structure st;

        st.type = fence;
        x_pos = 2140;
        y_pos = 1050;
        z_pos = 3255;
        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
        st.extend = (XMFLOAT3(80, 400, 990));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);

        //계단 난간

        x_pos = 2410;
        y_pos = 1050;
        z_pos = 2300;
        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
        st.extend = (XMFLOAT3(270, 300, 40));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);

    }

    //중앙방 작은 층 난간
    {
        float x_pos, y_pos, z_pos;
        // float height = 1000;
        structure st;

        st.type = fence;

        x_pos = 4255.58;
        y_pos = 1050;
        z_pos = 2805;
        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
        st.extend = (XMFLOAT3(40, 110, 1020));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);
    }

    //시작방 난간
    {
        //입구에서 출구바라보고 계단왼쪽난간
        float x_pos, y_pos, z_pos;
        // float height = 1000;
        structure st;

        st.type = fence;
        x_pos = 4276.01;
        y_pos = 1050;
        z_pos = 4900.77;
        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
        st.extend = (XMFLOAT3(40, 100, 120));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);

        //입구에서 출구바라보고 계단오른쪽난간
        x_pos = 4276.01;
        y_pos = 1050;
        z_pos = 4365;
        st.center = (XMFLOAT3(x_pos, y_pos, z_pos));
        st.extend = (XMFLOAT3(40, 100, 120));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);
    }

    //중앙방 출구 후 방 계단난간
    {
        structure st;

        st.type = fence;

        st.center = (XMFLOAT3(3550, 1050, 1564));
        st.extend = (XMFLOAT3(40, 100, 200));
        st.OOBB = BoundingOrientedBox(st.center, st.extend, XMFLOAT4(0, 0, 0, 1));
        structure_.push_back(st);
    }

    BoundingOrientedBox lava;
    lava.Center = { 665.265, 1260, 3328.36 };
    lava.Extents = { 800, 1.f, 800 };
    map_lava_.emplace_back(lava);

    zone zone;
    zone.is_clear = false;
    //시작방
    zone.monster_id = { 3,4,10,11 };
    game_zones_.push_back(zone);
    //중앙방
    zone.monster_id = { 0,1,5,6,7,12,13,14 };
    game_zones_.push_back(zone);
    zone.monster_id = { 2, 15, 16 };
    game_zones_.push_back(zone);
    //안개방?
    zone.monster_id = { 8 };
    game_zones_.push_back(zone);
    //보스방
    zone.monster_id = { 9 };
    game_zones_.push_back(zone);

    is_running = true;
}



void GameMgr::update()
{
    for (int n_id = 0; n_id < max_object; ++n_id) {
        switch (npc_[n_id].state)
        {
        case none:
            for (auto& p : arr_player)
            {
                if (find(game_zones_.at(zone_level_).monster_id.begin(), game_zones_.at(zone_level_).monster_id.end(), n_id) == game_zones_.at(zone_level_).monster_id.end()) continue;
                if (p.ps.hp <= 0) continue;
                if (Vector3::Distance(npc_[n_id].cur_pos, p.cur_pos) < npc_[n_id].sight)
                {
                    npc_[n_id].dest_pl = p.id;
                    npc_[n_id].state = hit;
                }
            }
            break;
        case hit:
            trace_player(n_id);
            break;

        case attack:
            attack_player(n_id);
            break;

        case dead:
        {
            if (chrono::system_clock::now() - npc_[n_id].time_death > chrono::milliseconds(2000)) {
                npc_[n_id].cur_pos.x = -1000.f;
            npc_[n_id].cur_pos.z = -1000.f;
            npc_[n_id].prev_pos.x = -1000.f;
            npc_[n_id].prev_pos.z = -1000.f;
            }
            break;
        }
        default:
            break;
        }

        //gravity
        if (npc_[n_id].cur_pos.y > map_data->GetHeight(npc_[n_id].cur_pos.x, npc_[n_id].cur_pos.z)* map_data->GetScale().y + 80)
            npc_[n_id].cur_pos.y -= 5;
        else
            npc_[n_id].cur_pos.y = map_data->GetHeight(npc_[n_id].cur_pos.x, npc_[n_id].cur_pos.z) * map_data->GetScale().y + 80;


        if (npc_[n_id].cur_pos != npc_[n_id].prev_pos) {
            Vector3 look_vector = npc_[n_id].cur_pos - npc_[n_id].prev_pos;
            look_vector.y = 0.f;
            npc_[n_id].look_vec = look_vector.normalized();
        }

        XMFLOAT4X4 unit
        (
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            npc_[n_id].cur_pos.x, npc_[n_id].cur_pos.y, npc_[n_id].cur_pos.z, 1
        );


        npc_[n_id].OOBB.Transform(npc_[n_id].OOBB, DirectX::XMLoadFloat4x4(&unit));

        switch (npc_[n_id].mob)
        {
        case magma:
            npc_[n_id].OOBB.Extents = { obb_scale_magmaa_x, obb_scale_magmaa_y, obb_scale_magmaa_z };
            break;
        case golem:
            npc_[n_id].OOBB.Extents = { obb_scale_golem_x, obb_scale_golem_y, obb_scale_golem_z };
            break;
        case ogre:
            npc_[n_id].OOBB.Extents = { obb_scale_orge_x, obb_scale_orge_y, obb_scale_orge_z };
            break;
        }

        // init look_vec
        if (npc_[n_id].look_vec.x == 0.f && npc_[n_id].look_vec.y == 0.f && npc_[n_id].look_vec.z == 0.f)
            npc_[n_id].look_vec = { 1, 0, 0 };


        npc_coll_check(n_id);

        npc_[n_id].prev_pos = npc_[n_id].cur_pos;
    }

    for (auto& ra : range_attack_)
    {
        if (!ra.active_enable) continue;
        ra.pos = ra.pos + ra.look * ra.speed;
        ra.OOBB.Center = ra.pos;
        if (chrono::system_clock::now() - ra.live_time > chrono::seconds(3))
        {
            ra.active_enable = false;
            continue;
        }
        for (auto& p : arr_player)
        {
            if (ra.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].ps.block < Mathf::RandF(0, 100))
                    arr_player[p.id].ps.hp = Mathf::Max(arr_player[p.id].ps.hp - 10, 0);
                ra.active_enable = false;

                cout << "range Attack" << endl;
                break;
            }
        }
        for (auto& p : structure_)
        {
            if (ra.OOBB.Intersects(p.OOBB))
            {
                if (p.type == wall)
                {
                    ra.active_enable = false;
                    break;
                }
            }
        }
           
    }
    if (zone_level_ == 4 && chrono::system_clock::now() - stone_time_ > chrono::milliseconds(500))
    {
        for (auto& st : stone_attack_)
        {
            if (!st.active_enable)
            {
                st.pos = Vector3(Mathf::RandF(1250.f, 4800.f), 1800, Mathf::RandF(200.f, 900.f));
                st.OOBB.Center = st.pos;
                st.active_enable = true;
                st.live_time = chrono::system_clock::now();
                break;
            }
        }
        stone_time_ = chrono::system_clock::now();
    }

    for (auto& st : stone_attack_)
    {
        if (!st.active_enable) continue;
        st.pos.y -= st.speed;
        st.OOBB.Center = st.pos;
        if (chrono::system_clock::now() - st.live_time > chrono::seconds(4))
        {
            st.active_enable = false;
            continue;
        }
        for (auto& p : arr_player)
        {
            if (st.OOBB.Intersects(p.OOBB))
            {
                if (arr_player[p.id].ps.block < Mathf::RandF(0, 100))
                    arr_player[p.id].ps.hp = Mathf::Max(arr_player[p.id].ps.hp - 20, 0);
                st.active_enable = false;
                break;
            }
        }
    }

    if (is_slow_ && chrono::system_clock::now() - slow_time_ > chrono::seconds(20))
        is_slow_ = false;

    if (npc_[9].state == dead && chrono::system_clock::now() - npc_[9].time_death > chrono::seconds(4))
    {
        if (is_ending == false) {
            is_ending = true;
            e_time = chrono::system_clock::now();
        }
    }

    cur_update_time = chrono::system_clock::now();
}


void GameMgr::trace_player(const int n_id)
{
    const int p_id = npc_[n_id].dest_pl;
    if (arr_player[p_id].ps.hp <= 0)
    {
        npc_[n_id].state = none;
        return;
    }
    Vector3 player_pos = arr_player[p_id].cur_pos;
    player_pos.y -= 20;
    const float distance = Vector3::Distance(npc_[n_id].cur_pos, player_pos);
    float speed = npc_[n_id].speed;
    if (is_slow_) speed *= 0.5;

    if (npc_[n_id].attack_range < distance)    
        npc_[n_id].cur_pos = Vector3::MoveTowards(npc_[n_id].cur_pos, player_pos, speed);
    else
    {
        npc_[n_id].state = attack;
        npc_[n_id].is_attack = true;
        npc_[n_id].time_last_attack = chrono::system_clock::now();
    }
}


void GameMgr::attack_player(const int n_id)
{
    const int p_id = npc_[n_id].dest_pl;

    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack > chrono::milliseconds(npc_[n_id].cool_time) && npc_[n_id].is_attack)
    {
        if (npc_[n_id].mob == ogre)
        {
            if (arr_player[p_id].ps.block < Mathf::RandF(0, 100))
            {
                arr_player[p_id].m_slock.lock();
                arr_player[p_id].ps.hp = Mathf::Max(arr_player[p_id].ps.hp - 10, 0);
                arr_player[p_id].m_slock.unlock(); 
            }
        }
        else if(npc_[n_id].mob == magma)
        {
            for (auto& p : range_attack_)
            {
                Matrix4x4 world = Matrix4x4::identity;
                if (!p.active_enable)
                {
                    p.pos = npc_[n_id].cur_pos + npc_[n_id].look_vec * 30;
                    world._41 = p.pos.x; world._42 = p.pos.y; world._43 = p.pos.z;
                    p.OOBB.Transform(p.OOBB, XMLoadFloat4x4(&world));
                    p.speed = 25;
                    p.look =  (arr_player[p_id].cur_pos- npc_[n_id].cur_pos).normalized();
                    p.active_enable = true;
                    p.live_time = chrono::system_clock::now();
                    break;
                }
            }
        }
        else
        {      
            for (auto& p : arr_player)
            {
                if (npc_[n_id].cur_pos.x - 400 < p.cur_pos.x && npc_[n_id].cur_pos.x + 400 > p.cur_pos.x
                    && npc_[n_id].cur_pos.z - 400 < p.cur_pos.z && npc_[n_id].cur_pos.z + 400 > p.cur_pos.z)
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
        npc_[n_id].attack_packet_enable = true;
        npc_[n_id].is_attack = false;
    } 
    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack > chrono::milliseconds(npc_[n_id].cool_time + 200))
    {
        npc_[n_id].attack_packet_enable = false;
    }
    if (0 < arr_player[p_id].ps.hp && chrono::system_clock::now() - npc_[n_id].time_last_attack > chrono::milliseconds(2500))
    {
        npc_[n_id].state = hit;
    }
    if (arr_player[p_id].ps.hp <= 0)
    {
        npc_[n_id].state = none;
        npc_[n_id].attack_packet_enable = false;
    }
}

void GameMgr::check_player_dead(const int p_id)
{
    if (arr_player[p_id].ps.hp <= 0 && arr_player[p_id].state != dead) {\
        arr_player[p_id].m_slock.lock();
        arr_player[p_id].time_dead = chrono::system_clock::now();
        arr_player[p_id].state = dead;
        arr_player[p_id].cur_pos = arr_player[p_id].init_pos;
        arr_player[p_id].m_slock.unlock();
    }


    if (arr_player[p_id].state == dead && chrono::system_clock::now() - arr_player[p_id].time_dead > chrono::milliseconds(2000)) {
        player_status ps;
        arr_player[p_id].m_slock.lock();
        const int hp = arr_player[p_id].ps.max_hp;
        arr_player[p_id].ps = ps;
        arr_player[p_id].ps.max_hp = arr_player[p_id].ps.hp = Mathf::Min(hp, 100);
        switch (arr_player[p_id].wp_type)
        {
        case WEAPON_RIFLE:
            arr_player[p_id].ps.attack_speed = 0.15f;
            arr_player[p_id].ps.max_ammo = 30;
            arr_player[p_id].ps.ammo = 30;
            arr_player[p_id].ps.attack_damage = 7.f;
            break;
        case WEAPON_SHOTGUN:
            arr_player[p_id].ps.attack_speed = 0.5f;
            arr_player[p_id].ps.max_ammo = 7;
            arr_player[p_id].ps.ammo = 7;
            arr_player[p_id].ps.attack_damage = 5.f;
            break;
        case WEAPON_SNIPER:
            arr_player[p_id].ps.attack_speed = 1.0f;
            arr_player[p_id].ps.max_ammo = 5;
            arr_player[p_id].ps.ammo = 5;
            arr_player[p_id].ps.attack_damage = 50.f;
            break;
        default:
            cout << "type error" << endl;
            break;
        }
        arr_player[p_id].active_item = ITEM_EMPTY;
        arr_player[p_id].state = none;
        arr_player[p_id].m_slock.unlock();
    }
}

void GameMgr::key_input(cs_ingame_packet cs_packet)
{
}

void GameMgr::process_packet(const int p_id, unsigned char* p_buf)
{
    const auto cs_packet = reinterpret_cast<cs_ingame_packet*>(p_buf);

        for (int i = 0; i < 5; ++i)
        {
            arr_player[p_id].bullet[i].in_use = false;
            arr_player[p_id].bullet[i].type = type_none;
        }
        arr_player[p_id].current_item = ITEM_EMPTY;


        if (arr_player[p_id].state != dead)
        {
            if (cs_packet->input.Key_W == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].cur_pos.z = arr_player[p_id].cur_pos.z + cs_packet->look.z * arr_player[p_id].ps.speed;
                arr_player[p_id].cur_pos.x = arr_player[p_id].cur_pos.x + cs_packet->look.x * arr_player[p_id].ps.speed;
            }
            if (cs_packet->input.Key_S == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].cur_pos.z = arr_player[p_id].cur_pos.z - cs_packet->look.z * arr_player[p_id].ps.speed;
                arr_player[p_id].cur_pos.x = arr_player[p_id].cur_pos.x - cs_packet->look.x * arr_player[p_id].ps.speed;
            }
            if (cs_packet->input.Key_A == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].cur_pos.z = arr_player[p_id].cur_pos.z + cs_packet->look.x * arr_player[p_id].ps.speed;
                arr_player[p_id].cur_pos.x = arr_player[p_id].cur_pos.x - cs_packet->look.z * arr_player[p_id].ps.speed;
            }
            if (cs_packet->input.Key_D == true) {
                arr_player[p_id].state = ::move;
                arr_player[p_id].cur_pos.z = arr_player[p_id].cur_pos.z - cs_packet->look.x * arr_player[p_id].ps.speed;
                arr_player[p_id].cur_pos.x = arr_player[p_id].cur_pos.x + cs_packet->look.z * arr_player[p_id].ps.speed;
            }
            if (!(cs_packet->input.Key_W || cs_packet->input.Key_S || cs_packet->input.Key_A || cs_packet->input.Key_D)) arr_player[p_id].state = ::none;
            if (cs_packet->input.Key_Q == true) {
                if (arr_player[p_id].active_item == ITEM_MAXHPUP)
                {
                    arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + 20, arr_player[p_id].ps.max_hp);
                }
                else if (arr_player[p_id].active_item == ITEM_MONSTER_SLOW)
                {
                    is_slow_ = true;  
                    slow_time_ = chrono::system_clock::now();
                }
                arr_player[p_id].active_item = ITEM_EMPTY;
            }
            if (cs_packet->input.Key_B == true) {
                arr_player[p_id].cur_pos = Vector3(487, 0, 589);
                arr_player[p_id].cur_pos.y = map_data->GetHeight(arr_player[p_id].cur_pos.x, arr_player[p_id].cur_pos.z) * map_data->GetScale().y + 100;
                zone_level_ = 4;
            }
            if (cs_packet->input.Key_N == true) {
                arr_player[p_id].cur_pos = Vector3(557.519, 0, 1202.554);
                arr_player[p_id].cur_pos.y = map_data->GetHeight(arr_player[p_id].cur_pos.x, arr_player[p_id].cur_pos.z) * map_data->GetScale().y + 100;
                zone_level_ = 3;
            }
            if (cs_packet->input.Key_M == true) {
                arr_player[p_id].ps.attack_damage = 500;
            }
        }
        if (cs_packet->input.Key_E == true) {
            pick_interaction_object(p_id);
        }

        if (cs_packet->input.Key_R == true) {
            if(!arr_player[p_id].reload_enable && arr_player[p_id].ps.ammo < arr_player[p_id].ps.max_ammo)
            {
                arr_player[p_id].reload_enable = true;
                arr_player[p_id].time_reload = chrono::system_clock::now();
            }
        }

        //아이템 처리
        if (cs_packet->item.doSend)
        {
            const int chest_id = cs_packet->item.chestId;
            const int item_id = cs_packet->item.itemId;
            if(!interaction_[chest_id].item.at(item_id).getEnable)
            { 
                arr_player[p_id].pl_items.find(interaction_[chest_id].item.at(item_id).item)->second += 1;
                interaction_[chest_id].item.at(item_id).getEnable = true;
                set_item(p_id, interaction_[chest_id].item.at(item_id).item);
            }
        }

        if (cs_packet->type == CS_SHOOT_PACKET && arr_player[p_id].state != dead) {
            if (arr_player[p_id].ps.ammo > 0 && !arr_player[p_id].reload_enable) {
                arr_player[p_id].state = attack;
                arr_player[p_id].ps.ammo--;
                if (arr_player[p_id].wp_type == WEAPON_SHOTGUN)
                    find_collide_object_shot_gun(p_id);
                else
                    find_collide_object(p_id);
            }

        }
        if (arr_player[p_id].ps.ammo == 0)
        {
            if (!arr_player[p_id].reload_enable)
            {
                arr_player[p_id].reload_enable = true;
                arr_player[p_id].time_reload = chrono::system_clock::now();
            }
        }
        if (chrono::system_clock::now() - arr_player[p_id].time_reload > chrono::milliseconds(2200) && arr_player[p_id].reload_enable)
        {
            arr_player[p_id].ps.ammo = arr_player[p_id].ps.max_ammo;
            arr_player[p_id].reload_enable = false;
        }
    // gravity
    if (arr_player[p_id].cur_pos.y > map_data->GetHeight(arr_player[p_id].cur_pos.x, arr_player[p_id].cur_pos.z) * map_data->GetScale().y + 100
        && arr_player[p_id].ps.hp >= 0) // ���� �߻�
        arr_player[p_id].cur_pos.y -= 7;
    else
        arr_player[p_id].cur_pos.y = map_data->GetHeight(arr_player[p_id].cur_pos.x, arr_player[p_id].cur_pos.z) * map_data->GetScale().y + 100;

    arr_player[p_id].pl_look = cs_packet->look;
    arr_player[p_id].cam_look = cs_packet->cameraLook;

    const XMFLOAT4X4 unit
    (
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        arr_player[p_id].cur_pos.x, arr_player[p_id].cur_pos.y, arr_player[p_id].cur_pos.z, 1
    );

    arr_player[p_id].OOBB.Transform(arr_player[p_id].OOBB, DirectX::XMLoadFloat4x4(&unit));
    arr_player[p_id].OOBB.Extents.x = obb_scale_player_x;
    arr_player[p_id].OOBB.Extents.y = obb_scale_player_y;
    arr_player[p_id].OOBB.Extents.z = obb_scale_player_z;

    player_coll_check(p_id);
    check_player_dead(p_id);

    arr_player[p_id].prev_pos = arr_player[p_id].cur_pos;
}


void GameMgr::player_coll_check(const int id) const
{
    for (int i = 0; i < structure_.size(); ++i) {
        const ContainmentType contain_type = arr_player[id].OOBB.Contains(structure_[i].OOBB);

        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            break;
        case CONTAINS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            break;

        default:
            break;
        }
    }

    for (auto& p : interaction_)
    {
        if (p.object_name == door && p.interact_enable == true) continue;
        const ContainmentType contain_type = arr_player[id].OOBB.Contains(p.OOBB);
        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            break;
        case CONTAINS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            break;
        default:
            break;
        }
    }

    for (int i = 0; i < map_lava_.size(); ++i)
    {
        const ContainmentType contain_type = map_lava_[i].Contains(arr_player[id].OOBB);

        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            if (arr_player[id].ps.hp > 0)
                arr_player[id].ps.hp -= 1;
            break;
        case CONTAINS:
            arr_player[id].cur_pos = arr_player[id].prev_pos;
            if (arr_player[id].ps.hp > 0)
                arr_player[id].ps.hp -= 1;
            break;

        default:
            break;
        }
    }

}

void GameMgr::npc_coll_check(const int id)

{
    for (int i = 0; i < structure_.size(); ++i) {
        const ContainmentType contain_type = npc_[id].OOBB.Contains(structure_[i].OOBB);

        switch (contain_type)
        {
        case DISJOINT:
            break;
        case INTERSECTS:
        {
   /*         npc[id].state = none;*/
            npc_[id].cur_pos = npc_[id].prev_pos;
            Vector3 look_vec = npc_[id].init_pos - npc_[id].cur_pos;
            npc_[id].look_vec = look_vec.normalized();
            break;
        }
        case CONTAINS:
        {
            //npc[id].state = none;
            npc_[id].cur_pos = npc_[id].prev_pos;
            Vector3 look_vec = npc_[id].init_pos - npc_[id].cur_pos;
            npc_[id].look_vec = look_vec.normalized();
            break;
        }
        default:
            break;
        }
    }

}

bool GameMgr::collide_object_by_ray_intersection(const BoundingOrientedBox& object_bounding_box, const Vector3& position, const Vector3& direction, float* distance)
{
    const XMVECTOR xm_ray_origin = XMLoadFloat3(&position);
    const XMVECTOR xm_ray_direction = XMLoadFloat3(&direction);
    return object_bounding_box.Intersects(xm_ray_origin, xm_ray_direction, *distance);
}

void GameMgr::pick_interaction_object(int p_id)
{
    bool			is_intersected = false;
    float			hit_distance = 75.f, nearest_hit_distance = 150.f;
    obj_type        object_type = type_none;
    int             object_id = 0;
    Vector3         camera_pos = arr_player[p_id].cur_pos;

    camera_pos.y += 20;

    for (int i = 0; i < max_intraction; ++i) // monster check
    {
        is_intersected = collide_object_by_ray_intersection(interaction_[i].OOBB, camera_pos, arr_player[p_id].cam_look, &hit_distance);
        if (is_intersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = type_static;
            object_id = i;
        }
    }
    if (object_type == type_static)
    {
        if (interaction_[object_id].interact_enable == false)
        {
            bool is_open = true;
            if (interaction_[object_id].zone_num != 99)
            {
                for (const auto p : game_zones_.at(interaction_[object_id].zone_num).monster_id)
                {
                    if (npc_[p].hp > 0) is_open = false;
                }
                if (interaction_[object_id].zone_num == 3 && !interaction_[12].interact_enable) is_open = false;
                if (is_open) zone_level_ = ++interaction_[object_id].zone_num;
            }
            interaction_[object_id].interact_enable = is_open;
        }
    }
}

void GameMgr::find_collide_object(int p_id)
{
    Vector3         camera_pos = arr_player[p_id].cur_pos;
    camera_pos.y += 20;
    bool			is_intersected = false;
    float			hit_distance = FLT_MAX, nearest_hit_distance = FLT_MAX;
    int             object_id = 0;
    obj_type object_type = type_none;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0)) arr_player[p_id].cam_look = Vector3(0, 1, 0);
    arr_player[p_id].bullet[0].in_use = true;

    Matrix4x4		mat_collide_position = Matrix4x4::identity; // identity: 4x4 unit
    for (int i = 0; i < max_object; ++i) // monster check
    {
        if (npc_[i].hp <= 0) continue;
        is_intersected = collide_object_by_ray_intersection(npc_[i].OOBB, camera_pos, arr_player[p_id].cam_look, &hit_distance);
        if (is_intersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = type_npc;
            object_id = i;
        }
    }

    for (int i = 0; i < max_intraction; ++i) // monster check
    {
        if (interaction_[i].object_name == door && interaction_[i].interact_enable == true) continue;
        is_intersected = collide_object_by_ray_intersection(interaction_[i].OOBB, camera_pos, arr_player[p_id].cam_look, &hit_distance);
        if (is_intersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = type_static;
            object_id = i;
        }
    }

    for (int i = 0; i < structure_.size(); ++i)
    {
        if (structure_[i].type == fence) continue;
        is_intersected = collide_object_by_ray_intersection(structure_[i].OOBB, camera_pos, arr_player[p_id].cam_look, &hit_distance);
        if (is_intersected && (hit_distance < nearest_hit_distance))
        {
            nearest_hit_distance = hit_distance;
            object_type = type_static;
            object_id = i;
        }
    }

    ////////////////////////////////////////
    if (type_none != object_type)
    {
        const auto distance = static_cast<float>(pow(nearest_hit_distance, 2));
        const float sum_look_pos = static_cast<float>(pow(arr_player[p_id].cam_look.x, 2)) + static_cast<float>(pow(arr_player[p_id].cam_look.y, 2)) + static_cast<float>(pow(arr_player[p_id].cam_look.z, 2));
        const float final = distance / sum_look_pos;
        // return collision CurPos (burn effects)
        mat_collide_position._41 = camera_pos.x + arr_player[p_id].cam_look.x * (sqrt(final) - 2);
        mat_collide_position._42 = camera_pos.y + arr_player[p_id].cam_look.y * (sqrt(final) - 2);
        mat_collide_position._43 = camera_pos.z + arr_player[p_id].cam_look.z * (sqrt(final) - 2);

        arr_player[p_id].bullet[0].pos.x = mat_collide_position._41;
        arr_player[p_id].bullet[0].pos.y = mat_collide_position._42;
        arr_player[p_id].bullet[0].pos.z = mat_collide_position._43;
    }

    switch (object_type)
    {
    case type_none:
        arr_player[p_id].bullet[0].in_use = false;
        break;
    case type_npc:
        if (npc_[object_id].state != dead)
        {
            arr_player[p_id].bullet[0].type = type_npc;
            for (const int p : game_zones_.at(npc_[object_id].zone_num).monster_id)
            {
                if (npc_[p].state != none) continue;
                npc_[p].dest_pl = p_id;
                npc_[p].state = hit;
            }
            if (npc_[object_id].dest_pl != p_id)
            {
                if (Vector3::Distance(npc_[object_id].cur_pos, arr_player[p_id].cur_pos) < Vector3::Distance(npc_[object_id].cur_pos, arr_player[npc_[object_id].dest_pl].cur_pos))
                {
                    npc_[object_id].dest_pl = p_id;
                    npc_[object_id].state = hit;
                }
            }

            if (0 < npc_[object_id].hp)
            {
                //몬스터 즉사 아이템 처리
                if (arr_player[p_id].ps.instant_death > npc_[object_id].hp && npc_[object_id].mob != golem)
                    npc_[object_id].hp = 0;
                else
                {
                    float damage = arr_player[p_id].ps.attack_damage;
                    //보스 데미지 증가 아이템처리
                    if (npc_[object_id].mob == golem)
                        damage = damage + damage * arr_player[p_id].ps.boss_damage * 0.01;
                    npc_[object_id].hp -= damage;
                    npc_[object_id].hp = Mathf::Max(npc_[object_id].hp, 0);
                }

                if (npc_[object_id].hp == 0)
                {
                    arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + arr_player[p_id].ps.kill_max_hp, arr_player[p_id].ps.max_hp);
                    npc_[object_id].state = dead;
                    npc_[object_id].time_death = chrono::system_clock::now();
                }
            }
            else
            {
                npc_[object_id].state = dead;
                npc_[object_id].time_death = chrono::system_clock::now();
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
    default: break;
    }
}

void GameMgr::find_collide_object_shot_gun(int p_id)
{
    Vector3         camera_pos = arr_player[p_id].cur_pos;
    camera_pos.y += 20;
    if (arr_player[p_id].cam_look == Vector3(0, 0, 0)) arr_player[p_id].cam_look = Vector3(0, 1, 0);

    for (int j = 0; j < 5; ++j)
    {
        bool			is_intersected = false;
        float			hit_distance = FLT_MAX, nearest_hit_distance = FLT_MAX;
        int             object_id = 0; 
        obj_type object_type = type_none;

        Vector3 camera_look = arr_player[p_id].cam_look;
        camera_look.x += Mathf::RandF(-0.15f, 0.15f);
        camera_look.y += Mathf::RandF(-0.15f, 0.15f);
        camera_look.z += Mathf::RandF(-0.15f, 0.15f);
        camera_look = camera_look.normalized();
        arr_player[p_id].bullet[j].in_use = true;
        Matrix4x4		mat_collide_position = Matrix4x4::identity; // identity: 4x4 unit
        for (int i = 0; i < max_object; ++i) // monster check
        {
            if (npc_[i].hp <= 0) continue;
            is_intersected = collide_object_by_ray_intersection(npc_[i].OOBB, camera_pos, camera_look, &hit_distance);
            if (is_intersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = type_npc;
                object_id = i;
            }
        }

        for (int i = 0; i < max_intraction; ++i) // monster check
        {
            if (interaction_[i].object_name == door && interaction_[i].interact_enable == true) continue;
            is_intersected = collide_object_by_ray_intersection(interaction_[i].OOBB, camera_pos, camera_look, &hit_distance);
            if (is_intersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = type_static;
                object_id = i;
            }
        }

        for (int i = 0; i < structure_.size(); ++i)
        {
            if (structure_[i].type == fence) continue;
            is_intersected = collide_object_by_ray_intersection(structure_[i].OOBB, camera_pos, camera_look, &hit_distance);
            if (is_intersected && (hit_distance < nearest_hit_distance))
            {
                nearest_hit_distance = hit_distance;
                object_type = type_static;
                object_id = i;
            }
        }

        ////////////////////////////////////////
        if (type_none != object_type)
        {
            const auto distance = static_cast<float>(pow(nearest_hit_distance, 2));
            const float sum_look_pos = static_cast<float>(pow(camera_look.x, 2)) + static_cast<float>(pow(camera_look.y, 2)) + static_cast<float>(pow(camera_look.z, 2));
            const float final = distance / sum_look_pos;
            // return collision CurPos (burn effects)
            mat_collide_position._41 = camera_pos.x + camera_look.x * (sqrt(final) - 2);
            mat_collide_position._42 = camera_pos.y + camera_look.y * (sqrt(final) - 2);
            mat_collide_position._43 = camera_pos.z + camera_look.z * (sqrt(final) - 2);

            arr_player[p_id].bullet[j].pos.x = mat_collide_position._41;
            arr_player[p_id].bullet[j].pos.y = mat_collide_position._42;
            arr_player[p_id].bullet[j].pos.z = mat_collide_position._43;
        }

        switch (object_type)
        {
        case type_none:
            arr_player[p_id].bullet[j].in_use = false;
            break;
        case type_npc:
            if (npc_[object_id].state != dead)
            {
                arr_player[p_id].bullet[j].type = type_npc;
                for (const int p : game_zones_.at(npc_[object_id].zone_num).monster_id)
                {
                    if (npc_[p].state != none) continue;
                    npc_[p].dest_pl = p_id;
                    npc_[p].state = hit;
                }
                if (npc_[object_id].dest_pl != p_id)
                {
                    if (Vector3::Distance(npc_[object_id].cur_pos, arr_player[p_id].cur_pos) < Vector3::Distance(npc_[object_id].cur_pos, arr_player[npc_[object_id].dest_pl].cur_pos))
                    {
                        npc_[object_id].dest_pl = p_id;
                        npc_[object_id].state = hit;
                    }
                }

                if (0 < npc_[object_id].hp)
                {
                    //몬스터 즉사 아이템 처리
                    if (arr_player[p_id].ps.instant_death > npc_[object_id].hp&& npc_[object_id].mob != golem)
                        npc_[object_id].hp = 0;
                    else
                    {
                        float damage = arr_player[p_id].ps.attack_damage;
                        //보스 데미지 증가 아이템처리
                        if (npc_[object_id].mob == golem)
                            damage = damage + damage * arr_player[p_id].ps.boss_damage * 0.01;
                        npc_[object_id].hp -= damage;
                        npc_[object_id].hp = Mathf::Max(npc_[object_id].hp, 0);
                    }

                    if (npc_[object_id].hp == 0)
                    {
                        arr_player[p_id].ps.hp = Mathf::Min(arr_player[p_id].ps.hp + arr_player[p_id].ps.kill_max_hp, arr_player[p_id].ps.max_hp);
                        npc_[object_id].state = dead;
                        npc_[object_id].time_death = chrono::system_clock::now();
                    }
                }
                else
                {
                    npc_[object_id].state = dead;
                    npc_[object_id].time_death = chrono::system_clock::now();
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
        default: ;
        }
    }
}

void GameMgr::check_interaction_object(int p_id)
{
}

void GameMgr::set_hp(int id, int hp)
{
    arr_player[id].ps.hp = hp;
}

void GameMgr::set_pos(int id, XMFLOAT3 pos)
{
}


void GameMgr::set_item(int id, ITEM_TYPE item)
{
    arr_player[id].current_item = item;
    switch (item)
    {
    case ITEM_TYPE::ITEM_DAMAGEUP_MAXHPDOWN:
        //데미지 증가 최대체력 감소 아이템처리
        arr_player[id].ps.max_hp /= 2;
        arr_player[id].ps.hp = Mathf::Min(arr_player[id].ps.max_hp, arr_player[id].ps.hp);
        arr_player[id].ps.attack_damage *= 2;
        break;
    case ITEM_TYPE::ITEM_BLOCK_DAMAGE:
        arr_player[id].ps.block += 10;
        break;
    case ITEM_TYPE::ITEM_BOSS_DAMAGEUP:
        arr_player[id].ps.boss_damage += 20;
        break;
    case ITEM_TYPE::ITEM_KILL_MAXHPUP:
        arr_player[id].ps.kill_max_hp += 1;
        break;
    case ITEM_TYPE::ITEM_INSTANT_DEATH:
        if (arr_player[id].ps.instant_death == 0)
            arr_player[id].ps.instant_death += 10;
        else
            arr_player[id].ps.instant_death += 5;
        break;
    case ITEM_TYPE::ITEM_MONSTER_SLOW:
        arr_player[id].active_item = item;
        //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ITEM_TYPE::ITEM_PLAYER_SPEEDUP:
        //플레이어 스피드업 아이템처리
        arr_player[id].ps.speed *= 1.2;
        break;
    case ITEM_TYPE::ITEM_MAXHPUP:
        //최대 체력 증가 아이템 처리
        arr_player[id].active_item = item;
        //arr_player[id].currentItem = ITEM_EMPTY;
        break;
    case ITEM_TYPE::ITEM_ATTACK_SPEEDUP:
        arr_player[id].ps.attack_speed /= 1.2f;
        break;
    default: break;
    }
}

Vector3 GameMgr::get_position_to_height_map(float x, float z, const float addy) const
{
    Vector3 pos;
    pos.x = x;
    pos.z = z;
    pos.y = map_data->GetHeight(x, z) * map_data->GetScale().y + addy;
    return pos;
}



sc_ingame_packet GameMgr::get_packet(sc_ingame_packet packet) const
{
    sc_ingame_packet sc_packet;
    
    if (is_ending) {
        sc_packet.type = SC_GAME_TO_ENDING_PACKET;
        const chrono::duration<int> t = chrono::duration_cast<chrono::seconds> (e_time - s_time);
        sc_packet.play_time = t.count();
    }
    else
        sc_packet.type = SC_INGAME_PACKET;
    sc_packet.size = sizeof(sc_ingame_packet);
    sc_packet.id = 0;


    for (int i = 0; i < max_player; ++i)
    {
        const int id = pl_list[i];

        sc_packet.player[i].pos = arr_player[id].cur_pos;
        sc_packet.player[i].look = arr_player[id].pl_look;
        sc_packet.player[i].cameraLook = arr_player[id].cam_look;
        sc_packet.player[i].state = arr_player[id].state;
        sc_packet.player[i].id = i;
        sc_packet.player[i].zoneNum = zone_level_;
        sc_packet.player[i].ps.hp = arr_player[id].ps.hp;
        sc_packet.player[i].ps.maxHp = arr_player[id].ps.max_hp;
        sc_packet.player[i].ps.attackSpeed = arr_player[id].ps.attack_speed;
        sc_packet.player[i].reloadEnable = arr_player[id].reload_enable;
        sc_packet.player[i].ammo = arr_player[id].ps.ammo;
        memset(sc_packet.player[i].bullet, NULL, sizeof(Bullet) * 5);
        memcpy(sc_packet.player[i].bullet, arr_player[id].bullet, sizeof(Bullet) * 5);
        sc_packet.player[i].currentItem = arr_player[id].current_item;
    }

    for (int i = 0; i < max_object; ++i) // 오류 발생
    {
        sc_packet.npc[i].id = i;
        sc_packet.npc[i].pos = npc_[i].cur_pos;
        sc_packet.npc[i].look = npc_[i].look_vec;
        sc_packet.npc[i].hp = npc_[i].hp;
        sc_packet.npc[i].attackEnable = npc_[i].attack_packet_enable;
        sc_packet.npc[i].state = npc_[i].state;
    }

    for (int i = 0; i < max_intraction; ++i)
    {
        sc_packet.interaction[i].pos = interaction_[i].pos;
        sc_packet.interaction[i].look = interaction_[i].lookvec;
        if (interaction_[i].item.size() != 0)
        {
            sc_packet.interaction[i].item[0].itemType = interaction_[i].item.at(0).item;
            sc_packet.interaction[i].item[0].isAlive = !interaction_[i].item.at(0).getEnable;
            sc_packet.interaction[i].item[1].itemType = interaction_[i].item.at(1).item;
            sc_packet.interaction[i].item[1].isAlive = !interaction_[i].item.at(1).getEnable;
            sc_packet.interaction[i].item[2].itemType = interaction_[i].item.at(2).item;
            sc_packet.interaction[i].item[2].isAlive = !interaction_[i].item.at(2).getEnable;
            sc_packet.interaction[i].item[3].itemType = interaction_[i].item.at(3).item;
            sc_packet.interaction[i].item[3].isAlive = !interaction_[i].item.at(3).getEnable;
        }
        sc_packet.interaction[i].interactEnable = interaction_[i].interact_enable;
        sc_packet.interaction[i].state = interaction_[i].state;
    }

    for (int i = 0; i < stone_attack_.size(); ++i)
    {
        sc_packet.stone[i].activeEnable = stone_attack_[i].active_enable;
        sc_packet.stone[i].pos = stone_attack_[i].pos;
    }

    for (int i = 0; i < 20; ++i)
    {
        sc_packet.attack[i].activeEnable = range_attack_[i].active_enable;
        sc_packet.attack[i].pos = range_attack_[i].pos;
    }

    return sc_packet;
}
