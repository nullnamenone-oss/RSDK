#pragma once

#include "RSDKTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSDK_PREDICTION_API_VERSION 2u
#define RSDK_PREDICTION_MAX_OBJECTS 32u

typedef enum RSDK_HitChance
{
    RSDK_HITCHANCE_COLLISION = 0,
    RSDK_HITCHANCE_OUT_OF_RANGE = 1,
    RSDK_HITCHANCE_IMPOSSIBLE = 2,
    RSDK_HITCHANCE_LOW = 3,
    RSDK_HITCHANCE_MEDIUM = 4,
    RSDK_HITCHANCE_HIGH = 5,
    RSDK_HITCHANCE_VERY_HIGH = 6,
    RSDK_HITCHANCE_DASHING = 7,
    RSDK_HITCHANCE_IMMOBILE = 8
} RSDK_HitChance;

typedef enum RSDK_SkillshotType
{
    RSDK_SKILLSHOT_LINE = 0,
    RSDK_SKILLSHOT_CIRCLE = 1,
    RSDK_SKILLSHOT_CONE = 2
} RSDK_SkillshotType;

typedef enum RSDK_CollisionableObjects
{
    RSDK_COLLISION_NONE = 0,
    RSDK_COLLISION_MINIONS = 1u << 0,
    RSDK_COLLISION_HEROES = 1u << 1,
    RSDK_COLLISION_YASUO_WALL = 1u << 2,
    RSDK_COLLISION_WALLS = 1u << 3,
    RSDK_COLLISION_ALLIES = 1u << 4
} RSDK_CollisionableObjects;

typedef struct RSDK_PredictionRequest
{
    uint32_t structSize;
    uint32_t unitNetworkId;
    float delay;
    float radius;
    float range;
    float speed;
    uint8_t type;
    RSDK_Bool useBoundingRadius;
    RSDK_Bool collision;
    RSDK_Bool aoe;
    uint32_t collisionObjects;
    RSDK_Bool hasFrom;
    RSDK_Vec3 from;
    RSDK_Bool hasRangeCheckFrom;
    RSDK_Vec3 rangeCheckFrom;
} RSDK_PredictionRequest;

typedef struct RSDK_PredictionResult
{
    uint32_t structSize;
    uint8_t hitchance;
    RSDK_Vec3 castPosition;
    RSDK_Vec3 unitPosition;
    uint32_t aoeTargetsHitCount;
    uint32_t aoeTargetNetworkIds[RSDK_PREDICTION_MAX_OBJECTS];
    uint32_t collisionObjectCount;
    uint32_t collisionObjectNetworkIds[RSDK_PREDICTION_MAX_OBJECTS];
} RSDK_PredictionResult;

typedef struct RSDK_CollisionRequest
{
    uint32_t structSize;
    uint32_t unitNetworkId;
    float delay;
    float radius;
    float range;
    float speed;
    uint8_t type;
    RSDK_Bool useBoundingRadius;
    uint32_t collisionObjects;
    RSDK_Bool hasFrom;
    RSDK_Vec3 from;
    RSDK_Bool hasRangeCheckFrom;
    RSDK_Vec3 rangeCheckFrom;
    const RSDK_Vec3* positions;
    uint32_t positionCount;
} RSDK_CollisionRequest;

typedef struct RSDK_CollisionResult
{
    uint32_t structSize;
    uint32_t collisionObjectCount;
    uint32_t collisionObjectNetworkIds[RSDK_PREDICTION_MAX_OBJECTS];
} RSDK_CollisionResult;

typedef struct RSDK_PredictionProviderApi
{
    uint32_t structSize;
    uint32_t apiVersion;
    RSDK_Bool(RSDK_CALL* GetPrediction)(const RSDK_PredictionRequest* request, RSDK_PredictionResult* result);
    RSDK_Bool(RSDK_CALL* GetCollision)(const RSDK_CollisionRequest* request, RSDK_CollisionResult* result);
} RSDK_PredictionProviderApi;

typedef struct RSDK_PredictionApi
{
    uint32_t structSize;
    uint32_t apiVersion;
    RSDK_Bool(RSDK_CALL* IsAvailable)(void);
    RSDK_Bool(RSDK_CALL* GetPrediction)(const RSDK_PredictionRequest* request, RSDK_PredictionResult* result);
    RSDK_Bool(RSDK_CALL* GetCollision)(const RSDK_CollisionRequest* request, RSDK_CollisionResult* result);
    RSDK_Bool(RSDK_CALL* RegisterProvider)(const RSDK_PredictionProviderApi* provider);
    RSDK_Bool(RSDK_CALL* UnregisterProvider)(const RSDK_PredictionProviderApi* provider);
} RSDK_PredictionApi;

#ifdef __cplusplus
}
#endif
