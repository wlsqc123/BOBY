#pragma once
#include "../stdafx.h"
#include "../MapData/mapData.h"

class GameMgr
{
private:
    static constexpr int MAP_WIDTH = 513;
    static constexpr int MAP_HEIGHT = 513;
    static constexpr float SCALE_X = 10.0f;
    static constexpr float SCALE_Y = 25.0f;
    static constexpr float SCALE_Z = 10.0f;

    vector<NPC> npc_;

    chrono::system_clock::time_point lastUpdateDuration_;
    chrono::system_clock::time_point lastUpdateTime_;

    vector<Structure> structures_;
    vector<BoundingOrientedBox> lavaZones_;
    vector<INTERACTION> interactions_;
    vector<ZONE> zones_;
    vector<RangeAttack> rangeAttacks_;

    bool isSlowed_ = false;
    chrono::system_clock::time_point slowEffectEndTime;
    int currentZoneLevel_;

    vector<RangeAttack> stoneAttacks_;
    chrono::system_clock::time_point lastStoneAttackTime_;

public:
    GameMgr();
    ~GameMgr();

    unique_ptr<CHeightMapImage> mapData;
    bool isRunning = false;
    bool isEnding = false;
    int roomId;
    int playerIds[MAX_NUM_PLAYER];

    Vector3 GetPositionToHeightMap(float x, float z, float addy) const;

    void InitGame(int id[4]);

    void Update();
    void TracePlayer(int n_id);
    void AttackPlayer(int n_id);

    void CheckPlayerDead(int p_id);
    void ProcessKeyInput(cs_ingame_packet cspacket);
    void ProcessPacket(int p_id, unsigned char *p_buf);

    void CheckPlayerCollision(int id) const;
    void CheckNPCCollision(int id);

    //bullet collision
    bool CollideObjectByRayIntersection(const BoundingOrientedBox &objectBoundingBox, const Vector3 &position,
                                        const Vector3 &direction, float *distance);
    void FindCollideObject(int p_id);
    void FindCollideObjectShotGun(int p_id);
    void PickInteractionObject(int p_id);
    static void CheckinteractionObject(int p_id);

    static void SetHP(int id, int hp);
    void SetPosition(int id, XMFLOAT3 pos);
    static void SetItem(int id, ITEM_TYPE item);


    sc_ingame_packet GetPacket(sc_ingame_packet packet) const;
    static XMFLOAT3 GetPlayerPosition(int id);
    static XMFLOAT3 GetPlayerLook(int id);
    static XMFLOAT3 GetPlayerCameraLook(int id);
    static float GetPlayerHP(int id);
    XMFLOAT3 GetNPCPosition(int id);

    chrono::system_clock::time_point currentUpdateTime;
    chrono::system_clock::time_point startTime;
    chrono::system_clock::time_point endTime;
};
