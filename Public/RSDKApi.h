#pragma once

#include "RSDKEvents.h"
#include "RSDKMenu.h"
#include "RSDKTargetSelector.h"
#include "RSDKOrbwalker.h"
#include "RSDKPrediction.h"
#include "RSDKDamage.h"



typedef struct RSDK_EventApi
{
    uint32_t structSize;
    RSDK_EventToken(RSDK_CALL* RegisterOnTick)(RSDK_OnTickCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnUpdate)(RSDK_OnUpdateCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnDraw)(RSDK_OnDrawCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnWndProc)(RSDK_OnWndProcCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnCreateObject)(RSDK_OnCreateObjectCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnDeleteObject)(RSDK_OnDeleteObjectCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnIssueOrder)(RSDK_OnIssueOrderCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnNewPath)(RSDK_OnNewPathCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnProcessSpell)(RSDK_OnProcessSpellCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnDoCast)(RSDK_OnDoCastCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnPlayAnimation)(RSDK_OnPlayAnimationCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnStopCast)(RSDK_OnStopCastCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnCastSpell)(RSDK_OnCastSpellCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnUpdateChargeableSpell)(RSDK_OnUpdateChargeableSpellCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnBuffAddRemove)(RSDK_OnBuffAddRemoveCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnUpdateBuff)(RSDK_OnUpdateBuffCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnSpellImpact)(RSDK_OnSpellImpactCallback callback, void* userData);
    RSDK_EventToken(RSDK_CALL* RegisterOnGameUpdate)(RSDK_OnGameUpdateCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* Unregister)(RSDK_EventToken token);
} RSDK_EventApi;

typedef struct RSDK_ObjectApi
{
    uint32_t structSize;

    uint32_t(RSDK_CALL* GetId)(RSDK_ObjectHandle object);
    uint32_t(RSDK_CALL* GetNetworkId)(RSDK_ObjectHandle object);
    int32_t(RSDK_CALL* GetTeam)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* GetPosition)(RSDK_ObjectHandle object, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetServerPosition)(RSDK_ObjectHandle object, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetDirection)(RSDK_ObjectHandle object, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetBounds)(RSDK_ObjectHandle object, RSDK_Vec3* outMin, RSDK_Vec3* outMax);
    float(RSDK_CALL* GetBoundingRadius)(RSDK_ObjectHandle object);

    float(RSDK_CALL* GetHealth)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMaxHealth)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetHealthPercent)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetHealthRegenRate)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMana)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMaxMana)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetManaPercent)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetSpecialPoint)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetManaSpell)(RSDK_ObjectHandle object, int32_t slot);

    float(RSDK_CALL* GetAttackRange)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetAttackSpeed)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMovementSpeed)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetCrit)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetCritDamageBonusMultiplier)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetCritDamageMultiplier)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetArmorPenetrationPercent)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetArmorPenetrationFlat)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMagicPenetrationPercent)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMagicPenetrationFlat)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetArmor)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetBonusArmor)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMagicResist)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetBonusMagicResist)(RSDK_ObjectHandle object);

    float(RSDK_CALL* GetBaseAttackDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetBonusPhysicalDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetPercentPhysicalDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetTotalPhysicalDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMagicDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetPercentMagicDamage)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetTotalMagicDamage)(RSDK_ObjectHandle object);

    float(RSDK_CALL* GetAllShield)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetMagicalShield)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetSpecialHealth)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetCooldownReductionPercent)(RSDK_ObjectHandle object);
    uint8_t(RSDK_CALL* GetLevel)(RSDK_ObjectHandle object);
    uint32_t(RSDK_CALL* GetTargetId)(RSDK_ObjectHandle object);
    uint8_t(RSDK_CALL* GetTargetableTeamFlags)(RSDK_ObjectHandle object);

    void(RSDK_CALL* ReadObjectName)(RSDK_ObjectHandle object, RSDK_StringCallback callback, void* userData);
    void(RSDK_CALL* ReadChampionName)(RSDK_ObjectHandle object, RSDK_StringCallback callback, void* userData);
    void(RSDK_CALL* ReadBaseSkinName)(RSDK_ObjectHandle object, RSDK_StringCallback callback, void* userData);

    RSDK_Bool(RSDK_CALL* IsEnemy)(RSDK_ObjectHandle object, RSDK_ObjectHandle other);
    RSDK_Bool(RSDK_CALL* IsInvulnerable)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsTargetable)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsMelee)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsRanged)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsTeleport)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsDead)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsDeleted)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsMinion)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsCreep)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsHero)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsTurret)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsNexus)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsInhibitor)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsMissile)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsVisible)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsOnScreen)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsMoving)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsMe)(RSDK_ObjectHandle object);

    uint32_t(RSDK_CALL* GetObjectType)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsObjectType)(RSDK_ObjectHandle object, uint32_t type);

    float(RSDK_CALL* GetAttackDelay)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetAttackCastDelay)(RSDK_ObjectHandle object);
    uint32_t(RSDK_CALL* GetSpellState)(RSDK_ObjectHandle object, int8_t slot);
    RSDK_Bool(RSDK_CALL* CanUseSpell)(RSDK_ObjectHandle object, int8_t slot);
    RSDK_Bool(RSDK_CALL* Cast)(RSDK_ObjectHandle object, int8_t slot);
    RSDK_Bool(RSDK_CALL* CastPosition)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* CastFromTo)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 startPos, RSDK_Vec3 endPos);
    RSDK_Bool(RSDK_CALL* CastTarget)(RSDK_ObjectHandle object, int8_t slot, RSDK_ObjectHandle target);
    RSDK_Bool(RSDK_CALL* ReleaseCharge)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 position, RSDK_Bool flag);

    void(RSDK_CALL* GetAllSpells)(RSDK_ObjectHandle object, RSDK_SpellDataListCallback callback, void* userData);
    RSDK_SpellDataHandle(RSDK_CALL* GetSpellData)(RSDK_ObjectHandle object, int8_t slot);
    RSDK_SpellCastHandle(RSDK_CALL* GetActiveSpellCast)(RSDK_ObjectHandle object);
    RSDK_SpellCastHandle(RSDK_CALL* GetBasicAttack)(RSDK_ObjectHandle object);
    int8_t(RSDK_CALL* GetSpellSlot)(RSDK_ObjectHandle object, const char* spellName);
    void(RSDK_CALL* ReadSpellName)(RSDK_ObjectHandle object, int8_t slot, RSDK_StringCallback callback, void* userData);

    void(RSDK_CALL* GetAllPerks)(RSDK_ObjectHandle object, RSDK_PerkListCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* HasPerkId)(RSDK_ObjectHandle object, uint32_t perkId);
    RSDK_Bool(RSDK_CALL* HasPerkName)(RSDK_ObjectHandle object, const char* perkName);

    void(RSDK_CALL* GetAllBuffs)(RSDK_ObjectHandle object, RSDK_BuffListCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* HasBuffType)(RSDK_ObjectHandle object, int8_t buffType);
    RSDK_Bool(RSDK_CALL* HasBuffName)(RSDK_ObjectHandle object, const char* buffName);
    RSDK_BuffHandle(RSDK_CALL* GetFirstBuffByName)(RSDK_ObjectHandle object, const char* buffName);
    RSDK_BuffHandle(RSDK_CALL* GetFirstBuffByType)(RSDK_ObjectHandle object, int8_t buffType);
    void(RSDK_CALL* GetAllBuffsByName)(RSDK_ObjectHandle object, const char* buffName, RSDK_BuffListCallback callback, void* userData);
    void(RSDK_CALL* GetAllBuffsByType)(RSDK_ObjectHandle object, int8_t buffType, RSDK_BuffListCallback callback, void* userData);
    void(RSDK_CALL* ForEachBuff)(RSDK_ObjectHandle object, RSDK_BuffVisitorCallback callback, void* userData);

    void(RSDK_CALL* GetAllItems)(RSDK_ObjectHandle object, RSDK_ItemListCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* HasItemId)(RSDK_ObjectHandle object, uint32_t itemId);
    RSDK_Bool(RSDK_CALL* HasItemName)(RSDK_ObjectHandle object, const char* itemName);
    RSDK_ItemHandle(RSDK_CALL* FindItemById)(RSDK_ObjectHandle object, uint32_t itemId, uint8_t* outSlot);
    RSDK_ItemHandle(RSDK_CALL* FindItemByName)(RSDK_ObjectHandle object, const char* itemName, uint8_t* outSlot);
    void(RSDK_CALL* ForEachItem)(RSDK_ObjectHandle object, RSDK_ItemVisitorCallback callback, void* userData);

    RSDK_Bool(RSDK_CALL* GetNavPath)(RSDK_ObjectHandle object, RSDK_NavPath* output);
    RSDK_Bool(RSDK_CALL* IsDashing)(RSDK_ObjectHandle object);
    void(RSDK_CALL* GetPath)(RSDK_ObjectHandle object, RSDK_Vec3 toPos, RSDK_Bool smoothPath, RSDK_Vec3ListCallback callback, void* userData);
    void(RSDK_CALL* GetPathFromTo)(RSDK_ObjectHandle object, RSDK_Vec3 fromPos, RSDK_Vec3 toPos, RSDK_Bool smoothPath, RSDK_Vec3ListCallback callback, void* userData);
    void(RSDK_CALL* GetCurrentPath)(RSDK_ObjectHandle object, RSDK_Vec3ListCallback callback, void* userData);

    RSDK_Bool(RSDK_CALL* IssueOrderPosition)(RSDK_ObjectHandle object, uint8_t order, RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* IssueOrderTarget)(RSDK_ObjectHandle object, uint8_t order, RSDK_ObjectHandle target);
    float(RSDK_CALL* GetBaseHealthForLevel)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetRespawnTimeRemaining)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* GetHealthBarScreenPosition)(RSDK_ObjectHandle object, RSDK_Vec2* output);

    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_ObjectHandle object);

    /*
     * Extended cast entry points. triggerEvent=RSDK_FALSE bypasses RSDK OnCastSpell /
     * OnUpdateChargeableSpell callbacks while still invoking the native game function.
     * The legacy Cast* functions above always trigger events.
     */
    RSDK_Bool(RSDK_CALL* CastEx)(RSDK_ObjectHandle object, int8_t slot, RSDK_Bool triggerEvent);
    RSDK_Bool(RSDK_CALL* CastPositionEx)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 position, RSDK_Bool triggerEvent);
    RSDK_Bool(RSDK_CALL* CastFromToEx)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 startPos, RSDK_Vec3 endPos, RSDK_Bool triggerEvent);
    RSDK_Bool(RSDK_CALL* CastTargetEx)(RSDK_ObjectHandle object, int8_t slot, RSDK_ObjectHandle target, RSDK_Bool triggerEvent);
    RSDK_Bool(RSDK_CALL* ReleaseChargeEx)(RSDK_ObjectHandle object, int8_t slot, RSDK_Vec3 position, RSDK_Bool flag, RSDK_Bool triggerEvent);

    /* v5.2.2 append-only Object extensions */
    RSDK_Bool(RSDK_CALL* DoEmote)(RSDK_ObjectHandle object, uint8_t emoteId);
    RSDK_Bool(RSDK_CALL* IsZombie)(RSDK_ObjectHandle object);
    RSDK_Bool(RSDK_CALL* IsClone)(RSDK_ObjectHandle object);
    float(RSDK_CALL* GetPhysicalShield)(RSDK_ObjectHandle object);
    // Optional 5.2.2_fix extension; check structSize before accessing on older Core.
    // Borrowed native pointer: copy the state immediately if it must be retained.
    const char*(RSDK_CALL* GetTeleportState)(RSDK_ObjectHandle object);
} RSDK_ObjectApi;

/*
 * Object-list functions publish one synchronous snapshot per API call.
 * Returned handles are non-owning native game addresses and should be used immediately.
 */
typedef struct RSDK_ObjectManager
{
    uint32_t structSize;
    RSDK_ObjectHandle(RSDK_CALL* GetLocalPlayer)(void);
    RSDK_ObjectHandle(RSDK_CALL* FindObjectByNetworkId)(uint32_t networkId);
    RSDK_ObjectHandle(RSDK_CALL* FindObjectById)(uint32_t id);
    RSDK_MissileHandle(RSDK_CALL* FindMissileByNetworkId)(uint32_t networkId);
    RSDK_MissileHandle(RSDK_CALL* FindMissileById)(uint32_t id);

    void(RSDK_CALL* GetAllHeroes)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllAttackableUnits)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllMinions)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllTurrets)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllBuildingObjects)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllObjects)(RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllInhibitors)(uint8_t filter, RSDK_ObjectListCallback callback, void* userData);
    void(RSDK_CALL* GetAllMissiles)(uint8_t filter, RSDK_MissileListCallback callback, void* userData);

    RSDK_ObjectHandle(RSDK_CALL* GetEnemyNexus)(void);
    RSDK_ObjectHandle(RSDK_CALL* GetTeamNexus)(void);
} RSDK_ObjectManager;


typedef struct RSDK_PerkApi
{
    uint32_t structSize;
    uint32_t(RSDK_CALL* GetId)(RSDK_PerkHandle perk);
    void(RSDK_CALL* ReadName)(RSDK_PerkHandle perk, RSDK_StringCallback callback, void* userData);
    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_PerkHandle perk);
} RSDK_PerkApi;

typedef struct RSDK_BuffApi
{
    uint32_t structSize;
    int8_t(RSDK_CALL* GetType)(RSDK_BuffHandle buff);
    float(RSDK_CALL* GetBeginTime)(RSDK_BuffHandle buff);
    float(RSDK_CALL* GetEndTime)(RSDK_BuffHandle buff);
    float(RSDK_CALL* GetDuration)(RSDK_BuffHandle buff);
    float(RSDK_CALL* GetRemainingTime)(RSDK_BuffHandle buff);
    uint32_t(RSDK_CALL* GetStacks)(RSDK_BuffHandle buff);
    uint32_t(RSDK_CALL* GetCount)(RSDK_BuffHandle buff);
    RSDK_Bool(RSDK_CALL* IsActive)(RSDK_BuffHandle buff);
    RSDK_Bool(RSDK_CALL* IsValid)(RSDK_BuffHandle buff);
    void(RSDK_CALL* ReadName)(RSDK_BuffHandle buff, RSDK_StringCallback callback, void* userData);
    RSDK_ObjectHandle(RSDK_CALL* GetCaster)(RSDK_BuffHandle buff);
    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_BuffHandle buff);
} RSDK_BuffApi;

typedef struct RSDK_ItemApi
{
    uint32_t structSize;
    uint32_t(RSDK_CALL* GetId)(RSDK_ItemHandle item);
    uint32_t(RSDK_CALL* GetStacks)(RSDK_ItemHandle item);
    uint32_t(RSDK_CALL* GetCharges)(RSDK_ItemHandle item);
    void(RSDK_CALL* ReadName)(RSDK_ItemHandle item, RSDK_StringCallback callback, void* userData);
} RSDK_ItemApi;

typedef struct RSDK_MissileApi
{
    uint32_t structSize;
    uint32_t(RSDK_CALL* GetId)(RSDK_MissileHandle missile);
    uint32_t(RSDK_CALL* GetNetworkId)(RSDK_MissileHandle missile);
    int32_t(RSDK_CALL* GetTeam)(RSDK_MissileHandle missile);
    RSDK_Bool(RSDK_CALL* GetPosition)(RSDK_MissileHandle missile, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetStartPosition)(RSDK_MissileHandle missile, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetEndPosition)(RSDK_MissileHandle missile, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetCastEndPosition)(RSDK_MissileHandle missile, RSDK_Vec3* output);
    float(RSDK_CALL* GetSpawnTime)(RSDK_MissileHandle missile);
    RSDK_ObjectHandle(RSDK_CALL* GetCaster)(RSDK_MissileHandle missile);
    RSDK_ObjectHandle(RSDK_CALL* GetTarget)(RSDK_MissileHandle missile);
    void(RSDK_CALL* ReadName)(RSDK_MissileHandle missile, RSDK_StringCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* IsDeleted)(RSDK_MissileHandle missile);
    RSDK_Bool(RSDK_CALL* IsMissile)(RSDK_MissileHandle missile);
    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_MissileHandle missile);
} RSDK_MissileApi;

typedef struct RSDK_SpellDataApi
{
    uint32_t structSize;
    uint8_t(RSDK_CALL* GetLevel)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetCooldownExpires)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetCooldownExpires2)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetCooldownTime)(RSDK_SpellDataHandle spellData);
    uint8_t(RSDK_CALL* GetAmmo)(RSDK_SpellDataHandle spellData);
    uint8_t(RSDK_CALL* GetToggleState)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetProjectileSpeed)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetRange)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetRadius)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetWidth)(RSDK_SpellDataHandle spellData);
    float(RSDK_CALL* GetBaseCooldownTime)(RSDK_SpellDataHandle spellData);
    void(RSDK_CALL* ReadSpellName)(RSDK_SpellDataHandle spellData, RSDK_StringCallback callback, void* userData);
    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_SpellDataHandle spellData);
} RSDK_SpellDataApi;

typedef struct RSDK_SpellCastApi
{
    uint32_t structSize;
    RSDK_ObjectHandle(RSDK_CALL* GetCaster)(RSDK_SpellCastHandle spellCast);
    uint32_t(RSDK_CALL* GetMissileNetworkId)(RSDK_SpellCastHandle spellCast);
    RSDK_ObjectHandle(RSDK_CALL* GetTarget)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetAttackCastDelay)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetAttackDelay)(RSDK_SpellCastHandle spellCast);
    uint8_t(RSDK_CALL* GetSlot)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetCastTime)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetCastEndTime)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetCooldownExpires)(RSDK_SpellCastHandle spellCast);
    RSDK_Bool(RSDK_CALL* GetStartPosition)(RSDK_SpellCastHandle spellCast, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetEndPosition)(RSDK_SpellCastHandle spellCast, RSDK_Vec3* output);
    RSDK_Bool(RSDK_CALL* GetCastEndPosition)(RSDK_SpellCastHandle spellCast, RSDK_Vec3* output);
    void(RSDK_CALL* ReadSpellName)(RSDK_SpellCastHandle spellCast, RSDK_StringCallback callback, void* userData);
    uintptr_t(RSDK_CALL* GetRawAddress)(RSDK_SpellCastHandle spellCast);
    RSDK_Bool(RSDK_CALL* IsAutoAttack)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetProjectileSpeed)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetRange)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetRadius)(RSDK_SpellCastHandle spellCast);
    float(RSDK_CALL* GetWidth)(RSDK_SpellCastHandle spellCast);
} RSDK_SpellCastApi;

typedef struct RSDK_GameApi
{
    uint32_t structSize;
    float(RSDK_CALL* GetTime)(void);
    int32_t(RSDK_CALL* GetLatency)(void);
    void(RSDK_CALL* PrintChat)(const char* text);
    RSDK_Bool(RSDK_CALL* GetCursorPosition)(RSDK_Vec3* output);
    int32_t(RSDK_CALL* GetState)(void);
    RSDK_Bool(RSDK_CALL* IsReady)(void);
    RSDK_Bool(RSDK_CALL* IsFocused)(void);
    RSDK_Bool(RSDK_CALL* IsChatOpen)(void);
    uint16_t(RSDK_CALL* GetCollisionFlag)(RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* IsWall)(RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* IsGrass)(RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* IsInFow)(RSDK_Vec3 position);
    RSDK_Bool(RSDK_CALL* WorldToScreen)(RSDK_Vec3 position, RSDK_Vec2* output);
    RSDK_Bool(RSDK_CALL* WorldToMinimap)(RSDK_Vec3 position, RSDK_Vec2* output);
} RSDK_GameApi;

typedef struct RSDK_LogApi
{
    uint32_t structSize;
    void(RSDK_CALL* Write)(RSDK_LogLevel level, const char* format, ...);
    void(RSDK_CALL* Trace)(const char* format, ...);
    void(RSDK_CALL* Info)(const char* format, ...);
    void(RSDK_CALL* Warning)(const char* format, ...);
    void(RSDK_CALL* Error)(const char* format, ...);
    RSDK_Bool(RSDK_CALL* Clear)(void);
} RSDK_LogApi;

typedef struct RSDK_RenderApi
{
    uint32_t structSize;
    RSDK_Bool(RSDK_CALL* IsReady)(void);
    void(RSDK_CALL* DrawText)(RSDK_Vec2 position, const char* text, RSDK_Color color, float fontSize);
    void(RSDK_CALL* DrawLine2D)(RSDK_Vec2 from, RSDK_Vec2 to, RSDK_Color color, float thickness);
    void(RSDK_CALL* DrawCircle2D)(RSDK_Vec2 center, float radius, RSDK_Color color, float thickness, uint32_t segments);
    void(RSDK_CALL* DrawRect)(RSDK_Vec2 position, RSDK_Vec2 size, RSDK_Color color, float thickness, float rounding);
    void(RSDK_CALL* DrawFilledRect)(RSDK_Vec2 position, RSDK_Vec2 size, RSDK_Color color, float rounding);
    void(RSDK_CALL* DrawLine3D)(RSDK_Vec3 from, RSDK_Vec3 to, RSDK_Color color, float thickness);
    void(RSDK_CALL* DrawCircle3D)(RSDK_Vec3 center, float radius, RSDK_Color color, float thickness, uint32_t segments);
    RSDK_TextureHandle(RSDK_CALL* LoadTextureFromFile)(const char* filePath);
    RSDK_TextureHandle(RSDK_CALL* LoadTextureFromMemory)(const void* data, uint32_t size);
    void(RSDK_CALL* ReleaseTexture)(RSDK_TextureHandle texture);
    void(RSDK_CALL* DrawImage)(RSDK_TextureHandle texture, RSDK_Vec2 position, RSDK_Vec2 size, uint8_t shape, RSDK_Color tint, RSDK_Color borderColor, float borderThickness, float rounding);

    /* v4.7 world-circle helpers. Existing DrawCircle3D remains the unclipped compatibility primitive. */
    void(RSDK_CALL* DrawCircle3DEx)(RSDK_Vec3 center, float radius, RSDK_Color color, float thickness, uint32_t segments, RSDK_Bool clipTerrain);
    void(RSDK_CALL* DrawCircleGlow3D)(RSDK_Vec3 center, float radius, RSDK_Color color, float thickness, float glowSize, uint32_t glowLayers, uint32_t segments, RSDK_Bool clipTerrain);
    void(RSDK_CALL* DrawRainbowCircle3D)(RSDK_Vec3 center, float radius, float thickness, uint32_t segments, float speed, RSDK_Bool clockwise, uint8_t alpha, RSDK_Bool clipTerrain);
} RSDK_RenderApi;

typedef struct RSDK_ConfigApi
{
    uint32_t structSize;
    void(RSDK_CALL* SetUseRawPointer)(RSDK_Bool enabled);
    RSDK_Bool(RSDK_CALL* GetUseRawPointer)(void);
} RSDK_ConfigApi;

typedef struct RSDK_LocalizationApi
{
    uint32_t structSize;
    void(RSDK_CALL* ReadText)(const char* key, const char* fallback, RSDK_StringCallback callback, void* userData);
    void(RSDK_CALL* ReadLocale)(RSDK_StringCallback callback, void* userData);
    RSDK_Bool(RSDK_CALL* SetLocale)(const char* locale);
    RSDK_Bool(RSDK_CALL* Reload)(void);
} RSDK_LocalizationApi;

typedef struct RSDK_Api
{
    uint32_t structSize;
    uint32_t abiVersion;
    const RSDK_EventApi* events;
    const RSDK_ObjectApi* objects;
    const RSDK_ObjectManager* objectManager;
    const RSDK_GameApi* game;
    const RSDK_PerkApi* perks;
    const RSDK_BuffApi* buffs;
    const RSDK_ItemApi* items;
    const RSDK_MissileApi* missiles;
    const RSDK_SpellDataApi* spellData;
    const RSDK_SpellCastApi* spellCasts;
    const RSDK_LogApi* log;
    const RSDK_RenderApi* render;
    const RSDK_ConfigApi* config;
    const RSDK_LocalizationApi* localization;
    const RSDK_MenuApi* menu;
    const RSDK_TargetSelectorApi* targetSelector;
    const RSDK_OrbwalkerApi* orbwalker;
    const RSDK_PredictionApi* prediction;
    const RSDK_DamageApi* damage;
} RSDK_Api;

RSDK_API const RSDK_Api* RSDK_CALL RSDK_GetApi(uint32_t requestedVersion);
RSDK_API uint32_t RSDK_CALL RSDK_GetApiVersion(void);
RSDK_API RSDK_Bool RSDK_CALL RSDK_IsInitialized(void);
RSDK_API void RSDK_CALL RSDK_RequestShutdown(void);
RSDK_API void RSDK_CALL RSDK_LogWrite(RSDK_LogLevel level, const char* format, ...);
RSDK_API void RSDK_CALL RSDK_LogTrace(const char* format, ...);
RSDK_API void RSDK_CALL RSDK_LogInfo(const char* format, ...);
RSDK_API void RSDK_CALL RSDK_LogWarning(const char* format, ...);
RSDK_API void RSDK_CALL RSDK_LogError(const char* format, ...);
RSDK_API RSDK_Bool RSDK_CALL RSDK_LogClear(void);

