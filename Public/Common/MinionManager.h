#pragma once

#include "../RSDK.h"
#include "../Geometry.h"
#include "Prediction.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace rsdk::MinionManager
{
    struct FarmLocation final
    {
        Vec3 Position{};
        int MinionsHit{};
    };

    // LeagueSharp.Common/MinionManager.IsWard() also checks IsHPBarRendered.
    // RSDK intentionally uses the current targetability/name checks and does not require that extra field.
    [[nodiscard]] inline bool IsWard(const Object& minion)
    {
        if (!minion || minion.TargetableTeamFlags() != 4 || !minion.IsTargetable())
            return false;

        // Name matching below is the accepted RSDK ward classification for the current public API.

            // WardCorpse   - S5Test_WardCorpse
            // SightWard    - YellowTrinket
            // SightWard    - BlueTrinket
            // JammerDevice - JammerDevice

        std::string name = minion.Name();
        return (name.find("Ward") != std::string::npos || name.find("JammerDevice") != std::string::npos);
    }

    // Narrow LeagueSharp-style minion classification used by Orbwalker priorities.
    // This is intentionally distinct from RSDK Object::IsMinion(), whose core classification
    // is broader and includes lane minions, jungle monsters, wards, pets, traps, etc.
    [[nodiscard]] inline bool IsMinion(const Object& minion, const bool includeWards = false)
    {
        if (!minion)
            return false;

        const bool laneMinion = minion.Name().find("Minion") != std::string::npos;
        return laneMinion || (includeWards && IsWard(minion));
    }

    [[nodiscard]] inline std::vector<Vec3> GetMinionsPredictedPositions(
        const std::vector<Object>& minions,
        const float delay,
        const float width,
        const float speed,
        Vec3 from,
        const float range,
        const bool collision,
        const SkillshotType type,
        Vec3 rangeCheckFrom = {})
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!from.IsValid() && player)
            from = player.ServerPos().IsValid() ? player.ServerPos() : player.Position();
        if (!rangeCheckFrom.IsValid())
            rangeCheckFrom = from;

        std::vector<Vec3> result{};
        result.reserve(minions.size());

        for (const Object& minion : minions)
        {
            if (!minion)
                continue;

            PredictionInput input{};
            input.Unit = minion;
            input.Delay = delay;
            input.Radius = width;
            input.Speed = speed;
            input.From = from;
            input.Range = range;
            input.Collision = collision;
            input.Type = type;
            input.RangeCheckFrom = rangeCheckFrom;

            const PredictionOutput prediction = Prediction::GetPrediction(input);
            if (static_cast<std::uint8_t>(prediction.Hitchance) >= static_cast<std::uint8_t>(HitChance::High))
                result.push_back(prediction.UnitPosition);
        }

        return result;
    }

    [[nodiscard]] inline FarmLocation GetBestCircularFarmLocation(
        const std::vector<Vec3>& minionPositions,
        const float width,
        const float range,
        Vec3 from,
        const int useMecMax = 9)
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!from.IsValid() && player)
            from = player.ServerPos().IsValid() ? player.ServerPos() : player.Position();

        FarmLocation result{};
        if (minionPositions.empty() || !from.IsValid() || width < 0.0f || range < 0.0f)
            return result;

        std::vector<Vec2> points{};
        points.reserve(minionPositions.size());
        for (const Vec3& position : minionPositions)
            points.push_back(position.To2D());

        const Vec2 start = from.To2D();
        const float rangeSquared = range * range;

        // LeagueSharp uses MEC for small sets. Enumerate subsets by descending size so the
        // first valid circle is guaranteed to hit the maximum number of minions.
        if (points.size() <= static_cast<std::size_t>(std::max(useMecMax, 0)) && points.size() < 31u)
        {
            const std::uint32_t combinationCount = 1u << static_cast<std::uint32_t>(points.size());
            const auto bitCount = [](std::uint32_t value) noexcept
            {
                std::size_t count{};
                while (value != 0u)
                {
                    value &= value - 1u;
                    ++count;
                }
                return count;
            };

            for (std::size_t wanted = points.size(); wanted > 0u; --wanted)
            {
                for (std::uint32_t mask = 1u; mask < combinationCount; ++mask)
                {
                    if (bitCount(mask) != wanted)
                        continue;

                    std::vector<Vec2> subgroup{};
                    subgroup.reserve(wanted);
                    for (std::size_t index = 0; index < points.size(); ++index)
                    {
                        if ((mask & (1u << static_cast<std::uint32_t>(index))) != 0u)
                            subgroup.push_back(points[index]);
                    }

                    const geometry::Circle2D circle = geometry::MinimumEnclosingCircle(subgroup);
                    if (circle.Valid && circle.Radius <= width && geometry::DistanceSquared(circle.Center, start) <= rangeSquared)
                    {
                        result.Position = Vec3(circle.Center.x, from.y, circle.Center.y);
                        result.MinionsHit = static_cast<int>(wanted);
                        return result;
                    }
                }
            }
        }
        else
        {
            const float widthSquared = width * width;
            for (const Vec2& position : points)
            {
                if (geometry::DistanceSquared(position, start) > rangeSquared)
                    continue;

                int count = 0;
                for (const Vec2& other : points)
                {
                    if (geometry::DistanceSquared(position, other) <= widthSquared)
                        ++count;
                }

                if (count >= result.MinionsHit)
                {
                    result.Position = Vec3(position.x, from.y, position.y);
                    result.MinionsHit = count;
                }
            }
        }

        return result;
    }

    [[nodiscard]] inline FarmLocation GetBestCircularFarmLocation(
        const std::vector<Vec3>& minionPositions,
        const float width,
        const float range,
        const int useMecMax = 9)
    {
        const Object player = ObjectManager::LocalPlayer();
        return GetBestCircularFarmLocation(minionPositions, width, range, player ? (player.ServerPos().IsValid() ? player.ServerPos() : player.Position()) : Vec3{}, useMecMax);
    }

    [[nodiscard]] inline FarmLocation GetBestLineFarmLocation(
        const std::vector<Vec3>& minionPositions,
        const float width,
        const float range,
        Vec3 from)
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!from.IsValid() && player)
            from = player.ServerPos().IsValid() ? player.ServerPos() : player.Position();

        FarmLocation result{};
        if (minionPositions.empty() || !from.IsValid() || width < 0.0f || range < 0.0f)
            return result;

        std::vector<Vec2> positions{};
        positions.reserve(minionPositions.size());
        for (const Vec3& position : minionPositions)
            positions.push_back(position.To2D());

        std::vector<Vec2> possiblePositions = positions;
        possiblePositions.reserve(positions.size() + (positions.size() * (positions.size() - 1u)) / 2u);
        for (std::size_t i = 0; i < positions.size(); ++i)
        {
            for (std::size_t j = i + 1u; j < positions.size(); ++j)
            {
                if (positions[j] != positions[i])
                    possiblePositions.push_back((positions[j] + positions[i]) * 0.5f);
            }
        }

        const Vec2 start = from.To2D();
        const float rangeSquared = range * range;
        const float widthSquared = width * width;

        for (const Vec2& position : possiblePositions)
        {
            if (geometry::DistanceSquared(position, start) > rangeSquared)
                continue;

            const Vec2 direction = (position - start).Normalized();
            if (!direction.IsValid())
                continue;

            const Vec2 end = start + direction * range;
            int count = 0;
            for (const Vec2& minion : positions)
            {
                const Vec2 closest = geometry::ClosestPointOnSegment(minion, start, end);
                if (geometry::DistanceSquared(minion, closest) <= widthSquared)
                    ++count;
            }

            if (count >= result.MinionsHit)
            {
                result.Position = Vec3(end.x, from.y, end.y);
                result.MinionsHit = count;
            }
        }

        return result;
    }

    [[nodiscard]] inline FarmLocation GetBestLineFarmLocation(
        const std::vector<Vec3>& minionPositions,
        const float width,
        const float range)
    {
        const Object player = ObjectManager::LocalPlayer();
        return GetBestLineFarmLocation(minionPositions, width, range, player ? player.Position() : Vec3{});
    }
}
