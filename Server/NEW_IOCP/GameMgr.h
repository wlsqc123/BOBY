#pragma once
#include "stdafx.h"
#include "mapData.h"

class GameMgr
{
private:
	vector<NPC>							npc_;

	chrono::system_clock::time_point	delta_time_;
	chrono::system_clock::time_point	current_time_;

	vector<structure>					structure_;
	vector<BoundingOrientedBox>			map_lava_;
	vector<interaction>					interaction_;
	vector<zone>						game_zones_;
	vector<range_attack>					range_attack_;
	vector<range_attack>					stone_attack_;

	bool								is_slow_ = false;
	chrono::system_clock::time_point	slow_time_;
	int zone_level_;
	chrono::system_clock::time_point	stone_time_;
	
public:
	CHeightMapImage* map_data;
	bool	is_running = false;
	bool    is_ending = false;
	
	GameMgr();
	~GameMgr();

	int r_id;
	int pl_list[max_player];

	void init_game(int id[4]);


	void update();
	void trace_player(int n_id);
	void attack_player(int n_id);

	static void check_player_dead(int p_id);

	static void key_input(cs_ingame_packet cs_packet);
	void process_packet(int p_id, unsigned char* p_buf);

	void player_coll_check(int id);
	void npc_coll_check(int id);

	//bullet collision
	static bool collide_object_by_ray_intersection(const BoundingOrientedBox& object_bounding_box, const Vector3& position, const Vector3& direction, float* distance);
	void find_collide_object(int p_id);
	void find_collide_object_shot_gun(int p_id);
	void pick_interaction_object(int p_id);
	static void check_interaction_object(int p_id);


	static void set_hp(int id, int hp);
	static void set_pos(int id, XMFLOAT3 pos);
	static void set_item(int id, ITEM_TYPE item);


	Vector3 get_position_to_height_map(float x, float z, float addy) const;

	sc_ingame_packet	get_packet(sc_ingame_packet packet) const;
	static XMFLOAT3	get_pos(const int id) { return arr_player[id].cur_pos; };
	static XMFLOAT3	get_look(const int id) { return arr_player[id].pl_look; };
	static XMFLOAT3	get_camera_look(const int id) { return arr_player[id].cam_look; };
	XMFLOAT3		get_npc(const int id) const { return npc_[id].cur_pos; };
	static float	get_hp(const int id) { return arr_player[id].ps.hp; };
	int			get_id(int id) const { return r_id; };

	chrono::system_clock::time_point cur_update_time = chrono::system_clock::now();
	chrono::system_clock::time_point s_time;
	chrono::system_clock::time_point e_time;
};

