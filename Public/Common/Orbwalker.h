#pragma once

#include "../RSDKCpp.hpp"
#include "../RSDKRuntime.hpp"
#include "../RSDKOrbwalker.h"

#include <array>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace rsdk
{
    struct BeforeAttackEvent final
    {
        Object Unit{};
        Object Target{};
        bool Process{true};
    };

    struct OrbwalkerAttackEvent final
    {
        Object Unit{};
        Object Target{};
    };

    struct OrbwalkerTargetChangeEvent final
    {
        Object OldTarget{};
        Object NewTarget{};
    };

    namespace detail_orbwalker
    {
        [[nodiscard]] inline const RSDK_OrbwalkerApi* Api() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->orbwalker : nullptr;
        }

        template <typename Callable>
        struct BeforeBinding final { Callable callable; };

        template <typename Callable>
        void RSDK_CALL BeforeTrampoline(RSDK_OrbwalkerBeforeAttackEvent* const eventData, void* const userData)
        {
            auto* const binding = static_cast<BeforeBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;
            BeforeAttackEvent event{
                ObjectManager::FindObjectByNetworkId(eventData->unitNetworkId),
                ObjectManager::FindObjectByNetworkId(eventData->targetNetworkId),
                eventData->process != RSDK_FALSE};
            std::invoke(binding->callable, event);
            eventData->process = event.Process ? RSDK_TRUE : RSDK_FALSE;
        }

        template <typename Callable>
        struct AttackBinding final { Callable callable; };

        template <typename Callable>
        void RSDK_CALL AttackTrampoline(const RSDK_OrbwalkerAttackEvent* const eventData, void* const userData)
        {
            auto* const binding = static_cast<AttackBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;

            const OrbwalkerAttackEvent event{
                ObjectManager::FindObjectByNetworkId(eventData->unitNetworkId),
                ObjectManager::FindObjectByNetworkId(eventData->targetNetworkId)};
            std::invoke(binding->callable, event);
        }

        template <typename Callable>
        void RSDK_CALL NonKillableTrampoline(const RSDK_OrbwalkerAttackEvent* const eventData, void* const userData)
        {
            auto* const binding = static_cast<AttackBinding<Callable>*>(userData);
            if (binding != nullptr && eventData != nullptr)
                std::invoke(binding->callable, ObjectManager::FindObjectByNetworkId(eventData->targetNetworkId));
        }

        template <typename Callable>
        struct TargetChangeBinding final { Callable callable; };

        template <typename Callable>
        void RSDK_CALL TargetChangeTrampoline(const RSDK_OrbwalkerTargetChangeEvent* const eventData, void* const userData)
        {
            auto* const binding = static_cast<TargetChangeBinding<Callable>*>(userData);
            if (binding == nullptr || eventData == nullptr)
                return;
            const OrbwalkerTargetChangeEvent event{
                ObjectManager::FindObjectByNetworkId(eventData->oldTargetNetworkId),
                ObjectManager::FindObjectByNetworkId(eventData->newTargetNetworkId)};
            std::invoke(binding->callable, event);
        }
    }

    class OrbwalkerBeforeAttackEventSource final
    {
    public:
        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::BeforeBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterBeforeAttack == nullptr || api->UnregisterEvent == nullptr)
                return false;
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterBeforeAttack(&detail_orbwalker::BeforeTrampoline<Stored>, binding);
            return detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::BeforeBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterBeforeAttack == nullptr || api->UnregisterEvent == nullptr)
                return {};
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterBeforeAttack(&detail_orbwalker::BeforeTrampoline<Stored>, binding);
            if (!detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent))
                return {};
            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const { Subscribe(std::forward<Callable>(callable)); }
    };

    class OrbwalkerAttackEventSource final
    {
    public:
        enum class Kind : std::uint8_t { OnAttack, AfterAttack };
        constexpr explicit OrbwalkerAttackEventSource(const Kind kind) noexcept : kind_(kind) {}

        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::AttackBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->UnregisterEvent == nullptr)
                return false;

            const auto registerFn = kind_ == Kind::OnAttack ? api->RegisterOnAttack : api->RegisterAfterAttack;
            if (registerFn == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = registerFn(&detail_orbwalker::AttackTrampoline<Stored>, binding);
            return detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::AttackBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->UnregisterEvent == nullptr)
                return {};
            const auto registerFn = kind_ == Kind::OnAttack ? api->RegisterOnAttack : api->RegisterAfterAttack;
            if (registerFn == nullptr)
                return {};
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = registerFn(&detail_orbwalker::AttackTrampoline<Stored>, binding);
            if (!detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent))
                return {};
            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const { Subscribe(std::forward<Callable>(callable)); }

    private:
        Kind kind_{};
    };

    class OrbwalkerNonKillableMinionEventSource final
    {
    public:
        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::AttackBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterOnNonKillableMinion == nullptr || api->UnregisterEvent == nullptr)
                return false;

            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterOnNonKillableMinion(&detail_orbwalker::NonKillableTrampoline<Stored>, binding);
            return detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::AttackBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterOnNonKillableMinion == nullptr || api->UnregisterEvent == nullptr)
                return {};
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterOnNonKillableMinion(&detail_orbwalker::NonKillableTrampoline<Stored>, binding);
            if (!detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent))
                return {};
            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const { Subscribe(std::forward<Callable>(callable)); }
    };

    class OrbwalkerTargetChangeEventSource final
    {
    public:
        template <typename Callable>
        bool Subscribe(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::TargetChangeBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterOnTargetChange == nullptr || api->UnregisterEvent == nullptr)
                return false;
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterOnTargetChange(&detail_orbwalker::TargetChangeTrampoline<Stored>, binding);
            return detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent);
        }

        template <typename Callable>
        [[nodiscard]] EventConnection Connect(Callable&& callable) const
        {
            using Stored = std::decay_t<Callable>;
            using Binding = detail_orbwalker::TargetChangeBinding<Stored>;
            const auto* const api = detail_orbwalker::Api();
            if (api == nullptr || api->RegisterOnTargetChange == nullptr || api->UnregisterEvent == nullptr)
                return {};
            auto* const binding = new Binding{std::forward<Callable>(callable)};
            const auto token = api->RegisterOnTargetChange(&detail_orbwalker::TargetChangeTrampoline<Stored>, binding);
            if (!detail::Runtime::Instance().TrackCustom(token, binding, &detail::DestroyContext<Binding>, api->UnregisterEvent))
                return {};
            return EventConnection(token);
        }

        template <typename Callable>
        void operator+=(Callable&& callable) const { Subscribe(std::forward<Callable>(callable)); }
    };

    class Orbwalker final
    {
    public:
        enum class Mode : std::uint8_t
        {
            LastHit = RSDK_ORBWALKER_MODE_LAST_HIT,
            Mixed = RSDK_ORBWALKER_MODE_MIXED,
            LaneClear = RSDK_ORBWALKER_MODE_LANE_CLEAR,
            Combo = RSDK_ORBWALKER_MODE_COMBO,
            Freeze = RSDK_ORBWALKER_MODE_FREEZE,
            CustomMode = RSDK_ORBWALKER_MODE_CUSTOM,
            None = RSDK_ORBWALKER_MODE_NONE,
            Flee = RSDK_ORBWALKER_MODE_FLEE
        };

        [[nodiscard]] static bool IsAvailable() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->IsAvailable&&a->IsAvailable()!=RSDK_FALSE; }
        [[nodiscard]] static bool SpellFarmEnabled() noexcept
        {
            const auto* const api = detail_orbwalker::Api();
            return api && api->GetSpellFarmEnabled && api->GetSpellFarmEnabled() != RSDK_FALSE;
        }
        [[nodiscard]] static Mode ActiveMode() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetActiveMode?static_cast<Mode>(a->GetActiveMode()):Mode::None; }
        static void SetMode(const Mode mode) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetActiveMode)a->SetActiveMode(static_cast<std::uint8_t>(mode)); }
        [[nodiscard]] static Object GetTarget() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetTarget?ObjectManager::FindObjectByNetworkId(a->GetTarget()):Object{}; }
        static void ForceTarget(const Object& target) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->ForceTarget)a->ForceTarget(target?target.NetworkId():0u); }
        static void ClearForcedTarget() noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->ClearForcedTarget)a->ClearForcedTarget(); }
        [[nodiscard]] static bool AttackEnabled() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetAttackEnabled&&a->GetAttackEnabled()!=RSDK_FALSE; }
        static void SetAttack(const bool enabled) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetAttackEnabled)a->SetAttackEnabled(enabled?RSDK_TRUE:RSDK_FALSE); }
        [[nodiscard]] static bool MovementEnabled() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetMovementEnabled&&a->GetMovementEnabled()!=RSDK_FALSE; }
        static void SetMovement(const bool enabled) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetMovementEnabled)a->SetMovementEnabled(enabled?RSDK_TRUE:RSDK_FALSE); }
        [[nodiscard]] static bool CanAttack() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->CanAttack&&a->CanAttack()!=RSDK_FALSE; }
        [[nodiscard]] static bool CanMove(const float extraWindup = 0.0f, const bool disableMissileCheck = false) noexcept
        { const auto* a=detail_orbwalker::Api(); return a&&a->CanMove&&a->CanMove(extraWindup,disableMissileCheck?RSDK_TRUE:RSDK_FALSE)!=RSDK_FALSE; }
        [[nodiscard]] static float GetAttackRange(const Object& target) noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetAttackRange?a->GetAttackRange(target?target.NetworkId():0u):0.0f; }
        [[nodiscard]] static float GetRealAutoAttackRange(const Object& target = {}) noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetRealAutoAttackRange?a->GetRealAutoAttackRange(target?target.NetworkId():0u):0.0f; }
        [[nodiscard]] static bool InAutoAttackRange(const Object& target) noexcept { const auto* a=detail_orbwalker::Api(); return target&&a&&a->InAutoAttackRange&&a->InAutoAttackRange(target.NetworkId())!=RSDK_FALSE; }
        [[nodiscard]] static float GetMyProjectileSpeed() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetMyProjectileSpeed?a->GetMyProjectileSpeed():0.0f; }
        [[nodiscard]] static bool IsAutoAttack(const char* name) noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->IsAutoAttack&&a->IsAutoAttack(name)!=RSDK_FALSE; }
        [[nodiscard]] static bool IsAutoAttackReset(const char* name) noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->IsAutoAttackReset&&a->IsAutoAttackReset(name)!=RSDK_FALSE; }
        static void ResetAutoAttackTimer() noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->ResetAutoAttackTimer)a->ResetAutoAttackTimer(); }
        static void SetMinimumOrbwalkDistance(const float distance) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetMinimumOrbwalkDistance)a->SetMinimumOrbwalkDistance(distance); }
        static void SetMovementDelay(const float delay) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetMovementDelay)a->SetMovementDelay(delay); }
        static void SetOrbwalkingPoint(const Vec3& position) noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->SetOrbwalkingPoint)a->SetOrbwalkingPoint(position); }
        static void ClearOrbwalkingPoint() noexcept { const auto* a=detail_orbwalker::Api(); if(a&&a->ClearOrbwalkingPoint)a->ClearOrbwalkingPoint(); }
        static void Orbwalk(const Object& target, const Vec3& position, const float extraWindup = 0.090f, const float holdAreaRadius = 0.0f,
            const bool useFixedDistance = true, const bool randomizeMinDistance = true) noexcept
        { const auto* a=detail_orbwalker::Api(); if(a&&a->Orbwalk)a->Orbwalk(target?target.NetworkId():0u,position,extraWindup,holdAreaRadius,useFixedDistance?RSDK_TRUE:RSDK_FALSE,randomizeMinDistance?RSDK_TRUE:RSDK_FALSE); }
        static void MoveTo(const Vec3& position, const float holdAreaRadius = 0.0f, const bool overrideTimer = false,
            const bool useFixedDistance = true, const bool randomizeMinDistance = true) noexcept
        { const auto* a=detail_orbwalker::Api(); if(a&&a->MoveTo)a->MoveTo(position,holdAreaRadius,overrideTimer?RSDK_TRUE:RSDK_FALSE,useFixedDistance?RSDK_TRUE:RSDK_FALSE,randomizeMinDistance?RSDK_TRUE:RSDK_FALSE); }
        [[nodiscard]] static bool ShouldWait() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->ShouldWait&&a->ShouldWait()!=RSDK_FALSE; }
        [[nodiscard]] static Object LastTarget() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetLastTarget?ObjectManager::FindObjectByNetworkId(a->GetLastTarget()):Object{}; }
        [[nodiscard]] static std::array<Object, 3> LastTargets() noexcept
        {
            std::array<Object, 3> result{};
            const auto* a=detail_orbwalker::Api();
            if (!a || !a->GetLastTargetAt) return result;
            for (std::uint32_t i=0; i<3u; ++i) result[i]=ObjectManager::FindObjectByNetworkId(a->GetLastTargetAt(i));
            return result;
        }
        // LeagueSharp exposes LastAATick/command timestamps as millisecond ticks.
        // RSDK stores the same gameplay timestamps in Game::TimeGame() seconds.
        [[nodiscard]] static float LastAATick() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetLastAATick?a->GetLastAATick():0.0f; }
        [[nodiscard]] static float LastAttackCommandTime() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetLastAttackCommandTime?a->GetLastAttackCommandTime():0.0f; }
        [[nodiscard]] static float LastMoveCommandTime() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetLastMoveCommandTime?a->GetLastMoveCommandTime():0.0f; }
        [[nodiscard]] static Vec3 LastMoveCommandPosition() noexcept { const auto* a=detail_orbwalker::Api(); return a&&a->GetLastMoveCommandPosition?Vec3(a->GetLastMoveCommandPosition()):Vec3{}; }

        static inline OrbwalkerBeforeAttackEventSource BeforeAttack{};
        static inline OrbwalkerAttackEventSource OnAttack{OrbwalkerAttackEventSource::Kind::OnAttack};
        static inline OrbwalkerAttackEventSource AfterAttack{OrbwalkerAttackEventSource::Kind::AfterAttack};
        static inline OrbwalkerNonKillableMinionEventSource OnNonKillableMinion{};
        static inline OrbwalkerTargetChangeEventSource OnTargetChange{};
    };
}
