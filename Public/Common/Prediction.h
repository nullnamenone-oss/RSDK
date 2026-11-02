#pragma once

#include "../RSDK.h"
#include "HeroManager.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace rsdk
{
    enum class HitChance : std::uint8_t
    {
        Collision = RSDK_HITCHANCE_COLLISION,
        OutOfRange = RSDK_HITCHANCE_OUT_OF_RANGE,
        Impossible = RSDK_HITCHANCE_IMPOSSIBLE,
        Low = RSDK_HITCHANCE_LOW,
        Medium = RSDK_HITCHANCE_MEDIUM,
        High = RSDK_HITCHANCE_HIGH,
        VeryHigh = RSDK_HITCHANCE_VERY_HIGH,
        Dashing = RSDK_HITCHANCE_DASHING,
        Immobile = RSDK_HITCHANCE_IMMOBILE
    };

    enum class SkillshotType : std::uint8_t
    {
        Line = RSDK_SKILLSHOT_LINE,
        Circle = RSDK_SKILLSHOT_CIRCLE,
        Cone = RSDK_SKILLSHOT_CONE
    };

    enum class CollisionableObjects : std::uint32_t
    {
        None = RSDK_COLLISION_NONE,
        Minions = RSDK_COLLISION_MINIONS,
        Heroes = RSDK_COLLISION_HEROES,
        YasuoWall = RSDK_COLLISION_YASUO_WALL,
        Walls = RSDK_COLLISION_WALLS,
        Allies = RSDK_COLLISION_ALLIES
    };

    [[nodiscard]] constexpr CollisionableObjects operator|(const CollisionableObjects a, const CollisionableObjects b) noexcept
    {
        return static_cast<CollisionableObjects>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    [[nodiscard]] constexpr CollisionableObjects operator&(const CollisionableObjects a, const CollisionableObjects b) noexcept
    {
        return static_cast<CollisionableObjects>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    struct PredictionInput final
    {
        Object Unit{};
        bool Aoe{false};
        bool Collision{false};
        CollisionableObjects CollisionObjects{CollisionableObjects::Minions | CollisionableObjects::YasuoWall};
        float Delay{};
        float Radius{1.0f};
        float Range{std::numeric_limits<float>::max()};
        float Speed{std::numeric_limits<float>::max()};
        SkillshotType Type{SkillshotType::Line};
        bool UseBoundingRadius{true};
        Vec3 From{};
        Vec3 RangeCheckFrom{};

        [[nodiscard]] float RealRadius() const noexcept
        {
            return UseBoundingRadius && Unit ? Radius + Unit.BoundingRadius() : Radius;
        }
    };

    struct PredictionOutput final
    {
        std::vector<Object> AoeTargetsHit{};
        std::vector<Object> CollisionObjects{};
        HitChance Hitchance{HitChance::Impossible};
        Vec3 CastPosition{};
        Vec3 UnitPosition{};
        std::uint32_t AoeTargetsHitCount{};
    };

    class Prediction final
    {
    public:
        [[nodiscard]] static bool IsAvailable() noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->IsAvailable != nullptr && api->IsAvailable() != RSDK_FALSE;
        }

        [[nodiscard]] static PredictionOutput GetPrediction(const Object& unit, const float delay)
        {
            PredictionInput input{};
            input.Unit = unit;
            input.Delay = delay;
            return GetPrediction(input);
        }

        [[nodiscard]] static PredictionOutput GetPrediction(const Object& unit, const float delay, const float radius)
        {
            PredictionInput input{};
            input.Unit = unit;
            input.Delay = delay;
            input.Radius = radius;
            return GetPrediction(input);
        }

        [[nodiscard]] static PredictionOutput GetPrediction(const Object& unit, const float delay, const float radius, const float speed)
        {
            PredictionInput input{};
            input.Unit = unit;
            input.Delay = delay;
            input.Radius = radius;
            input.Speed = speed;
            return GetPrediction(input);
        }

        [[nodiscard]] static PredictionOutput GetPrediction(const PredictionInput& input)
        {
            PredictionOutput output{};
            const auto* const api = Api();
            if (!input.Unit || api == nullptr || api->GetPrediction == nullptr)
                return output;

            RSDK_PredictionRequest request{};
            request.structSize = sizeof(request);
            request.unitNetworkId = input.Unit.NetworkId();
            request.delay = input.Delay;
            request.radius = input.Radius;
            request.range = input.Range;
            request.speed = input.Speed;
            request.type = static_cast<std::uint8_t>(input.Type);
            request.useBoundingRadius = input.UseBoundingRadius ? RSDK_TRUE : RSDK_FALSE;
            request.collision = input.Collision ? RSDK_TRUE : RSDK_FALSE;
            request.aoe = input.Aoe ? RSDK_TRUE : RSDK_FALSE;
            request.collisionObjects = static_cast<std::uint32_t>(input.CollisionObjects);
            request.hasFrom = input.From.IsValid() ? RSDK_TRUE : RSDK_FALSE;
            request.from = input.From;
            request.hasRangeCheckFrom = input.RangeCheckFrom.IsValid() ? RSDK_TRUE : RSDK_FALSE;
            request.rangeCheckFrom = input.RangeCheckFrom;

            RSDK_PredictionResult result{};
            result.structSize = sizeof(result);
            if (api->GetPrediction(&request, &result) == RSDK_FALSE)
                return output;

            output.Hitchance = static_cast<HitChance>(std::clamp<std::uint8_t>(result.hitchance, 0u, 8u));
            output.CastPosition = Vec3(result.castPosition);
            output.UnitPosition = Vec3(result.unitPosition);
            output.AoeTargetsHitCount = result.aoeTargetsHitCount;

            const std::uint32_t aoeCount = std::min(result.aoeTargetsHitCount, RSDK_PREDICTION_MAX_OBJECTS);
            output.AoeTargetsHit.reserve(aoeCount);
            for (std::uint32_t i = 0; i < aoeCount; ++i)
            {
                const Object object = ObjectManager::FindObjectByNetworkId(result.aoeTargetNetworkIds[i]);
                if (object)
                    output.AoeTargetsHit.push_back(object);
            }

            const std::uint32_t collisionCount = std::min(result.collisionObjectCount, RSDK_PREDICTION_MAX_OBJECTS);
            output.CollisionObjects.reserve(collisionCount);
            for (std::uint32_t i = 0; i < collisionCount; ++i)
            {
                const Object object = ObjectManager::FindObjectByNetworkId(result.collisionObjectNetworkIds[i]);
                if (object)
                    output.CollisionObjects.push_back(object);
            }
            return output;
        }


        [[nodiscard]] static std::vector<Object> GetCollision(const std::vector<Vec3>& positions, const PredictionInput& input)
        {
            std::vector<Object> output{};
            const auto* const api = Api();
            if (positions.empty() || api == nullptr || api->GetCollision == nullptr)
                return output;

            RSDK_CollisionRequest request{};
            request.structSize = sizeof(request);
            request.unitNetworkId = input.Unit ? input.Unit.NetworkId() : 0u;
            request.delay = input.Delay;
            request.radius = input.Radius;
            request.range = input.Range;
            request.speed = input.Speed;
            request.type = static_cast<std::uint8_t>(input.Type);
            request.useBoundingRadius = input.UseBoundingRadius ? RSDK_TRUE : RSDK_FALSE;
            request.collisionObjects = static_cast<std::uint32_t>(input.CollisionObjects);
            request.hasFrom = input.From.IsValid() ? RSDK_TRUE : RSDK_FALSE;
            request.from = input.From;
            request.hasRangeCheckFrom = input.RangeCheckFrom.IsValid() ? RSDK_TRUE : RSDK_FALSE;
            request.rangeCheckFrom = input.RangeCheckFrom;
            std::vector<RSDK_Vec3> rawPositions{};
            rawPositions.reserve(positions.size());
            for (const Vec3& position : positions)
                rawPositions.push_back(static_cast<RSDK_Vec3>(position));
            request.positions = rawPositions.data();
            request.positionCount = static_cast<std::uint32_t>(rawPositions.size());

            RSDK_CollisionResult result{};
            result.structSize = sizeof(result);
            if (api->GetCollision(&request, &result) == RSDK_FALSE)
                return output;

            const std::uint32_t count = std::min(result.collisionObjectCount, RSDK_PREDICTION_MAX_OBJECTS);
            output.reserve(count);
            for (std::uint32_t i = 0; i < count; ++i)
            {
                const Object object = ObjectManager::FindObjectByNetworkId(result.collisionObjectNetworkIds[i]);
                if (object)
                    output.push_back(object);
            }
            return output;
        }

        [[nodiscard]] static std::vector<Object> GetCollision(const Vec3& position, const PredictionInput& input)
        {
            return GetCollision(std::vector<Vec3>{position}, input);
        }

    private:
        [[nodiscard]] static const RSDK_PredictionApi* Api() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->prediction : nullptr;
        }
    };
}
