#pragma once

#include "RSDKTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSDK_TARGET_SELECTOR_API_VERSION 1u

// Compatibility typedef: damage type is now a generic RSDK constant/type.
typedef RSDK_DamageType RSDK_TargetDamageType;

typedef enum RSDK_TargetingMode
{
    RSDK_TARGET_MODE_AUTO_PRIORITY = 0,
    RSDK_TARGET_MODE_LOW_HP = 1,
    RSDK_TARGET_MODE_MOST_AD = 2,
    RSDK_TARGET_MODE_MOST_AP = 3,
    RSDK_TARGET_MODE_CLOSEST = 4,
    RSDK_TARGET_MODE_NEAR_MOUSE = 5,
    RSDK_TARGET_MODE_LESS_ATTACK = 6,
    RSDK_TARGET_MODE_LESS_CAST = 7,
    RSDK_TARGET_MODE_MOST_STACK = 8
} RSDK_TargetingMode;

/*
 * LeagueSharp mapping:
 *   TargetSelectionConditionDelegate conditions
 * RSDK naming:
 *   TargetFilter/filter. The callback is synchronous and valid only for the GetTarget call.
 */
typedef RSDK_Bool(RSDK_CALL* RSDK_TargetFilterCallback)(uint32_t networkId, void* userData);

typedef struct RSDK_TargetSelectorRequest
{
    uint32_t structSize;
    uint32_t sourceNetworkId; /* 0 = LocalPlayer */
    float range;
    uint8_t damageType;
    RSDK_Bool ignoreShieldSpells;
    const uint32_t* ignoredNetworkIds;
    uint32_t ignoredCount;
    RSDK_Bool hasRangeCheckFrom;
    RSDK_Vec3 rangeCheckFrom;
    RSDK_TargetFilterCallback filter;
    void* filterUserData;
} RSDK_TargetSelectorRequest;

/*
 * Implemented by TargetSelector.dll. Core stores this table while the provider is loaded
 * and forwards all consumers through one shared instance/state/menu.
 */
typedef struct RSDK_TargetSelectorProviderApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    uint32_t(RSDK_CALL* GetTarget)(const RSDK_TargetSelectorRequest* request);
    uint32_t(RSDK_CALL* GetSelectedTarget)(void);
    void(RSDK_CALL* SetTarget)(uint32_t networkId);
    void(RSDK_CALL* ClearTarget)(void);

    float(RSDK_CALL* GetPriority)(uint32_t networkId);
    int32_t(RSDK_CALL* GetPriorityLevel)(uint32_t networkId);
    void(RSDK_CALL* SetPriority)(uint32_t networkId, int32_t priority);

    uint8_t(RSDK_CALL* GetMode)(void);
    void(RSDK_CALL* SetMode)(uint8_t mode);

    RSDK_Bool(RSDK_CALL* GetCustomTS)(void);
    void(RSDK_CALL* SetCustomTS)(RSDK_Bool value);
    RSDK_Bool(RSDK_CALL* IsInvulnerable)(uint32_t networkId, uint8_t damageType, RSDK_Bool ignoreShields);
} RSDK_TargetSelectorProviderApi;

/*
 * Stable Core facade visible to every plugin. Consumer DLLs never import TargetSelector.dll directly.
 * RegisterProvider/UnregisterProvider are used by the public TargetSelector module implementation.
 */
typedef struct RSDK_TargetSelectorApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    RSDK_Bool(RSDK_CALL* IsAvailable)(void);
    uint32_t(RSDK_CALL* GetTarget)(const RSDK_TargetSelectorRequest* request);
    uint32_t(RSDK_CALL* GetSelectedTarget)(void);
    void(RSDK_CALL* SetTarget)(uint32_t networkId);
    void(RSDK_CALL* ClearTarget)(void);

    float(RSDK_CALL* GetPriority)(uint32_t networkId);
    int32_t(RSDK_CALL* GetPriorityLevel)(uint32_t networkId);
    void(RSDK_CALL* SetPriority)(uint32_t networkId, int32_t priority);

    uint8_t(RSDK_CALL* GetMode)(void);
    void(RSDK_CALL* SetMode)(uint8_t mode);

    RSDK_Bool(RSDK_CALL* GetCustomTS)(void);
    void(RSDK_CALL* SetCustomTS)(RSDK_Bool value);
    RSDK_Bool(RSDK_CALL* IsInvulnerable)(uint32_t networkId, uint8_t damageType, RSDK_Bool ignoreShields);

    RSDK_Bool(RSDK_CALL* RegisterProvider)(const RSDK_TargetSelectorProviderApi* provider);
    RSDK_Bool(RSDK_CALL* UnregisterProvider)(const RSDK_TargetSelectorProviderApi* provider);
} RSDK_TargetSelectorApi;

#ifdef __cplusplus
}
#endif
