#pragma once

#include "RSDKTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSDK_ORBWALKER_API_VERSION 6u

typedef enum RSDK_OrbwalkingMode
{
    RSDK_ORBWALKER_MODE_LAST_HIT = 0,
    RSDK_ORBWALKER_MODE_MIXED = 1,
    RSDK_ORBWALKER_MODE_LANE_CLEAR = 2,
    RSDK_ORBWALKER_MODE_COMBO = 3,
    RSDK_ORBWALKER_MODE_FREEZE = 4,
    RSDK_ORBWALKER_MODE_CUSTOM = 5,
    RSDK_ORBWALKER_MODE_NONE = 6,
    RSDK_ORBWALKER_MODE_FLEE = 7
} RSDK_OrbwalkingMode;

typedef struct RSDK_OrbwalkerBeforeAttackEvent
{
    uint32_t unitNetworkId;
    uint32_t targetNetworkId;
    RSDK_Bool process;
    uint8_t reserved[3];
} RSDK_OrbwalkerBeforeAttackEvent;

typedef struct RSDK_OrbwalkerAttackEvent
{
    uint32_t unitNetworkId;
    uint32_t targetNetworkId;
} RSDK_OrbwalkerAttackEvent;

typedef struct RSDK_OrbwalkerTargetChangeEvent
{
    uint32_t oldTargetNetworkId;
    uint32_t newTargetNetworkId;
} RSDK_OrbwalkerTargetChangeEvent;

typedef void(RSDK_CALL* RSDK_OrbwalkerBeforeAttackCallback)(RSDK_OrbwalkerBeforeAttackEvent* eventData, void* userData);
typedef void(RSDK_CALL* RSDK_OrbwalkerAttackCallback)(const RSDK_OrbwalkerAttackEvent* eventData, void* userData);
typedef void(RSDK_CALL* RSDK_OrbwalkerNonKillableMinionCallback)(const RSDK_OrbwalkerAttackEvent* eventData, void* userData);
typedef void(RSDK_CALL* RSDK_OrbwalkerTargetChangeCallback)(const RSDK_OrbwalkerTargetChangeEvent* eventData, void* userData);

/* Implemented by Orbwalker.dll. Core owns the stable facade and forwards calls to one shared provider. */
typedef struct RSDK_OrbwalkerProviderApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    uint8_t(RSDK_CALL* GetActiveMode)(void);
    void(RSDK_CALL* SetActiveMode)(uint8_t mode);
    uint32_t(RSDK_CALL* GetTarget)(void);
    void(RSDK_CALL* ForceTarget)(uint32_t networkId);
    void(RSDK_CALL* ClearForcedTarget)(void);

    RSDK_Bool(RSDK_CALL* GetAttackEnabled)(void);
    void(RSDK_CALL* SetAttackEnabled)(RSDK_Bool enabled);
    RSDK_Bool(RSDK_CALL* GetMovementEnabled)(void);
    void(RSDK_CALL* SetMovementEnabled)(RSDK_Bool enabled);

    RSDK_Bool(RSDK_CALL* CanAttack)(void);
    RSDK_Bool(RSDK_CALL* CanMove)(float extraWindup, RSDK_Bool disableMissileCheck);
    float(RSDK_CALL* GetAttackRange)(uint32_t targetNetworkId);
    float(RSDK_CALL* GetRealAutoAttackRange)(uint32_t targetNetworkId);
    RSDK_Bool(RSDK_CALL* InAutoAttackRange)(uint32_t targetNetworkId);
    float(RSDK_CALL* GetMyProjectileSpeed)(void);
    RSDK_Bool(RSDK_CALL* IsAutoAttack)(const char* spellName);
    RSDK_Bool(RSDK_CALL* IsAutoAttackReset)(const char* spellName);
    void(RSDK_CALL* ResetAutoAttackTimer)(void);

    void(RSDK_CALL* SetMinimumOrbwalkDistance)(float distance);
    void(RSDK_CALL* SetMovementDelay)(float delay);
    void(RSDK_CALL* SetOrbwalkingPoint)(RSDK_Vec3 position);
    void(RSDK_CALL* ClearOrbwalkingPoint)(void);
    void(RSDK_CALL* Orbwalk)(uint32_t targetNetworkId, RSDK_Vec3 position, float extraWindup, float holdAreaRadius,
        RSDK_Bool useFixedDistance, RSDK_Bool randomizeMinDistance);
    void(RSDK_CALL* MoveTo)(RSDK_Vec3 position, float holdAreaRadius, RSDK_Bool overrideTimer,
        RSDK_Bool useFixedDistance, RSDK_Bool randomizeMinDistance);
    RSDK_Bool(RSDK_CALL* ShouldWait)(void);

    uint32_t(RSDK_CALL* GetLastTarget)(void);
    uint32_t(RSDK_CALL* GetLastTargetAt)(uint32_t index); /* 0 = newest, 2 = oldest */
    /* Gameplay timestamps are Game::TimeGame() seconds, not wall-clock/tick-count milliseconds. */
    float(RSDK_CALL* GetLastAATick)(void);
    float(RSDK_CALL* GetLastAttackCommandTime)(void);
    float(RSDK_CALL* GetLastMoveCommandTime)(void);
    RSDK_Vec3(RSDK_CALL* GetLastMoveCommandPosition)(void);

    /* HealthPrediction is implemented by the same Orbwalker provider but exposed as a separate public C++ class. */
    float(RSDK_CALL* GetHealthPrediction)(uint32_t unitNetworkId, float time, float delay);
    float(RSDK_CALL* LaneClearHealthPrediction)(uint32_t unitNetworkId, float time, float delay);
    RSDK_Bool(RSDK_CALL* HasMinionAggro)(uint32_t minionNetworkId);
    RSDK_Bool(RSDK_CALL* HasTurretAggro)(uint32_t minionNetworkId);
    float(RSDK_CALL* TurretAggroStartTime)(uint32_t minionNetworkId);
} RSDK_OrbwalkerProviderApi;

/*
 * Stable Core facade. Consumer plugins never import Orbwalker.dll directly.
 * Event callbacks are owned by the registering plugin and are removed safely before that DLL unloads.
 */
typedef struct RSDK_OrbwalkerApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    RSDK_Bool(RSDK_CALL* IsAvailable)(void);
    uint8_t(RSDK_CALL* GetActiveMode)(void);
    void(RSDK_CALL* SetActiveMode)(uint8_t mode);
    uint32_t(RSDK_CALL* GetTarget)(void);
    void(RSDK_CALL* ForceTarget)(uint32_t networkId);
    void(RSDK_CALL* ClearForcedTarget)(void);

    RSDK_Bool(RSDK_CALL* GetAttackEnabled)(void);
    void(RSDK_CALL* SetAttackEnabled)(RSDK_Bool enabled);
    RSDK_Bool(RSDK_CALL* GetMovementEnabled)(void);
    void(RSDK_CALL* SetMovementEnabled)(RSDK_Bool enabled);

    RSDK_Bool(RSDK_CALL* CanAttack)(void);
    RSDK_Bool(RSDK_CALL* CanMove)(float extraWindup, RSDK_Bool disableMissileCheck);
    float(RSDK_CALL* GetAttackRange)(uint32_t targetNetworkId);
    float(RSDK_CALL* GetRealAutoAttackRange)(uint32_t targetNetworkId);
    RSDK_Bool(RSDK_CALL* InAutoAttackRange)(uint32_t targetNetworkId);
    float(RSDK_CALL* GetMyProjectileSpeed)(void);
    RSDK_Bool(RSDK_CALL* IsAutoAttack)(const char* spellName);
    RSDK_Bool(RSDK_CALL* IsAutoAttackReset)(const char* spellName);
    void(RSDK_CALL* ResetAutoAttackTimer)(void);

    void(RSDK_CALL* SetMinimumOrbwalkDistance)(float distance);
    void(RSDK_CALL* SetMovementDelay)(float delay);
    void(RSDK_CALL* SetOrbwalkingPoint)(RSDK_Vec3 position);
    void(RSDK_CALL* ClearOrbwalkingPoint)(void);
    void(RSDK_CALL* Orbwalk)(uint32_t targetNetworkId, RSDK_Vec3 position, float extraWindup, float holdAreaRadius,
        RSDK_Bool useFixedDistance, RSDK_Bool randomizeMinDistance);
    void(RSDK_CALL* MoveTo)(RSDK_Vec3 position, float holdAreaRadius, RSDK_Bool overrideTimer,
        RSDK_Bool useFixedDistance, RSDK_Bool randomizeMinDistance);
    RSDK_Bool(RSDK_CALL* ShouldWait)(void);

    uint32_t(RSDK_CALL* GetLastTarget)(void);
    uint32_t(RSDK_CALL* GetLastTargetAt)(uint32_t index); /* 0 = newest, 2 = oldest */
    /* Gameplay timestamps are Game::TimeGame() seconds, not wall-clock/tick-count milliseconds. */
    float(RSDK_CALL* GetLastAATick)(void);
    float(RSDK_CALL* GetLastAttackCommandTime)(void);
    float(RSDK_CALL* GetLastMoveCommandTime)(void);
    RSDK_Vec3(RSDK_CALL* GetLastMoveCommandPosition)(void);

    float(RSDK_CALL* GetHealthPrediction)(uint32_t unitNetworkId, float time, float delay);
    float(RSDK_CALL* LaneClearHealthPrediction)(uint32_t unitNetworkId, float time, float delay);
    RSDK_Bool(RSDK_CALL* HasMinionAggro)(uint32_t minionNetworkId);
    RSDK_Bool(RSDK_CALL* HasTurretAggro)(uint32_t minionNetworkId);
    float(RSDK_CALL* TurretAggroStartTime)(uint32_t minionNetworkId);

    RSDK_EventToken(RSDK_CALL* RegisterBeforeAttack)(RSDK_OrbwalkerBeforeAttackCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnAttack)(RSDK_OrbwalkerAttackCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterAfterAttack)(RSDK_OrbwalkerAttackCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnNonKillableMinion)(RSDK_OrbwalkerNonKillableMinionCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnTargetChange)(RSDK_OrbwalkerTargetChangeCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* UnregisterEvent)(RSDK_EventToken token);

    RSDK_Bool(RSDK_CALL* RegisterProvider)(const RSDK_OrbwalkerProviderApi* provider);
    RSDK_Bool(RSDK_CALL* UnregisterProvider)(const RSDK_OrbwalkerProviderApi* provider);

    /* Provider-only event bridge. Calls from non-provider plugins are ignored. */
    RSDK_Bool(RSDK_CALL* FireBeforeAttack)(uint32_t unitNetworkId, uint32_t targetNetworkId);
    void(RSDK_CALL* FireOnAttack)(uint32_t unitNetworkId, uint32_t targetNetworkId);
    void(RSDK_CALL* FireAfterAttack)(uint32_t unitNetworkId, uint32_t targetNetworkId);
    void(RSDK_CALL* FireOnNonKillableMinion)(uint32_t targetNetworkId);
    void(RSDK_CALL* FireOnTargetChange)(uint32_t oldTargetNetworkId, uint32_t newTargetNetworkId);

    /* v5.2.2 shared spell-farm flag owned by Core; setter is provider-only. */
    RSDK_Bool(RSDK_CALL* GetSpellFarmEnabled)(void);
    void(RSDK_CALL* SetSpellFarmEnabled)(RSDK_Bool enabled);
} RSDK_OrbwalkerApi;

#ifdef __cplusplus
}
#endif
