#pragma once

#include "../RSDK.h"
#include "HeroManager.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace rsdk::Utility
{
    [[nodiscard]] inline std::string ToLower(const std::string_view text)
    {
        std::string result(text);

        for (char& ch : result)
        {
            if (ch >= 'A' && ch <= 'Z')
                ch = static_cast<char>(ch - 'A' + 'a');
        }

        return result;
    }

    [[nodiscard]] inline bool EqualsIgnoreCase(const std::string_view a, const std::string_view b) noexcept
    {
        if (a.size() != b.size())
            return false;

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            char left = a[i];
            char right = b[i];

            if (left >= 'A' && left <= 'Z')
                left = static_cast<char>(left - 'A' + 'a');

            if (right >= 'A' && right <= 'Z')
                right = static_cast<char>(right - 'A' + 'a');

            if (left != right)
                return false;
        }

        return true;
    }

    // LeagueSharp.Common.Utility.IsValidTarget(this AttackableUnit unit,...).
    // LeagueSharp checks: null, IsValid, IsVisible, IsDead, IsTargetable, IsInvulnerable,
    // optional team, WardCorpse and optional range/from.
    // TODO RSDK TEST: Object currently has no public IsValid() equivalent to LeagueSharp AttackableUnit.IsValid;
    // !Object + IsDeleted() are the temporary mapping.
    [[nodiscard]] inline bool IsValidTarget(
        const Object& unit,
        const float range = std::numeric_limits<float>::max(),
        const bool checkTeam = true,
        const Vec3* const from = nullptr) noexcept
    {
        //Log::Info("  -dead: %d, deleted: %d, Invu: %d, visible: %d ", unit.IsDead(), unit.IsDeleted(), unit.IsInvulnerable(), unit.IsVisible());
        if (!unit || unit.IsDead() || unit.IsDeleted() /*||  !unit.IsTargetable()*/ || unit.IsInvulnerable())
            return false;
        
        if (!unit.IsVisible())
            return false;

        const Object player = HeroManager::Player();
        if (!player)
            return false;
        
        if (checkTeam && unit.Team() == player.Team())
            return false;
        
        if (unit.Name() == "WardCorpse")
            return false;
        
        if (!(range < std::numeric_limits<float>::max()))
            return true;

        const Vec3 playerPosition = player.ServerPos().IsValid() ? player.ServerPos() : player.Position();
        const Vec3 unitPosition   = unit.ServerPos().IsValid() ? unit.ServerPos() : unit.Position();
        const Vec3 origin = from != nullptr && from->IsValid() ? *from : playerPosition;
        return origin.To2D().DistanceToSqr(unitPosition.To2D()) <= range * range;
    }

    // LeagueSharp.Common.Utility.GetWaypoints(this Obj_AI_Base unit).
    // RSDK CurrentPath() corresponds to LeagueSharp unit.Path, not GetWaypoints().
    // For visible units, LeagueSharp prepends ServerPosition then appends Path while suppressing a duplicate first node.
    [[nodiscard]] inline std::vector<Vec3> GetWaypoints(const Object& unit)
    {
        std::vector<Vec3> result;
        if (!unit)
            return result;

        if (unit.IsVisible())
        {
            const Vec3 serverPosition = unit.ServerPos().IsValid() ? unit.ServerPos() : unit.Position();
            result.push_back(serverPosition);
            const std::vector<Vec3> path = unit.CurrentPath();
            if (!path.empty())
            {
                // LeagueSharp: if (first.Distance(result[0], true) > 40) result.Add(first);
                if (path[0].To2D().DistanceToSqr(result[0].To2D()) > 40.0f)
                    result.push_back(path[0]);

                for (std::size_t i = 1; i < path.size(); ++i)
                    result.push_back(path[i]);
            }
        }

        // LeagueSharp invisible-unit fallback, intentionally disabled until Prediction/OnNewPath tracking is added:
        // else if (WaypointTracker.StoredPaths.ContainsKey(unit.NetworkId))
        // {
        //     var path = WaypointTracker.StoredPaths[unit.NetworkId];
        //     var timePassed = (Utils.TickCount - WaypointTracker.StoredTick[unit.NetworkId]) / 1000f;
        //     if (path.PathLength() >= unit.MoveSpeed * timePassed)
        //         result = CutPath(path, (int)(unit.MoveSpeed * timePassed));
        // }
        // TODO RSDK TEST: implement WaypointTracker together with Prediction::PathTracker using Events::OnNewPath.
        return result;
    }

    // LeagueSharp.Common.Geometry.PathLength(). Shared because Orbwalker and Prediction both consume path length.
    [[nodiscard]] inline float PathLength(const std::vector<Vec3>& path) noexcept
    {
        float result{};
        for (std::size_t i = 1; i < path.size(); ++i)
            result += path[i - 1].Distance2D(path[i]);
        return result;
    }

    [[nodiscard]] inline std::vector<Vec2> CutPath(const std::vector<Vec2>& path, float distance)
    {
        if (path.empty())
            return {};

        if (distance < 0.0f)
        {
            std::vector<Vec2> result = path;

            if (result.size() > 1u)
                result[0] += (result[1] - result[0]).Normalized() * distance;

            return result;
        }

        for (std::size_t i = 0; i + 1u < path.size(); ++i)
        {
            const float segment = path[i].DistanceTo(path[i + 1u]);
            if (segment > distance)
            {
                std::vector<Vec2> result;
                result.reserve(path.size() - i);

                result.push_back(path[i] + (path[i + 1u] - path[i]).Normalized() * distance);

                result.insert(result.end(), path.begin() + i + 1u, path.end());

                return result;
            }

            distance -= segment;
        }

        return {path.back()};
    }

    [[nodiscard]] inline std::vector<Vec3> CutPath(const std::vector<Vec3>& path, float distance)
    {
        if (path.empty())
            return {};

        if (distance < 0.0f)
        {
            std::vector<Vec3> result = path;
            if (result.size() > 1u)
            {
                const float segment = result[0].Distance2D(result[1]);
                if (segment > std::numeric_limits<float>::epsilon())
                    result[0] += (result[1] - result[0]) * (distance / segment);
            }
            return result;
        }

        for (std::size_t i = 0; i + 1u < path.size(); ++i)
        {
            const float segment = path[i].Distance2D(path[i + 1u]);
            if (segment > distance)
            {
                std::vector<Vec3> result;
                result.reserve(path.size() - i);
                const float t = segment > std::numeric_limits<float>::epsilon() ? distance / segment : 0.0f;
                result.push_back(path[i] + (path[i + 1u] - path[i]) * t);
                result.insert(result.end(), path.begin() + static_cast<std::ptrdiff_t>(i + 1u), path.end());
                return result;
            }
            distance -= segment;
        }

        return {path.back()};
    }

    [[nodiscard]] inline bool IsFacing(const Object& source, const Object& target) noexcept
    {
        if (!source || !target)
            return false;

        const Vec2 facing = source.Direction().To2D();
        const Vec2 toward = (target.Position() - source.Position()).To2D();

        return facing.IsValid() && toward.IsValid() && facing.DotProduct(toward) > 0.0f;
        //return facing.IsValid() && toward.IsValid() && facing.AngleBetween(toward) < 90.0f;
    }

    [[nodiscard]] inline bool IsBothFacing(const Object& source, const Object& target) noexcept
    {
        return IsFacing(source, target) && IsFacing(target, source);
    }

    // LeagueSharp.Common.Damage.CalcPhysicalDamage() resistance/penetration core.
    // Temporarily kept in Utility.h. This intentionally excludes champion passives, on-hit items
    // and other special bonus damage; those should be exposed by a separate helper/module later.
    [[nodiscard]] inline float PhysicalDamageMultiplier(const Object& source, const Object& target) noexcept
    {
        const float armor = target.Armor();
        if (armor < 0.0f)
            return 2.0f - 100.0f / (100.0f - armor);

        const float percentPenetrationMultiplier = std::max(0.0f, source.ArmorPenetrationPercent());
        const float effectiveArmor = armor * percentPenetrationMultiplier - source.ArmorPenetrationFlat();
        if (effectiveArmor <= 0.0f)
            return 1.0f;

        return 100.0f / (100.0f + effectiveArmor);
    }

    // LeagueSharp.Common.Damage.CalcMagicDamage() resistance/penetration core.
    [[nodiscard]] inline float MagicalDamageMultiplier(const Object& source, const Object& target) noexcept
    {
        const float magicResist = target.MagicResist();
        if (magicResist < 0.0f)
            return 2.0f - 100.0f / (100.0f - magicResist);

        const float percentPenetrationMultiplier = std::max(0.0f, source.MagicPenetrationPercent());
        const float effectiveMagicResist = magicResist * percentPenetrationMultiplier - source.MagicPenetrationFlat();
        if (effectiveMagicResist <= 0.0f)
            return 1.0f;

        return 100.0f / (100.0f + effectiveMagicResist);
    }

    // LeagueSharp.Common.Damage.CalcDamage() subset used by TargetSelector scoring.
    // Raw mitigation only. Passive/on-hit/special bonus damage is deliberately NOT mixed in here.
    [[nodiscard]] inline float CalculateDamage(
        const Object& source,
        const Object& target,
        const DamageType damageType,
        const float rawDamage) noexcept
    {
        if (!source || !target)
            return 0.0f;

        switch (damageType)
        {
        case DamageType::Physical: return std::max(0.0f, rawDamage * PhysicalDamageMultiplier(source, target));
        case DamageType::Magical: return std::max(0.0f, rawDamage * MagicalDamageMultiplier(source, target));
        case DamageType::True: return std::max(0.0f, rawDamage);
        }

        return 0.0f;
    }

    // LeagueSharp.Common.Utility.DrawCircle(center, radius, color, thickness, quality, onMinimap).
    // RSDK delegates to the public renderer instead of reimplementing a line loop in Common.
    inline void DrawCircle(
        const Vec3& center,
        const float radius,
        const Color color,
        const float thickness = 1.0f,
        const std::uint32_t segments = 64u,
        const bool onMinimap = false,
        const bool clipTerrain = false) noexcept
    {
        if (!Render::IsReady() || radius <= 0.0f || segments < 3u)
            return;

        if (!onMinimap)
        {
            Render::Circle3D(center, radius, color, thickness, segments, clipTerrain);
            return;
        }

        const Vec2 minimapCenter = Game::WorldToMinimap(center);
        const Vec2 minimapEdge = Game::WorldToMinimap(center + Vec3(radius, 0.0f, 0.0f));
        if (!minimapCenter.IsValid() || !minimapEdge.IsValid())
            return;

        const Vec2 delta = minimapEdge - minimapCenter;
        const float minimapRadius = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (minimapRadius > 0.0f)
            Render::Circle2D(minimapCenter, minimapRadius, color, thickness, segments);
    }
}
