#pragma once

#include "../RSDK.h"
#include "../RSDKDamage.h"

#include <algorithm>
#include <cstdint>

namespace rsdk
{
    enum class DamageStage : std::uint8_t
    {
        Default = RSDK_DAMAGE_STAGE_DEFAULT,
        WayBack = RSDK_DAMAGE_STAGE_WAY_BACK,
        Detonation = RSDK_DAMAGE_STAGE_DETONATION,
        DamagePerSecond = RSDK_DAMAGE_STAGE_DAMAGE_PER_SECOND,
        SecondForm = RSDK_DAMAGE_STAGE_SECOND_FORM,
        SecondCast = RSDK_DAMAGE_STAGE_SECOND_CAST,
        Buff = RSDK_DAMAGE_STAGE_BUFF,
        Empowered = RSDK_DAMAGE_STAGE_EMPOWERED
    };

    class Damage final
    {
    public:
        [[nodiscard]] static bool IsAvailable() noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->IsAvailable != nullptr && api->IsAvailable() != RSDK_FALSE;
        }

        [[nodiscard]] static float CalculateDamage(
            const Object& source,
            const Object& target,
            const DamageType damageType,
            const float rawDamage) noexcept
        {
            const auto* const api = Api();
            if (!source || !target || api == nullptr || api->CalculateDamage == nullptr)
                return 0.0f;

            RSDK_DamageCalculateRequest request{};
            request.structSize = sizeof(request);
            request.sourceNetworkId = source.NetworkId();
            request.targetNetworkId = target.NetworkId();
            request.damageType = static_cast<std::uint8_t>(damageType);
            request.rawDamage = rawDamage;

            float result{};
            return api->CalculateDamage(&request, &result) != RSDK_FALSE ? std::max(0.0f, result) : 0.0f;
        }

        [[nodiscard]] static float GetSpellDamage(
            const Object& source,
            const Object& target,
            const SpellSlot slot,
            const DamageStage stage = DamageStage::Default,
            const std::uint32_t flags = 0u) noexcept
        {
            const auto* const api = Api();
            if (!source || !target || api == nullptr || api->GetSpellDamage == nullptr)
                return 0.0f;

            RSDK_SpellDamageRequest request{};
            request.structSize = sizeof(request);
            request.sourceNetworkId = source.NetworkId();
            request.targetNetworkId = target.NetworkId();
            request.slot = static_cast<std::int8_t>(slot);
            request.stage = static_cast<std::uint8_t>(stage);
            request.flags = flags;

            float result{};
            return api->GetSpellDamage(&request, &result) != RSDK_FALSE ? std::max(0.0f, result) : 0.0f;
        }

        [[nodiscard]] static float GetAutoAttackDamage(
            const Object& source,
            const Object& target,
            const bool includePassives = true) noexcept
        {
            const auto* const api = Api();
            if (!source || !target || api == nullptr || api->GetAutoAttackDamage == nullptr)
                return 0.0f;

            RSDK_AutoAttackDamageRequest request{};
            request.structSize = sizeof(request);
            request.sourceNetworkId = source.NetworkId();
            request.targetNetworkId = target.NetworkId();
            request.flags = includePassives ? RSDK_AUTO_ATTACK_DAMAGE_INCLUDE_PASSIVES : RSDK_AUTO_ATTACK_DAMAGE_NONE;

            float result{};
            return api->GetAutoAttackDamage(&request, &result) != RSDK_FALSE ? std::max(0.0f, result) : 0.0f;
        }

    private:
        [[nodiscard]] static const RSDK_DamageApi* Api() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->damage : nullptr;
        }
    };
}
