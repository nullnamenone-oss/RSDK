#pragma once

#include "../RSDKCpp.hpp"
#include "../RSDKRuntime.hpp"
#include "../RSDKOrbwalker.h"

namespace rsdk
{
    // HealthPrediction is implemented by Orbwalker.dll but remains a separate public API/class.
    class HealthPrediction final
    {
    public:
        [[nodiscard]] static bool IsAvailable() noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->IsAvailable != nullptr && api->IsAvailable() != RSDK_FALSE
                && api->GetHealthPrediction != nullptr && api->LaneClearHealthPrediction != nullptr;
        }

        [[nodiscard]] static float GetHealthPrediction(const Object& unit, const float time, const float delay = 0.070f) noexcept
        {
            const auto* const api = Api();
            return unit && api != nullptr && api->GetHealthPrediction != nullptr
                ? api->GetHealthPrediction(unit.NetworkId(), time, delay) : 0.0f;
        }

        [[nodiscard]] static float LaneClearHealthPrediction(const Object& unit, const float time, const float delay = 0.070f) noexcept
        {
            const auto* const api = Api();
            return unit && api != nullptr && api->LaneClearHealthPrediction != nullptr
                ? api->LaneClearHealthPrediction(unit.NetworkId(), time, delay) : 0.0f;
        }

        [[nodiscard]] static bool HasMinionAggro(const Object& minion) noexcept
        {
            const auto* const api = Api();
            return minion && api != nullptr && api->HasMinionAggro != nullptr && api->HasMinionAggro(minion.NetworkId()) != RSDK_FALSE;
        }

        [[nodiscard]] static bool HasTurretAggro(const Object& minion) noexcept
        {
            const auto* const api = Api();
            return minion && api != nullptr && api->HasTurretAggro != nullptr && api->HasTurretAggro(minion.NetworkId()) != RSDK_FALSE;
        }

        [[nodiscard]] static float TurretAggroStartTime(const Object& minion) noexcept
        {
            const auto* const api = Api();
            return minion && api != nullptr && api->TurretAggroStartTime != nullptr ? api->TurretAggroStartTime(minion.NetworkId()) : 0.0f;
        }

    private:
        [[nodiscard]] static const RSDK_OrbwalkerApi* Api() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->orbwalker : nullptr;
        }
    };
}
