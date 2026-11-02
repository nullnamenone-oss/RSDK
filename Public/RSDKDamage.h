#pragma once

#include "RSDKTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSDK_DAMAGE_API_VERSION 1u

typedef enum RSDK_DamageStage
{
    RSDK_DAMAGE_STAGE_DEFAULT = 0,
    RSDK_DAMAGE_STAGE_WAY_BACK = 1,
    RSDK_DAMAGE_STAGE_DETONATION = 2,
    RSDK_DAMAGE_STAGE_DAMAGE_PER_SECOND = 3,
    RSDK_DAMAGE_STAGE_SECOND_FORM = 4,
    RSDK_DAMAGE_STAGE_SECOND_CAST = 5,
    RSDK_DAMAGE_STAGE_BUFF = 6,
    RSDK_DAMAGE_STAGE_EMPOWERED = 7
} RSDK_DamageStage;

typedef enum RSDK_SpellDamageFlags
{
    RSDK_SPELL_DAMAGE_NONE = 0,
    // Deprecated compatibility flag in v5.2.1: single versioned damage database; ignored by provider.
    RSDK_SPELL_DAMAGE_CURRENT_DATA_ONLY = 1u << 0
} RSDK_SpellDamageFlags;

typedef enum RSDK_AutoAttackDamageFlags
{
    RSDK_AUTO_ATTACK_DAMAGE_NONE = 0,
    RSDK_AUTO_ATTACK_DAMAGE_INCLUDE_PASSIVES = 1u << 0
} RSDK_AutoAttackDamageFlags;

typedef struct RSDK_DamageCalculateRequest
{
    uint32_t structSize;
    uint32_t sourceNetworkId;
    uint32_t targetNetworkId;
    uint8_t damageType;
    uint8_t reserved[3];
    float rawDamage;
} RSDK_DamageCalculateRequest;

typedef struct RSDK_SpellDamageRequest
{
    uint32_t structSize;
    uint32_t sourceNetworkId;
    uint32_t targetNetworkId;
    int8_t slot;
    uint8_t stage;
    uint8_t reserved[2];
    uint32_t flags;
} RSDK_SpellDamageRequest;

typedef struct RSDK_AutoAttackDamageRequest
{
    uint32_t structSize;
    uint32_t sourceNetworkId;
    uint32_t targetNetworkId;
    uint32_t flags;
} RSDK_AutoAttackDamageRequest;

typedef struct RSDK_DamageProviderApi
{
    uint32_t structSize;
    uint32_t apiVersion;
    RSDK_Bool(RSDK_CALL* CalculateDamage)(const RSDK_DamageCalculateRequest* request, float* result);
    RSDK_Bool(RSDK_CALL* GetSpellDamage)(const RSDK_SpellDamageRequest* request, float* result);
    RSDK_Bool(RSDK_CALL* GetAutoAttackDamage)(const RSDK_AutoAttackDamageRequest* request, float* result);
} RSDK_DamageProviderApi;

typedef struct RSDK_DamageApi
{
    uint32_t structSize;
    uint32_t apiVersion;
    RSDK_Bool(RSDK_CALL* IsAvailable)(void);
    RSDK_Bool(RSDK_CALL* CalculateDamage)(const RSDK_DamageCalculateRequest* request, float* result);
    RSDK_Bool(RSDK_CALL* GetSpellDamage)(const RSDK_SpellDamageRequest* request, float* result);
    RSDK_Bool(RSDK_CALL* GetAutoAttackDamage)(const RSDK_AutoAttackDamageRequest* request, float* result);
    RSDK_Bool(RSDK_CALL* RegisterProvider)(const RSDK_DamageProviderApi* provider);
    RSDK_Bool(RSDK_CALL* UnregisterProvider)(const RSDK_DamageProviderApi* provider);
} RSDK_DamageApi;

#ifdef __cplusplus
}
#endif
