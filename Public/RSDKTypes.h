#pragma once

#include <stddef.h>
#include <stdint.h>

#include "RSDKConstants.h"

#if defined(__cplusplus)
#define RSDK_EXTERN_C extern "C"
#else
#define RSDK_EXTERN_C extern
#endif

#if defined(_WIN32)
#define RSDK_CALL __cdecl
#if defined(RSDK_BUILD)
#define RSDK_EXPORT __declspec(dllexport)
#else
#define RSDK_EXPORT __declspec(dllimport)
#endif
#if defined(RSDK_PLUGIN_BUILD)
#define RSDK_PLUGIN_EXPORT RSDK_EXTERN_C __declspec(dllexport)
#else
#define RSDK_PLUGIN_EXPORT RSDK_EXTERN_C
#endif
#else
#define RSDK_CALL
#define RSDK_EXPORT
#define RSDK_PLUGIN_EXPORT RSDK_EXTERN_C
#endif

#define RSDK_API RSDK_EXTERN_C RSDK_EXPORT

#define RSDK_ABI_VERSION 48u

typedef uint8_t RSDK_Bool;
#define RSDK_FALSE ((RSDK_Bool)0u)
#define RSDK_TRUE ((RSDK_Bool)1u)

typedef uint64_t RSDK_EventToken;
/* Raw-pointer ABI: the object handles below contain native game addresses. */
typedef uint64_t RSDK_ObjectHandle;
typedef uint64_t RSDK_PerkHandle;
typedef uint64_t RSDK_BuffHandle;
typedef uint64_t RSDK_ItemHandle;
typedef uint64_t RSDK_MissileHandle;
typedef uint64_t RSDK_SpellCastHandle;
typedef uint64_t RSDK_SpellDataHandle;
typedef uint64_t RSDK_PluginId;
typedef uint64_t RSDK_TextureHandle;
typedef void* RSDK_PluginState;

#define RSDK_INVALID_EVENT ((RSDK_EventToken)0u)
#define RSDK_INVALID_OBJECT ((RSDK_ObjectHandle)0u)
#define RSDK_INVALID_PERK ((RSDK_PerkHandle)0u)
#define RSDK_INVALID_BUFF ((RSDK_BuffHandle)0u)
#define RSDK_INVALID_ITEM ((RSDK_ItemHandle)0u)
#define RSDK_INVALID_MISSILE ((RSDK_MissileHandle)0u)
#define RSDK_INVALID_SPELL_CAST ((RSDK_SpellCastHandle)0u)
#define RSDK_INVALID_SPELL_DATA ((RSDK_SpellDataHandle)0u)
#define RSDK_INVALID_PLUGIN ((RSDK_PluginId)0u)
#define RSDK_INVALID_TEXTURE ((RSDK_TextureHandle)0u)

typedef struct RSDK_Vec2
{
    float x;
    float y;
} RSDK_Vec2;

typedef struct RSDK_Vec3
{
    float x;
    float y;
    float z;
} RSDK_Vec3;

typedef struct RSDK_Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RSDK_Color;

#if defined(__cplusplus)
#include <type_traits>
static_assert(sizeof(RSDK_Vec2) == sizeof(float) * 2u);
static_assert(sizeof(RSDK_Vec3) == sizeof(float) * 3u);
static_assert(sizeof(RSDK_Color) == sizeof(uint8_t) * 4u);
static_assert(std::is_standard_layout_v<RSDK_Vec2> && std::is_trivially_copyable_v<RSDK_Vec2>);
static_assert(std::is_standard_layout_v<RSDK_Vec3> && std::is_trivially_copyable_v<RSDK_Vec3>);
static_assert(std::is_standard_layout_v<RSDK_Color> && std::is_trivially_copyable_v<RSDK_Color>);
#endif

typedef enum RSDK_ImageShape
{
    RSDK_IMAGE_SHAPE_SQUARE = 0,
    RSDK_IMAGE_SHAPE_ROUNDED = 1,
    RSDK_IMAGE_SHAPE_CIRCLE = 2
} RSDK_ImageShape;

typedef struct RSDK_NavPath
{
    uintptr_t address;
    uintptr_t navArray;
    uint32_t navArrayCount;
    RSDK_Vec3 startPosition;
    RSDK_Vec3 endPosition;
    RSDK_Bool isDashing;
    uint8_t dashGravityCount;
    uint16_t reserved;
    float dashSpeed;
} RSDK_NavPath;

typedef struct RSDK_IssueOrderEvent
{
    RSDK_ObjectHandle sender;
    uint8_t order;
    uint8_t isAttackMove;
    uint8_t process;
    uint8_t reserved;
    RSDK_Vec3 position;
    RSDK_ObjectHandle target;
} RSDK_IssueOrderEvent;


typedef struct RSDK_NewPathEvent
{
    RSDK_ObjectHandle sender;
    const RSDK_Vec3* path;
    uint32_t pathCount;
    RSDK_Bool isDash;
    uint8_t dashGravityCount;
    uint16_t reserved;
    float dashSpeed;
    uint32_t dashGravityFlags; 
} RSDK_NewPathEvent;

typedef struct RSDK_ProcessSpellEvent
{
    RSDK_ObjectHandle sender;
    RSDK_SpellCastHandle spellCast;
} RSDK_ProcessSpellEvent;

typedef struct RSDK_DoCastSpellEvent
{
    RSDK_SpellCastHandle spellCast;
    RSDK_ObjectHandle sender;
} RSDK_DoCastSpellEvent;

typedef struct RSDK_PlayAnimationEvent
{
    RSDK_ObjectHandle sender;
    const char* animationName;
} RSDK_PlayAnimationEvent;

typedef struct RSDK_StopCastEvent
{
    RSDK_ObjectHandle sender;
    RSDK_SpellCastHandle spellCast;
    RSDK_Bool stopAnimation;
    RSDK_Bool destroyMissile;
    uint16_t reserved;
    uint32_t missileNetworkId;
} RSDK_StopCastEvent;


typedef struct RSDK_CastSpellEvent
{
    RSDK_ObjectHandle sender;
    RSDK_ObjectHandle target;
    RSDK_SpellDataHandle spellData;
    RSDK_Vec3 sourcePosition;
    RSDK_Vec3 destinationPosition;
    RSDK_Vec3 castEndPosition;
    RSDK_Vec3 unknownPosition;
    uint32_t sourceId;
    uint32_t targetId;
    int8_t slot;
    RSDK_Bool process;
    uint16_t reserved;
} RSDK_CastSpellEvent;

typedef struct RSDK_UpdateChargeableSpellEvent
{
    RSDK_ObjectHandle sender;
    RSDK_SpellDataHandle spellData;
    RSDK_Vec3 position;
    int8_t slot;
    RSDK_Bool flag;
    RSDK_Bool process;
    uint8_t reserved;
} RSDK_UpdateChargeableSpellEvent;


typedef struct RSDK_BuffAddRemoveEvent
{
    RSDK_ObjectHandle sender;
    RSDK_BuffHandle buff;
    RSDK_Bool added;
    uint8_t reserved[7];
} RSDK_BuffAddRemoveEvent;

typedef struct RSDK_UpdateBuffEvent
{
    RSDK_ObjectHandle sender;
    RSDK_BuffHandle buff;
    int32_t stacks;
} RSDK_UpdateBuffEvent;

/*
 * Fired after the native spell-impact routine returns.
 * nativeArgument is preserved because the second native argument is not fully reversed yet.
 * Internal timing/health probes are diagnostics only and are intentionally not public ABI.
 */
typedef struct RSDK_SpellImpactEvent
{
    RSDK_ObjectHandle sender;
    RSDK_ObjectHandle target;
    RSDK_SpellCastHandle spellCast;
    int32_t nativeArgument;
} RSDK_SpellImpactEvent;


/*
 * Snapshot callbacks are invoked synchronously during the API call.
 * The data pointer is read-only and remains valid only until the callback returns.
 * An empty snapshot is reported with data=nullptr and count=0.
 */
typedef void(RSDK_CALL* RSDK_StringCallback)(const char* text, uint32_t length, void* userData);
typedef void(RSDK_CALL* RSDK_Vec3ListCallback)(const RSDK_Vec3* values, uint32_t count, void* userData);
typedef void(RSDK_CALL* RSDK_ObjectListCallback)(const RSDK_ObjectHandle* values, uint32_t count, void* userData);
typedef void(RSDK_CALL* RSDK_PerkListCallback)(const RSDK_PerkHandle* values, uint32_t count, void* userData);
typedef void(RSDK_CALL* RSDK_BuffListCallback)(const RSDK_BuffHandle* values, uint32_t count, void* userData);
typedef void(RSDK_CALL* RSDK_ItemListCallback)(const RSDK_ItemHandle* values, uint32_t count, void* userData);
typedef RSDK_Bool(RSDK_CALL* RSDK_BuffVisitorCallback)(RSDK_BuffHandle buff, void* userData);
typedef RSDK_Bool(RSDK_CALL* RSDK_ItemVisitorCallback)(RSDK_ItemHandle item, uint8_t slot, uint32_t ownerNetworkId, void* userData);
typedef void(RSDK_CALL* RSDK_MissileListCallback)(const RSDK_MissileHandle* values, uint32_t count, void* userData);
typedef void(RSDK_CALL* RSDK_SpellDataListCallback)(const RSDK_SpellDataHandle* values, uint32_t count, void* userData);

