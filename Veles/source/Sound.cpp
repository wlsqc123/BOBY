#include "stdafx.h"
#include "Sound.h"

Fmod_snd::Fmod_snd()
{
	FMOD_System_Create(&g_sound_system);
	FMOD_System_Init(
		g_sound_system,
		CHANNEL_NUM
		, FMOD_INIT_NORMAL
		, nullptr
	);
	for (auto& p : pSound) p = nullptr;
	for (auto& p : pChannel) p = nullptr;
	Add_sound();
}

Fmod_snd::~Fmod_snd()
{
	for (int i = 0; i < SOUND_NUM; ++i)
	{
		FMOD_Sound_Release(pSound[i]);
	}
}


void Fmod_snd::Add_sound()
{
	FMOD_System_CreateSound(
		g_sound_system,
		"sound/shotgun.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::RIFLE_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::RIFLE_SOUND], 36000, 100);
	FMOD_System_CreateSound(
		g_sound_system,
		"sound/Sniper_Fire.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::SNIPER_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::SNIPER_SOUND], 36000, 100);
	FMOD_System_CreateSound(
		g_sound_system,
		"sound/shotgun_fire.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::SHOTGUN_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::SHOTGUN_SOUND], 36000, 100);
	FMOD_System_CreateSound(
		g_sound_system,
		"sound/soldier-injured2.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::HIT_SOUND]
	);
	FMOD_System_CreateSound(
		g_sound_system,
		"sound/death.ogg"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::MAGMA_DEATH]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::MAGMA_DEATH], 36000, 128);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/golemDeath.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::GOLEM_DEATH]
	);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/Spacious.mp3"
		, FMOD_LOOP_NORMAL | FMOD_2D
		, nullptr
		, &pSound[(int)SOUND_TYPE::TITLE_BGM]
	);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/m4_reload.mp3"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::RIFLE_RELOAD_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::RIFLE_RELOAD_SOUND], 36000, 100);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/reload-rifle.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::SNIPER_RELOAD_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::SNIPER_RELOAD_SOUND], 36000, 100);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/shotgun_insert.wav"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::SHOTGUN_RELOAD_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::SHOTGUN_RELOAD_SOUND], 36000, 100);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/WoodenDoor_Opening.mp3"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::DOOR_OPEN_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::DOOR_OPEN_SOUND], 36000, 100);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/Golem_Attack.mp3"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::GOLEM_ATTACK_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::GOLEM_ATTACK_SOUND], 36000, 130);

	FMOD_System_CreateSound(
		g_sound_system,
		"sound/Item_Pickup.mp3"
		, FMOD_DEFAULT | FMOD_LOOP_OFF
		, nullptr
		, &pSound[(int)SOUND_TYPE::ITEM_PICKUP_SOUND]
	);
	FMOD_Sound_SetDefaults(pSound[(int)SOUND_TYPE::ITEM_PICKUP_SOUND], 36000, 100);
}


void Fmod_snd::Play(SOUND_TYPE type)
{
	FMOD_BOOL m_bool;

	FMOD_System_Update(g_sound_system);
	FMOD_System_PlaySound(g_sound_system, pSound[(int)type], NULL, true, &pChannel[(int)type]);
	FMOD_Channel_SetVolume(pChannel[(int)type], 0.07f);
	FMOD_Channel_SetPaused(pChannel[(int)type], false);
}

void Fmod_snd::PlayShoot(SOUND_TYPE type, float volume)
{
	FMOD_BOOL m_bool;

	FMOD_System_Update(g_sound_system);
	FMOD_System_PlaySound(g_sound_system, pSound[(int)type], NULL, true, &pChannel[(int)type]);
	FMOD_Channel_SetVolume(pChannel[(int)type], volume);
	FMOD_Channel_SetPaused(pChannel[(int)type], false);
}

void Fmod_snd::PlayReload(SOUND_TYPE type, float volume)
{
	if ((pChannel[(int)type] == NULL))
	{
		FMOD_System_Update(g_sound_system);
		FMOD_System_PlaySound(g_sound_system, pSound[(int)type], NULL, true, &pChannel[(int)type]);
		FMOD_Channel_SetVolume(pChannel[(int)type], volume);
		FMOD_Channel_SetPaused(pChannel[(int)type], false);
	}
	else
	{
		FMOD_BOOL m_bool = false;
		FMOD_Channel_IsPlaying(pChannel[(int)type], &m_bool);
		if (!m_bool)
		{
			FMOD_System_Update(g_sound_system);
			FMOD_System_PlaySound(g_sound_system, pSound[(int)type], NULL, true, &pChannel[(int)type]);
			FMOD_Channel_SetVolume(pChannel[(int)type], volume);
			FMOD_Channel_SetPaused(pChannel[(int)type], false);
		}
	}
}

void Fmod_snd::PlayMonster(SOUND_TYPE type)
{
	FMOD_BOOL m_bool;

	FMOD_System_Update(g_sound_system);
	FMOD_System_PlaySound(g_sound_system, pSound[(int)type], NULL, true, &pChannel[(int)type]);
	//FMOD_Channel_SetVolume(pChannel[(int)type], 0.6f);
	FMOD_Channel_SetPaused(pChannel[(int)type], false);
}

void Fmod_snd::Stop(SOUND_TYPE type)
{
	FMOD_BOOL m_bool;

	FMOD_Channel_IsPlaying(pChannel[(int)type], &m_bool);

	if (m_bool)FMOD_Channel_Stop(pChannel[(int)type]);
}

void Fmod_snd::Update()
{
	FMOD_System_Update(g_sound_system);
}

void Fmod_snd::SetVolume(SOUND_TYPE type, float volume)
{
	FMOD_Channel_SetVolume(pChannel[(int)type], volume);
}