#pragma once

#include "../RSDK.h"
#include "HeroManager.h"
#include "Utility.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace rsdk
{
    class Spell;

    // Public C++ wrapper for the shared TargetSelector service.
    // The implementation/state/menu live in Modules/Gameplay/TargetSelector/TargetSelector.cpp (TargetSelector.dll).
    // Consumer DLLs only talk to the stable RSDK Core facade in RSDK_Api::targetSelector.
    class TargetSelector final
    {
    public:
        // LeagueSharp.Common.TargetSelector.DamageType -> shared rsdk::DamageType in RSDKConstants.h.
        // Alias kept so old TargetSelector::DamageType source still compiles while champion code can use rsdk::DamageType directly.
        using DamageType = rsdk::DamageType;

        // LeagueSharp.Common.TargetSelector.TargetingMode
        enum class TargetingMode : std::uint8_t
        {
            AutoPriority = RSDK_TARGET_MODE_AUTO_PRIORITY,
            LowHP = RSDK_TARGET_MODE_LOW_HP,
            MostAD = RSDK_TARGET_MODE_MOST_AD,
            MostAP = RSDK_TARGET_MODE_MOST_AP,
            Closest = RSDK_TARGET_MODE_CLOSEST,
            NearMouse = RSDK_TARGET_MODE_NEAR_MOUSE,
            LessAttack = RSDK_TARGET_MODE_LESS_ATTACK,
            LessCast = RSDK_TARGET_MODE_LESS_CAST,
            MostStack = RSDK_TARGET_MODE_MOST_STACK
        };

        // LeagueSharp: TargetSelectionConditionDelegate conditions.
        // RSDK rename: TargetFilter/filter because this callback filters candidates before scoring.
        using TargetFilter = bool(*)(const Object& target);

        class ModeProperty final
        {
        public:
            ModeProperty& operator=(const TargetingMode value) noexcept
            {
                TargetSelector::SetMode(value);
                return *this;
            }

            [[nodiscard]] operator TargetingMode() const noexcept
            {
                return TargetSelector::GetMode();
            }
        };

        // Keeps source usage close to LeagueSharp: TargetSelector::Mode = TargetingMode::AutoPriority;
        inline static ModeProperty Mode{};

        [[nodiscard]] static bool IsAvailable() noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->IsAvailable != nullptr && api->IsAvailable() != RSDK_FALSE;
        }

        // Compatibility with the old header-only test port. The shared module owns initialization now.
        [[nodiscard]] static bool Initialize() noexcept { return IsAvailable(); }
        static void Shutdown() noexcept {}

        [[nodiscard]] static bool GetCustomTS() noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetCustomTS != nullptr && api->GetCustomTS() != RSDK_FALSE;
        }

        static void SetCustomTS(const bool value) noexcept
        {
            const auto* const api = Api();
            if (api != nullptr && api->SetCustomTS != nullptr)
                api->SetCustomTS(value ? RSDK_TRUE : RSDK_FALSE);
        }

        static void AddToMenu(const MenuGroup& group) noexcept
        {
            if (group)
                group.AddText("target_selector_notice", "Target Selector settings: Gameplay > Target Selector");
        }

        [[nodiscard]] static Object GetSelectedTarget() noexcept
        {
            const auto* const api = Api();
            if (api == nullptr || api->GetSelectedTarget == nullptr)
                return {};
            return ObjectManager::FindObjectByNetworkId(api->GetSelectedTarget());
        }

        static void SetTarget(const Object& hero) noexcept
        {
            const auto* const api = Api();
            if (api != nullptr && api->SetTarget != nullptr)
                api->SetTarget(hero ? hero.NetworkId() : 0u);
        }

        static void ClearTarget() noexcept
        {
            const auto* const api = Api();
            if (api != nullptr && api->ClearTarget != nullptr)
                api->ClearTarget();
        }

        [[nodiscard]] static float GetPriority(const Object& hero) noexcept
        {
            const auto* const api = Api();
            return hero && api != nullptr && api->GetPriority != nullptr ? api->GetPriority(hero.NetworkId()) : 1.0f;
        }

        [[nodiscard]] static int GetPriorityLevel(const Object& hero) noexcept
        {
            const auto* const api = Api();
            return hero && api != nullptr && api->GetPriorityLevel != nullptr ? api->GetPriorityLevel(hero.NetworkId()) : 1;
        }

        static void SetPriority(const Object& hero, const int priority) noexcept
        {
            const auto* const api = Api();
            if (hero && api != nullptr && api->SetPriority != nullptr)
                api->SetPriority(hero.NetworkId(), std::clamp(priority, 1, 5));
        }

        [[nodiscard]] static TargetingMode GetMode() noexcept
        {
            const auto* const api = Api();
            const std::uint8_t value = api != nullptr && api->GetMode != nullptr ? api->GetMode() : static_cast<std::uint8_t>(RSDK_TARGET_MODE_AUTO_PRIORITY);
            return static_cast<TargetingMode>(std::clamp<std::uint8_t>(value, 0u, 8u));
        }

        static void SetMode(const TargetingMode mode) noexcept
        {
            const auto* const api = Api();
            if (api != nullptr && api->SetMode != nullptr)
                api->SetMode(static_cast<std::uint8_t>(mode));
        }

        [[nodiscard]] static bool IsInvulnerable(const Object& target, const DamageType damageType, const bool ignoreShields = true) noexcept
        {
            const auto* const api = Api();
            return target && api != nullptr && api->IsInvulnerable != nullptr
                && api->IsInvulnerable(target.NetworkId(), static_cast<std::uint8_t>(damageType), ignoreShields ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
        }

        [[nodiscard]] static Object GetTargetNoCollision(
            const Spell& spell,
            bool ignoreShield = true,
            const std::vector<Object>* ignoredChamps = nullptr,
            const Vec3* rangeCheckFrom = nullptr);

        [[nodiscard]] static Object GetTarget(
            const float range,
            const DamageType damageType,
            const bool ignoreShieldSpells = true,
            const Vec3* const rangeCheckFrom = nullptr,
            const TargetFilter filter = nullptr)
        {
            return GetTarget(HeroManager::Player(), range, damageType, ignoreShieldSpells, nullptr, rangeCheckFrom, filter);
        }

        [[nodiscard]] static Object GetTarget(
            const float range,
            const DamageType damageType,
            const bool ignoreShieldSpells,
            const std::vector<Object>& ignoredChamps,
            const Vec3* const rangeCheckFrom = nullptr,
            const TargetFilter filter = nullptr)
        {
            return GetTarget(HeroManager::Player(), range, damageType, ignoreShieldSpells, &ignoredChamps, rangeCheckFrom, filter);
        }

        [[nodiscard]] static Object GetTarget(
            const Object& champion,
            const float range,
            const DamageType damageType,
            const bool ignoreShieldSpells = true,
            const Vec3* const rangeCheckFrom = nullptr,
            const TargetFilter filter = nullptr)
        {
            return GetTarget(champion, range, damageType, ignoreShieldSpells, nullptr, rangeCheckFrom, filter);
        }

        [[nodiscard]] static Object GetTarget(
            const Object& champion,
            const float range,
            const DamageType damageType,
            const bool ignoreShieldSpells,
            const std::vector<Object>& ignoredChamps,
            const Vec3* const rangeCheckFrom = nullptr,
            const TargetFilter filter = nullptr)
        {
            return GetTarget(champion, range, damageType, ignoreShieldSpells, &ignoredChamps, rangeCheckFrom, filter);
        }

    private:
        struct FilterContext final
        {
            TargetFilter filter{};
        };

        static RSDK_Bool RSDK_CALL FilterTrampoline(const std::uint32_t networkId, void* const userData)
        {
            const auto* const context = static_cast<const FilterContext*>(userData);
            if (context == nullptr || context->filter == nullptr)
                return RSDK_TRUE;
            const Object target = ObjectManager::FindObjectByNetworkId(networkId);
            return target && context->filter(target) ? RSDK_TRUE : RSDK_FALSE;
        }

        [[nodiscard]] static Object GetTarget(
            const Object& champion,
            const float range,
            const DamageType damageType,
            const bool ignoreShieldSpells,
            const std::vector<Object>* const ignoredChamps,
            const Vec3* const rangeCheckFrom,
            const TargetFilter filter)
        {
            const auto* const api = Api();
            if (api == nullptr || api->GetTarget == nullptr)
                return {};

            std::vector<std::uint32_t> ignoredNetworkIds;
            if (ignoredChamps != nullptr)
            {
                ignoredNetworkIds.reserve(ignoredChamps->size());
                for (const Object& ignored : *ignoredChamps)
                {
                    if (ignored)
                        ignoredNetworkIds.push_back(ignored.NetworkId());
                }
            }

            FilterContext filterContext{filter};
            RSDK_TargetSelectorRequest request{};
            request.structSize = sizeof(request);
            request.sourceNetworkId = champion ? champion.NetworkId() : 0u;
            request.range = range;
            request.damageType = static_cast<std::uint8_t>(damageType);
            request.ignoreShieldSpells = ignoreShieldSpells ? RSDK_TRUE : RSDK_FALSE;
            request.ignoredNetworkIds = ignoredNetworkIds.empty() ? nullptr : ignoredNetworkIds.data();
            request.ignoredCount = static_cast<std::uint32_t>(ignoredNetworkIds.size());
            request.hasRangeCheckFrom = rangeCheckFrom != nullptr ? RSDK_TRUE : RSDK_FALSE;
            request.rangeCheckFrom = rangeCheckFrom != nullptr ? static_cast<RSDK_Vec3>(*rangeCheckFrom) : RSDK_Vec3{};
            request.filter = filter != nullptr ? &FilterTrampoline : nullptr;
            request.filterUserData = filter != nullptr ? &filterContext : nullptr;

            return ObjectManager::FindObjectByNetworkId(api->GetTarget(&request));
        }

        [[nodiscard]] static const RSDK_TargetSelectorApi* Api() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->targetSelector : nullptr;
        }
    };

    // LeagueSharp.Common.LockedTargetSelector behavior remains local to each consumer DLL.
    // This is intentional: locking is caller-specific, while the underlying TargetSelector state/menu is shared.
    class LockedTargetSelector final
    {
    public:
        static void AddToMenu(const MenuGroup& group) noexcept { TargetSelector::AddToMenu(group); }

        [[nodiscard]] static Object GetTarget(
            const float range,
            const TargetSelector::DamageType damageType,
            const bool ignoreShield = true,
            const std::vector<Object>* const ignoredChamps = nullptr,
            const Vec3* const rangeCheckFrom = nullptr)
        {
            const Object lastTarget = LastTarget();
            if (!lastTarget || !Utility::IsValidTarget(lastTarget) || detail::lastDamageType != damageType)
                return SelectNewTarget(range, damageType, ignoreShield, ignoredChamps, rangeCheckFrom);

            const Vec3* from = rangeCheckFrom;
            if (Utility::IsValidTarget(lastTarget, range, true, from) && detail::lastDamageType == damageType)
                return lastTarget;

            return SelectNewTarget(range, damageType, ignoreShield, ignoredChamps, rangeCheckFrom);
        }

        static void UnlockTarget() noexcept { detail::lastNetworkId = 0u; }

    private:
        struct detail final
        {
            inline static std::uint32_t lastNetworkId{};
            inline static TargetSelector::DamageType lastDamageType{TargetSelector::DamageType::Physical};
        };

        [[nodiscard]] static Object LastTarget() noexcept
        {
            return detail::lastNetworkId != 0u ? ObjectManager::FindObjectByNetworkId(detail::lastNetworkId) : Object{};
        }

        [[nodiscard]] static Object SelectNewTarget(
            const float range,
            const TargetSelector::DamageType damageType,
            const bool ignoreShield,
            const std::vector<Object>* const ignoredChamps,
            const Vec3* const rangeCheckFrom)
        {
            const Object target = ignoredChamps != nullptr
                ? TargetSelector::GetTarget(range, damageType, ignoreShield, *ignoredChamps, rangeCheckFrom)
                : TargetSelector::GetTarget(range, damageType, ignoreShield, rangeCheckFrom);

            detail::lastNetworkId = target ? target.NetworkId() : 0u;
            detail::lastDamageType = damageType;
            return target;
        }
    };
}
