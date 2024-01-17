#pragma once
#include "stdafx.h"
#include <inc/fmod.hpp>

#define SOUND_NUM 13
#define CHANNEL_NUM 200

using namespace FMOD;
enum SOUND_TYPE {
	SHOTGUN_SOUND,
	SNIPER_SOUND,
	RIFLE_SOUND,
	HIT_SOUND,
	MAGMA_DEATH,
	GOLEM_DEATH,
	TITLE_BGM,
	RIFLE_RELOAD_SOUND,
	SNIPER_RELOAD_SOUND,
	SHOTGUN_RELOAD_SOUND,
	DOOR_OPEN_SOUND,
	GOLEM_ATTACK_SOUND,
	ITEM_PICKUP_SOUND,
};

class Fmod_snd
{
public:
	Fmod_snd();
	~Fmod_snd();

private:
	
	FMOD_SYSTEM* g_sound_system;
	FMOD_SOUND* pSound[SOUND_NUM];
	FMOD_CHANNEL* pChannel[CHANNEL_NUM];
	void Add_sound();
public:
	void SetVolume(SOUND_TYPE type, float volume);
	void Play(SOUND_TYPE type);
	void PlayShoot(SOUND_TYPE type, float volume);
	void PlayReload(SOUND_TYPE type, float volume);
	void PlayMonster(SOUND_TYPE type);
	void Stop(SOUND_TYPE type);
	void Update();
};

