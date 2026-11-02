#pragma once

#include "RSDK.h"
#include "RSDKConstants.h"
#include "RSDKRuntime.hpp"
#include "Common/Vector.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace rsdk
{

    class NavPath final
    {
    public:
        NavPath() noexcept = default;
        explicit NavPath(const RSDK_NavPath& data) noexcept : data_(data) {}

        [[nodiscard]] explicit operator bool() const noexcept { return data_.address != 0u; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return data_.address; }
        [[nodiscard]] Vec3 StartPos() const noexcept { return data_.startPosition; }
        [[nodiscard]] Vec3 EndPos() const noexcept { return data_.endPosition; }
        [[nodiscard]] bool IsDashing() const noexcept { return data_.isDashing != RSDK_FALSE; }
        [[nodiscard]] float DashSpeed() const noexcept { return data_.dashSpeed; }
        [[nodiscard]] std::uint8_t DashGravityCount() const noexcept { return data_.dashGravityCount; }
        [[nodiscard]] std::uintptr_t NavArray() const noexcept { return data_.navArray; }
        [[nodiscard]] std::uint32_t NavArrayCount() const noexcept { return data_.navArrayCount; }

    private:
        RSDK_NavPath data_{};
    };

    class Object;
    class SpellData;
    class SpellCast;

    enum class SpellEventMode : std::uint8_t
    {
        Trigger = 0,
        Bypass = 1
    };

    namespace detail
    {
        inline void RSDK_CALL StringSnapshotCallback(
            const char* const text,
            const std::uint32_t length,
            void* const userData) noexcept
        {
            auto* const output = static_cast<std::string*>(userData);
            if (output == nullptr)
                return;

            if (text == nullptr || length == 0u)
            {
                output->clear();
                return;
            }

            output->assign(text, length);
        }

        template<typename T>
        void RSDK_CALL VectorSnapshotCallback(
            const T* const values,
            const std::uint32_t count,
            void* const userData) noexcept
        {
            auto* const output = static_cast<std::vector<T>*>(userData);
            if (output == nullptr)
                return;

            if (values == nullptr || count == 0u)
            {
                output->clear();
                return;
            }

            output->assign(values, values + count);
        }

        inline void RSDK_CALL Vec3SnapshotCallback(
            const RSDK_Vec3* const values,
            const std::uint32_t count,
            void* const userData) noexcept
        {
            auto* const output = static_cast<std::vector<Vec3>*>(userData);
            if (output == nullptr)
                return;

            output->clear();
            if (values == nullptr || count == 0u)
                return;

            output->reserve(count);
            for (std::uint32_t index = 0; index < count; ++index)
                output->emplace_back(values[index]);
        }

        template<typename Handle, typename ReadFn>
        inline std::string ReadHandleString(const Handle handle, const ReadFn function)
        {
            if (handle == 0 || function == nullptr)
                return {};

            std::string result{};
            function(handle, &StringSnapshotCallback, &result);
            return result;
        }
    }

    class Perk final
    {
    public:
        Perk() = default;
        explicit Perk(const RSDK_PerkHandle handle) noexcept
            : Perk(detail::Runtime::Instance().Api(), handle)
        {
        }
        Perk(const RSDK_Api* api, RSDK_PerkHandle address) noexcept : api_(api), handle_(address) {}
        [[nodiscard]] explicit operator bool() const noexcept { return api_ && api_->perks && handle_ != RSDK_INVALID_PERK; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] uint32_t Id() const noexcept { return *this && api_->perks->GetId ? api_->perks->GetId(handle_) : 0u; }
        [[nodiscard]] std::string Name() const { return *this ? detail::ReadHandleString(handle_, api_->perks->ReadName) : std::string{}; }
    private: const RSDK_Api* api_{}; RSDK_PerkHandle handle_{RSDK_INVALID_PERK};
    };

    class Buff final
    {
    public:
        Buff() = default;
        explicit Buff(const RSDK_BuffHandle handle) noexcept
            : Buff(detail::Runtime::Instance().Api(), handle)
        {
        }
        Buff(const RSDK_Api* api, RSDK_BuffHandle address) noexcept : api_(api), handle_(address) {}
        [[nodiscard]] explicit operator bool() const noexcept { return api_ && api_->buffs && handle_ != RSDK_INVALID_BUFF; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] BuffType Type() const noexcept { return *this && api_->buffs->GetType ? static_cast<BuffType>(api_->buffs->GetType(handle_)) : BuffType::Internal; }
        [[nodiscard]] float BeginTime() const noexcept { return *this && api_->buffs->GetBeginTime ? api_->buffs->GetBeginTime(handle_) : 0.0f; }
        [[nodiscard]] float EndTime() const noexcept { return *this && api_->buffs->GetEndTime ? api_->buffs->GetEndTime(handle_) : 0.0f; }
        [[nodiscard]] float Duration() const noexcept { return *this && api_->buffs->GetDuration ? api_->buffs->GetDuration(handle_) : 0.0f; }
        [[nodiscard]] float RemainingTime() const noexcept { return *this && api_->buffs->GetRemainingTime ? api_->buffs->GetRemainingTime(handle_) : 0.0f; }
        [[nodiscard]] uint32_t Stacks() const noexcept { return *this && api_->buffs->GetStacks ? api_->buffs->GetStacks(handle_) : 0u; }
        [[nodiscard]] uint32_t Count() const noexcept { return *this && api_->buffs->GetCount ? api_->buffs->GetCount(handle_) : 0u; }
        [[nodiscard]] bool IsActive() const noexcept { return *this && api_->buffs->IsActive && api_->buffs->IsActive(handle_) != RSDK_FALSE; }
        [[nodiscard]] bool IsValid() const noexcept { return *this && api_->buffs->IsValid && api_->buffs->IsValid(handle_) != RSDK_FALSE; }
        [[nodiscard]] std::string Name() const { return *this ? detail::ReadHandleString(handle_, api_->buffs->ReadName) : std::string{}; }
        [[nodiscard]] Object Caster() const noexcept;
    private: const RSDK_Api* api_{}; RSDK_BuffHandle handle_{RSDK_INVALID_BUFF};
    };

    class Item final
    {
    public:
        static constexpr std::uint8_t InvalidSlot = 0xFFu;

        Item() = default;
        explicit Item(const RSDK_ItemHandle handle) noexcept
            : Item(detail::Runtime::Instance().Api(), handle, InvalidSlot)
        {
        }

        Item(
            const RSDK_Api* api,
            const RSDK_ItemHandle handle,
            const std::uint8_t slot = InvalidSlot,
            const std::uint32_t ownerNetworkId = 0u) noexcept
            : api_(api), handle_(handle), slot_(slot), ownerNetworkId_(ownerNetworkId)
        {
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return api_ != nullptr && api_->items != nullptr && handle_ != RSDK_INVALID_ITEM;
        }

        [[nodiscard]] std::uintptr_t Address() const noexcept
        {
            return static_cast<std::uintptr_t>(handle_);
        }

        [[nodiscard]] std::uint8_t Slot() const noexcept
        {
            return slot_;
        }

        [[nodiscard]] bool HasSlot() const noexcept
        {
            return slot_ != InvalidSlot;
        }

        [[nodiscard]] std::uint32_t OwnerNetworkId() const noexcept
        {
            return ownerNetworkId_;
        }

        [[nodiscard]] SpellSlot ItemSpellSlot() const noexcept
        {
            if (!HasSlot())
                return SpellSlot::_SlotUnknown;

            return static_cast<SpellSlot>(
                static_cast<std::int32_t>(SpellSlot::_Item1) +
                static_cast<std::int32_t>(slot_));
        }

        [[nodiscard]] std::uint32_t Id() const noexcept
        {
            return *this && api_->items->GetId != nullptr ? api_->items->GetId(handle_) : 0u;
        }

        [[nodiscard]] std::uint32_t Stacks() const noexcept
        {
            return *this && api_->items->GetStacks != nullptr ? api_->items->GetStacks(handle_) : 0u;
        }

        [[nodiscard]] std::uint32_t Charges() const noexcept
        {
            return *this && api_->items->GetCharges != nullptr ? api_->items->GetCharges(handle_) : 0u;
        }

        [[nodiscard]] std::string Name() const
        {
            return *this ? detail::ReadHandleString(handle_, api_->items->ReadName) : std::string{};
        }

    private:
        const RSDK_Api* api_{};
        RSDK_ItemHandle handle_{RSDK_INVALID_ITEM};
        std::uint8_t slot_{InvalidSlot};
        std::uint32_t ownerNetworkId_{};
    };

    namespace detail
    {
        template <typename Visitor>
        struct BuffVisitorContext final
        {
            const RSDK_Api* api{};
            Visitor* visitor{};
        };

        template <typename Visitor>
        RSDK_Bool RSDK_CALL BuffVisitorThunk(const RSDK_BuffHandle handle, void* const userData) noexcept
        {
            auto* const context = static_cast<BuffVisitorContext<Visitor>*>(userData);
            if (context == nullptr || context->visitor == nullptr)
                return RSDK_FALSE;

            try
            {
                return (*context->visitor)(Buff(context->api, handle)) ? RSDK_TRUE : RSDK_FALSE;
            }
            catch (...)
            {
                return RSDK_FALSE;
            }
        }

        template <typename Visitor>
        struct ItemVisitorContext final
        {
            const RSDK_Api* api{};
            Visitor* visitor{};
        };

        template <typename Visitor>
        RSDK_Bool RSDK_CALL ItemVisitorThunk(
            const RSDK_ItemHandle handle,
            const std::uint8_t slot,
            const std::uint32_t ownerNetworkId,
            void* const userData) noexcept
        {
            auto* const context = static_cast<ItemVisitorContext<Visitor>*>(userData);
            if (context == nullptr || context->visitor == nullptr)
                return RSDK_FALSE;

            try
            {
                return (*context->visitor)(Item(context->api, handle, slot, ownerNetworkId)) ? RSDK_TRUE : RSDK_FALSE;
            }
            catch (...)
            {
                return RSDK_FALSE;
            }
        }
    }

    class Missile final
    {
    public:
        Missile() = default;
        explicit Missile(const RSDK_MissileHandle handle) noexcept
            : Missile(detail::Runtime::Instance().Api(), handle)
        {
        }
        Missile(const RSDK_Api* api, RSDK_MissileHandle address) noexcept : api_(api), handle_(address) {}
        [[nodiscard]] explicit operator bool() const noexcept { return api_ && api_->missiles && handle_ != RSDK_INVALID_MISSILE; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] uint32_t Id() const noexcept { return *this && api_->missiles->GetId ? api_->missiles->GetId(handle_) : 0u; }
        [[nodiscard]] uint32_t NetworkId() const noexcept { return *this && api_->missiles->GetNetworkId ? api_->missiles->GetNetworkId(handle_) : 0u; }
        [[nodiscard]] int32_t Team() const noexcept { return *this && api_->missiles->GetTeam ? api_->missiles->GetTeam(handle_) : 0; }
        [[nodiscard]] float SpawnTime() const noexcept { return *this && api_->missiles->GetSpawnTime ? api_->missiles->GetSpawnTime(handle_) : 0.0f; }
        [[nodiscard]] Object Caster() const noexcept;
        [[nodiscard]] Object Target() const noexcept;
        [[nodiscard]] bool IsDeleted() const noexcept { return !*this || !api_->missiles->IsDeleted || api_->missiles->IsDeleted(handle_) != RSDK_FALSE; }
        [[nodiscard]] bool IsMissile() const noexcept { return *this && api_->missiles->IsMissile && api_->missiles->IsMissile(handle_) != RSDK_FALSE; }
        [[nodiscard]] std::string Name() const { return *this ? detail::ReadHandleString(handle_, api_->missiles->ReadName) : std::string{}; }
        [[nodiscard]] Vec3 Position() const noexcept { return Read(api_->missiles->GetPosition); }
        [[nodiscard]] Vec3 StartPosition() const noexcept { return Read(api_->missiles->GetStartPosition); }
        [[nodiscard]] Vec3 EndPosition() const noexcept { return Read(api_->missiles->GetEndPosition); }
        [[nodiscard]] Vec3 CastEndPos() const noexcept { return Read(api_->missiles->GetCastEndPosition); }
    private:
        using Fn = RSDK_Bool(RSDK_CALL*)(RSDK_MissileHandle, RSDK_Vec3*);
        [[nodiscard]] Vec3 Read(Fn fn) const noexcept { RSDK_Vec3 value{}; return *this && fn && fn(handle_, &value) != RSDK_FALSE ? Vec3(value) : Vec3{}; }
        const RSDK_Api* api_{}; RSDK_MissileHandle handle_{RSDK_INVALID_MISSILE};
    };

    class Object final
    {
    public:
        Object() = default;
        explicit Object(const RSDK_ObjectHandle handle) noexcept
            : Object(detail::Runtime::Instance().Api(), handle)
        {
        }

        Object(const RSDK_Api* api, const RSDK_ObjectHandle handle) noexcept : api_(api), handle_(handle) {}

        [[nodiscard]] explicit operator bool() const noexcept { return HasHandle(); }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] bool DoEmote(const Emotes emoteId) const noexcept
        {
            return HasHandle() && api_->objects->DoEmote != nullptr &&
                api_->objects->DoEmote(handle_, static_cast<std::uint8_t>(emoteId)) != RSDK_FALSE;
        }

#define RSDK_OBJECT_VALUE(Name, ApiName, Type, DefaultValue) \
        [[nodiscard]] Type Name() const noexcept \
        { \
            return HasHandle() && api_->objects->ApiName != nullptr ? api_->objects->ApiName(handle_) : static_cast<Type>(DefaultValue); \
        }

        RSDK_OBJECT_VALUE(Id, GetId, uint32_t, 0u)
        RSDK_OBJECT_VALUE(NetworkId, GetNetworkId, uint32_t, 0u)
        RSDK_OBJECT_VALUE(Team, GetTeam, int32_t, 0)
        RSDK_OBJECT_VALUE(IsZombie, IsZombie, bool, false)
        RSDK_OBJECT_VALUE(IsClone, IsClone, bool, false)
        RSDK_OBJECT_VALUE(PhysicalShield, GetPhysicalShield, float, 0.0f)
        RSDK_OBJECT_VALUE(BoundingRadius, GetBoundingRadius, float, 0.0f)
        RSDK_OBJECT_VALUE(Health, GetHealth, float, 0.0f)
        RSDK_OBJECT_VALUE(MaxHealth, GetMaxHealth, float, 0.0f)
        RSDK_OBJECT_VALUE(HealthPercent, GetHealthPercent, float, 0.0f)
        RSDK_OBJECT_VALUE(HealthRegenRate, GetHealthRegenRate, float, 0.0f)
        RSDK_OBJECT_VALUE(Mana, GetMana, float, 0.0f)
        RSDK_OBJECT_VALUE(MaxMana, GetMaxMana, float, 0.0f)
        RSDK_OBJECT_VALUE(ManaPercent, GetManaPercent, float, 0.0f)
        RSDK_OBJECT_VALUE(SpecialPoint, GetSpecialPoint, float, 0.0f)
        RSDK_OBJECT_VALUE(AttackRange, GetAttackRange, float, 0.0f)
        RSDK_OBJECT_VALUE(AttackSpeed, GetAttackSpeed, float, 0.0f)
        RSDK_OBJECT_VALUE(MovementSpeed, GetMovementSpeed, float, 0.0f)
        RSDK_OBJECT_VALUE(Crit, GetCrit, float, 0.0f)
        RSDK_OBJECT_VALUE(CritDamageBonusMultiplier, GetCritDamageBonusMultiplier, float, 1.0f)
        RSDK_OBJECT_VALUE(CritDamageMultiplier, GetCritDamageMultiplier, float, 2.0f)
        RSDK_OBJECT_VALUE(ArmorPenetrationPercent, GetArmorPenetrationPercent, float, 0.0f)
        RSDK_OBJECT_VALUE(ArmorPenetrationFlat, GetArmorPenetrationFlat, float, 0.0f)
        RSDK_OBJECT_VALUE(MagicPenetrationPercent, GetMagicPenetrationPercent, float, 0.0f)
        RSDK_OBJECT_VALUE(MagicPenetrationFlat, GetMagicPenetrationFlat, float, 0.0f)
        RSDK_OBJECT_VALUE(Armor, GetArmor, float, 0.0f)
        RSDK_OBJECT_VALUE(BonusArmor, GetBonusArmor, float, 0.0f)
        RSDK_OBJECT_VALUE(MagicResist, GetMagicResist, float, 0.0f)
        RSDK_OBJECT_VALUE(BonusMagicResist, GetBonusMagicResist, float, 0.0f)
        RSDK_OBJECT_VALUE(BaseAttackDamage, GetBaseAttackDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(BonusPhysicalDamage, GetBonusPhysicalDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(PercentPhysicalDamage, GetPercentPhysicalDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(TotalPhysicalDamage, GetTotalPhysicalDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(MagicDamage, GetMagicDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(PercentMagicDamage, GetPercentMagicDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(TotalMagicDamage, GetTotalMagicDamage, float, 0.0f)
        RSDK_OBJECT_VALUE(AllShield, GetAllShield, float, 0.0f)
        RSDK_OBJECT_VALUE(MagicalShield, GetMagicalShield, float, 0.0f)
        RSDK_OBJECT_VALUE(SpecialHealth, GetSpecialHealth, float, 0.0f)
        RSDK_OBJECT_VALUE(CooldownReductionPercent, GetCooldownReductionPercent, float, 0.0f)
        RSDK_OBJECT_VALUE(Level, GetLevel, uint8_t, 0u)
        RSDK_OBJECT_VALUE(TargetId, GetTargetId, uint32_t, 0u)
        RSDK_OBJECT_VALUE(TargetableTeamFlags, GetTargetableTeamFlags, uint8_t, 0u)
        RSDK_OBJECT_VALUE(IsInvulnerable, IsInvulnerable, bool, false)
        RSDK_OBJECT_VALUE(IsTargetable, IsTargetable, bool, false)
        RSDK_OBJECT_VALUE(IsMelee, IsMelee, bool, false)
        RSDK_OBJECT_VALUE(IsRanged, IsRanged, bool, false)
        RSDK_OBJECT_VALUE(IsTeleport, IsTeleport, bool, false)
        // Borrowed pointer owned by the game; do not retain across game updates.
        [[nodiscard]] const char* TeleportState() const noexcept
        {
            if (!HasHandle() || api_->objects->structSize < sizeof(RSDK_ObjectApi) ||
                api_->objects->GetTeleportState == nullptr)
                return nullptr;
            return api_->objects->GetTeleportState(handle_);
        }
        RSDK_OBJECT_VALUE(IsDead, IsDead, bool, true)
        RSDK_OBJECT_VALUE(IsDeleted, IsDeleted, bool, true)
        RSDK_OBJECT_VALUE(IsMinion, IsMinion, bool, false)
        RSDK_OBJECT_VALUE(IsCreep, IsCreep, bool, false)
        RSDK_OBJECT_VALUE(IsHero, IsHero, bool, false)
        RSDK_OBJECT_VALUE(IsTurret, IsTurret, bool, false)
        RSDK_OBJECT_VALUE(IsNexus, IsNexus, bool, false)
        RSDK_OBJECT_VALUE(IsInhibitor, IsInhibitor, bool, false)
        RSDK_OBJECT_VALUE(IsMissile, IsMissile, bool, false)
        RSDK_OBJECT_VALUE(IsVisible, IsVisible, bool, false)
        RSDK_OBJECT_VALUE(IsOnScreen, IsOnScreen, bool, false)
        RSDK_OBJECT_VALUE(IsMoving, IsMoving, bool, false)
        RSDK_OBJECT_VALUE(IsMe, IsMe, bool, false)
        RSDK_OBJECT_VALUE(ObjectType, GetObjectType, uint32_t, 0u)

#undef RSDK_OBJECT_VALUE

        [[nodiscard]] Vec3 Position() const noexcept { return ReadVec3(api_ != nullptr && api_->objects != nullptr ? api_->objects->GetPosition : nullptr); }
        [[nodiscard]] Vec3 ServerPos() const noexcept { return ReadVec3(api_ != nullptr && api_->objects != nullptr ? api_->objects->GetServerPosition : nullptr); }
        [[nodiscard]] Vec3 Direction() const noexcept { return ReadVec3(api_ != nullptr && api_->objects != nullptr ? api_->objects->GetDirection : nullptr); }

        [[nodiscard]] bool GetBounds(Vec3& outMin, Vec3& outMax) const noexcept
        {
            outMin = {};
            outMax = {};
            if (!HasHandle() || api_->objects->GetBounds == nullptr)
                return false;

            RSDK_Vec3 minValue{};
            RSDK_Vec3 maxValue{};
            if (api_->objects->GetBounds(handle_, &minValue, &maxValue) == RSDK_FALSE)
                return false;

            outMin = Vec3(minValue);
            outMax = Vec3(maxValue);
            return true;
        }

        [[nodiscard]] float ManaSpell(const SpellSlot slot) const noexcept
        {
            return HasHandle() && api_->objects->GetManaSpell != nullptr
                ? api_->objects->GetManaSpell(handle_, static_cast<int32_t>(slot))
                : 0.0f;
        }

        [[nodiscard]] std::string ObjectName() const { return CopyString(api_ != nullptr && api_->objects != nullptr ? api_->objects->ReadObjectName : nullptr); }
        [[nodiscard]] std::string Name() const { return ObjectName(); }
        [[nodiscard]] std::string ChampionName() const { return CopyString(api_ != nullptr && api_->objects != nullptr ? api_->objects->ReadChampionName : nullptr); }
        [[nodiscard]] std::string BaseSkinName() const { return CopyString(api_ != nullptr && api_->objects != nullptr ? api_->objects->ReadBaseSkinName : nullptr); }

        [[nodiscard]] bool IsEnemy(const Object& other) const noexcept
        {
            return HasHandle() && other.HasHandle() && api_->objects->IsEnemy != nullptr &&
                api_->objects->IsEnemy(handle_, other.handle_) != RSDK_FALSE;
        }

        [[nodiscard]] bool IsObjectType(const uint32_t type) const noexcept
        {
            return HasHandle() && api_->objects->IsObjectType != nullptr &&
                api_->objects->IsObjectType(handle_, type) != RSDK_FALSE;
        }

        [[nodiscard]] bool IsObjectType(const ObjectTypeFlags type) const noexcept
        {
            return IsObjectType(static_cast<uint32_t>(type));
        }

        [[nodiscard]] NavPath GetNavPath() const noexcept
        {
            if (!HasHandle() || api_->objects->GetNavPath == nullptr)
                return {};

            RSDK_NavPath data{};
            return api_->objects->GetNavPath(handle_, &data) != RSDK_FALSE ? NavPath(data) : NavPath{};
        }

        [[nodiscard]] bool IsDashing() const noexcept
        {
            return HasHandle() &&
                api_->objects->IsDashing != nullptr &&
                api_->objects->IsDashing(handle_) != RSDK_FALSE;
        }

        [[nodiscard]] std::vector<Vec3> GetPath(
            const Vec3& toPos,
            const bool smoothPath = true) const
        {
            if (!HasHandle() || api_->objects->GetPath == nullptr)
                return {};

            std::vector<Vec3> result{};
            api_->objects->GetPath(
                handle_,
                toPos,
                smoothPath ? RSDK_TRUE : RSDK_FALSE,
                &detail::Vec3SnapshotCallback,
                &result);
            return result;
        }

        [[nodiscard]] std::vector<Vec3> GetPath(
            const Vec3& fromPos,
            const Vec3& toPos,
            const bool smoothPath = true) const
        {
            if (!HasHandle() || api_->objects->GetPathFromTo == nullptr)
                return {};

            std::vector<Vec3> result{};
            api_->objects->GetPathFromTo(
                handle_,
                fromPos,
                toPos,
                smoothPath ? RSDK_TRUE : RSDK_FALSE,
                &detail::Vec3SnapshotCallback,
                &result);
            return result;
        }

        [[nodiscard]] std::vector<Vec3> CurrentPath() const
        {
            if (!HasHandle() || api_->objects->GetCurrentPath == nullptr)
                return {};

            std::vector<Vec3> result{};
            api_->objects->GetCurrentPath(handle_, &detail::Vec3SnapshotCallback, &result);
            return result;
        }

        [[nodiscard]] float AttackDelay() const noexcept
        {
            return HasHandle() && api_->objects->GetAttackDelay != nullptr
                ? api_->objects->GetAttackDelay(handle_) : 1.0f;
        }

        [[nodiscard]] float AttackCastDelay() const noexcept
        {
            return HasHandle() && api_->objects->GetAttackCastDelay != nullptr
                ? api_->objects->GetAttackCastDelay(handle_) : 0.123f;
        }

        [[nodiscard]] SpellState GetSpellState(const SpellSlot slot) const noexcept
        {
            return HasHandle() && api_->objects->GetSpellState != nullptr
                ? static_cast<SpellState>(api_->objects->GetSpellState(handle_, static_cast<int8_t>(slot)))
                : SpellState::Unknown;
        }

        [[nodiscard]] bool CanUseSpell(const SpellSlot slot) const noexcept
        {
            return HasHandle() && api_->objects->CanUseSpell != nullptr &&
                api_->objects->CanUseSpell(handle_, static_cast<int8_t>(slot)) != RSDK_FALSE;
        }

        [[nodiscard]] bool Cast(const SpellSlot slot) const noexcept
        {
            return HasHandle() && api_->objects->Cast != nullptr &&
                api_->objects->Cast(handle_, static_cast<int8_t>(slot)) != RSDK_FALSE;
        }

        [[nodiscard]] bool Cast(const SpellSlot slot, const SpellEventMode eventMode) const noexcept
        {
            if (!HasHandle())
                return false;
            if (api_->objects->CastEx != nullptr)
                return api_->objects->CastEx(handle_, static_cast<int8_t>(slot),
                    eventMode == SpellEventMode::Trigger ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
            return eventMode == SpellEventMode::Trigger && Cast(slot);
        }

        [[nodiscard]] bool Cast(const SpellSlot slot, const Vec3& position) const noexcept
        {
            return HasHandle() && api_->objects->CastPosition != nullptr &&
                api_->objects->CastPosition(handle_, static_cast<int8_t>(slot), position) != RSDK_FALSE;
        }

        [[nodiscard]] bool Cast(const SpellSlot slot, const Vec3& position, const SpellEventMode eventMode) const noexcept
        {
            if (!HasHandle())
                return false;
            if (api_->objects->CastPositionEx != nullptr)
                return api_->objects->CastPositionEx(handle_, static_cast<int8_t>(slot), position,
                    eventMode == SpellEventMode::Trigger ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
            return eventMode == SpellEventMode::Trigger && Cast(slot, position);
        }

        [[nodiscard]] bool Cast(
            const SpellSlot slot,
            const Vec3& startPos,
            const Vec3& endPos) const noexcept
        {
            return HasHandle() && api_->objects->CastFromTo != nullptr &&
                api_->objects->CastFromTo(handle_, static_cast<int8_t>(slot), startPos, endPos) != RSDK_FALSE;
        }

        [[nodiscard]] bool Cast(
            const SpellSlot slot,
            const Vec3& startPos,
            const Vec3& endPos,
            const SpellEventMode eventMode) const noexcept
        {
            if (!HasHandle())
                return false;
            if (api_->objects->CastFromToEx != nullptr)
                return api_->objects->CastFromToEx(handle_, static_cast<int8_t>(slot), startPos, endPos,
                    eventMode == SpellEventMode::Trigger ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
            return eventMode == SpellEventMode::Trigger && Cast(slot, startPos, endPos);
        }

        [[nodiscard]] bool Cast(const SpellSlot slot, const Object& target) const noexcept
        {
            return HasHandle() && target && api_->objects->CastTarget != nullptr &&
                api_->objects->CastTarget(handle_, static_cast<int8_t>(slot), target.handle_) != RSDK_FALSE;
        }

        [[nodiscard]] bool Cast(const SpellSlot slot, const Object& target, const SpellEventMode eventMode) const noexcept
        {
            if (!HasHandle() || !target)
                return false;
            if (api_->objects->CastTargetEx != nullptr)
                return api_->objects->CastTargetEx(handle_, static_cast<int8_t>(slot), target.handle_,
                    eventMode == SpellEventMode::Trigger ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
            return eventMode == SpellEventMode::Trigger && Cast(slot, target);
        }

        [[nodiscard]] bool ReleaseCharge(
            const SpellSlot slot,
            const Vec3& position,
            const bool flag = true) const noexcept
        {
            return HasHandle() && api_->objects->ReleaseCharge != nullptr &&
                api_->objects->ReleaseCharge(
                    handle_,
                    static_cast<int8_t>(slot),
                    position,
                    flag ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
        }

        [[nodiscard]] bool ReleaseCharge(
            const SpellSlot slot,
            const Vec3& position,
            const bool flag,
            const SpellEventMode eventMode) const noexcept
        {
            if (!HasHandle())
                return false;
            if (api_->objects->ReleaseChargeEx != nullptr)
                return api_->objects->ReleaseChargeEx(handle_, static_cast<int8_t>(slot), position,
                    flag ? RSDK_TRUE : RSDK_FALSE,
                    eventMode == SpellEventMode::Trigger ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
            return eventMode == SpellEventMode::Trigger && ReleaseCharge(slot, position, flag);
        }

        [[nodiscard]] std::vector<SpellData> GetAllSpells() const;
        [[nodiscard]] SpellData GetSpellData(SpellSlot slot) const noexcept;
        [[nodiscard]] SpellCast BasicAttack() const noexcept;
        [[nodiscard]] SpellCast GetActiveSpellCast() const noexcept;
        [[nodiscard]] SpellSlot GetSpellSlot(const char* spellName) const noexcept;
        [[nodiscard]] std::string GetSpellName(SpellSlot slot) const;

        [[nodiscard]] std::vector<Perk> GetAllPerks() const
        {
            if (!HasHandle() || api_->objects->GetAllPerks == nullptr)
                return {};

            std::vector<RSDK_PerkHandle> handles{};
            api_->objects->GetAllPerks(handle_, &detail::VectorSnapshotCallback<RSDK_PerkHandle>, &handles);

            std::vector<Perk> result{};
            result.reserve(handles.size());
            for (const auto value : handles)
            {
                if (value != RSDK_INVALID_PERK)
                    result.emplace_back(api_, value);
            }
            return result;
        }
        [[nodiscard]] bool HasPerk(uint32_t id) const noexcept { return HasHandle()&&api_->objects->HasPerkId&&api_->objects->HasPerkId(handle_,id)!=RSDK_FALSE; }
        [[nodiscard]] bool HasPerk(const char* name) const noexcept { return HasHandle()&&api_->objects->HasPerkName&&api_->objects->HasPerkName(handle_,name)!=RSDK_FALSE; }

        [[nodiscard]] std::vector<Buff> GetAllBuffs() const { return ReadBuffs(api_&&api_->objects?api_->objects->GetAllBuffs:nullptr,nullptr,0); }
        [[nodiscard]] std::vector<Buff> GetAllBuffs(const char* name) const { return ReadBuffsByName(api_&&api_->objects?api_->objects->GetAllBuffsByName:nullptr,name); }
        [[nodiscard]] std::vector<Buff> GetAllBuffs(BuffType type) const { return ReadBuffsByType(api_&&api_->objects?api_->objects->GetAllBuffsByType:nullptr,type); }
        [[nodiscard]] bool HasBuff(BuffType type) const noexcept { return HasHandle()&&api_->objects->HasBuffType&&api_->objects->HasBuffType(handle_,static_cast<int8_t>(type))!=RSDK_FALSE; }
        [[nodiscard]] bool HasBuff(const char* name) const noexcept { return HasHandle()&&api_->objects->HasBuffName&&api_->objects->HasBuffName(handle_,name)!=RSDK_FALSE; }
        [[nodiscard]] Buff GetFirstBuff(const char* name) const noexcept { return HasHandle()&&api_->objects->GetFirstBuffByName?Buff(api_,api_->objects->GetFirstBuffByName(handle_,name)):Buff{}; }
        [[nodiscard]] Buff GetFirstBuff(BuffType type) const noexcept { return HasHandle()&&api_->objects->GetFirstBuffByType?Buff(api_,api_->objects->GetFirstBuffByType(handle_,static_cast<int8_t>(type))):Buff{}; }

        template <typename Visitor>
        void ForEachBuff(Visitor&& visitor) const
        {
            if (!HasHandle() || api_->objects->ForEachBuff == nullptr)
                return;

            using VisitorType = std::remove_reference_t<Visitor>;
            detail::BuffVisitorContext<VisitorType> context{api_, &visitor};
            api_->objects->ForEachBuff(handle_, &detail::BuffVisitorThunk<VisitorType>, &context);
        }

        [[nodiscard]] bool HasItem(const std::uint32_t itemId) const noexcept
        {
            return HasHandle() && api_->objects->HasItemId != nullptr && api_->objects->HasItemId(handle_, itemId) != RSDK_FALSE;
        }

        [[nodiscard]] bool HasItem(const char* const itemName) const noexcept
        {
            return HasHandle() && api_->objects->HasItemName != nullptr && api_->objects->HasItemName(handle_, itemName) != RSDK_FALSE;
        }

        [[nodiscard]] Item FindItem(const std::uint32_t itemId) const noexcept
        {
            if (!HasHandle() || api_->objects->FindItemById == nullptr)
                return {};

            std::uint8_t slot = Item::InvalidSlot;
            const RSDK_ItemHandle item = api_->objects->FindItemById(handle_, itemId, &slot);
            return item != RSDK_INVALID_ITEM ? Item(api_, item, slot, NetworkId()) : Item{};
        }

        [[nodiscard]] Item FindItem(const char* const itemName) const noexcept
        {
            if (!HasHandle() || api_->objects->FindItemByName == nullptr)
                return {};

            std::uint8_t slot = Item::InvalidSlot;
            const RSDK_ItemHandle item = api_->objects->FindItemByName(handle_, itemName, &slot);
            return item != RSDK_INVALID_ITEM ? Item(api_, item, slot, NetworkId()) : Item{};
        }

        template <typename Visitor>
        void ForEachItem(Visitor&& visitor) const
        {
            if (!HasHandle() || api_->objects->ForEachItem == nullptr)
                return;

            using VisitorType = std::remove_reference_t<Visitor>;
            detail::ItemVisitorContext<VisitorType> context{api_, &visitor};
            api_->objects->ForEachItem(handle_, &detail::ItemVisitorThunk<VisitorType>, &context);
        }

        [[nodiscard]] std::vector<Item> GetAllItems() const
        {
            if (!HasHandle() || api_->objects->GetAllItems == nullptr)
                return {};

            std::vector<RSDK_ItemHandle> handles{};
            api_->objects->GetAllItems(
                handle_,
                &detail::VectorSnapshotCallback<RSDK_ItemHandle>,
                &handles);

            std::vector<Item> result{};
            result.reserve(handles.size());
            const std::uint32_t ownerNetworkId = NetworkId();
            for (std::size_t slot = 0; slot < handles.size(); ++slot)
            {
                const RSDK_ItemHandle itemHandle = handles[slot];
                if (itemHandle == RSDK_INVALID_ITEM)
                    continue;

                result.emplace_back(api_, itemHandle, static_cast<std::uint8_t>(slot), ownerNetworkId);
            }
            return result;
        }

        [[nodiscard]] bool IssueOrder(
            const GameObjectOrder order,
            const Vec3& position) const noexcept
        {
            return HasHandle() && api_->objects->IssueOrderPosition != nullptr &&
                api_->objects->IssueOrderPosition(handle_, static_cast<uint8_t>(order), position) != RSDK_FALSE;
        }

        [[nodiscard]] bool IssueOrder(
            const GameObjectOrder order,
            const Object& target) const noexcept
        {
            return HasHandle() && target && api_->objects->IssueOrderTarget != nullptr &&
                api_->objects->IssueOrderTarget(handle_, static_cast<uint8_t>(order), target.handle_) != RSDK_FALSE;
        }

        [[nodiscard]] float BaseHealthForLevel() const noexcept
        {
            return HasHandle() && api_->objects->GetBaseHealthForLevel != nullptr
                ? api_->objects->GetBaseHealthForLevel(handle_) : 599.0f;
        }

        [[nodiscard]] float RespawnTimeRemaining() const noexcept
        {
            return HasHandle() && api_->objects->GetRespawnTimeRemaining != nullptr
                ? api_->objects->GetRespawnTimeRemaining(handle_) : 9999.0f;
        }

        [[nodiscard]] Vec2 HealthBarScreenPosition() const noexcept
        {
            RSDK_Vec2 result{};
            return HasHandle() && api_->objects->GetHealthBarScreenPosition != nullptr &&
                api_->objects->GetHealthBarScreenPosition(handle_, &result) != RSDK_FALSE
                ? Vec2(result) : Vec2{};
        }

    private:
        [[nodiscard]] bool HasHandle() const noexcept
        {
            return api_ != nullptr && api_->objects != nullptr && handle_ != RSDK_INVALID_OBJECT;
        }

        using ReadStringFn = void(RSDK_CALL*)(RSDK_ObjectHandle, RSDK_StringCallback, void*);
        using ReadVec3Fn = RSDK_Bool(RSDK_CALL*)(RSDK_ObjectHandle, RSDK_Vec3*);

        [[nodiscard]] Vec3 ReadVec3(const ReadVec3Fn function) const noexcept
        {
            RSDK_Vec3 value{};
            if (!HasHandle() || function == nullptr || function(handle_, &value) == RSDK_FALSE) return {};
            return Vec3(value);
        }

        [[nodiscard]] std::string CopyString(const ReadStringFn function) const
        {
            if (!HasHandle() || function == nullptr)
                return {};

            std::string result{};
            function(handle_, &detail::StringSnapshotCallback, &result);
            return result;
        }

        using BuffListFn = void(RSDK_CALL*)(RSDK_ObjectHandle, RSDK_BuffListCallback, void*);
        using BuffNameListFn = void(RSDK_CALL*)(RSDK_ObjectHandle, const char*, RSDK_BuffListCallback, void*);
        using BuffTypeListFn = void(RSDK_CALL*)(RSDK_ObjectHandle, int8_t, RSDK_BuffListCallback, void*);

        [[nodiscard]] std::vector<Buff> ToBuffs(const std::vector<RSDK_BuffHandle>& handles) const
        {
            std::vector<Buff> result{};
            result.reserve(handles.size());
            for (const auto value : handles)
            {
                if (value != RSDK_INVALID_BUFF)
                    result.emplace_back(api_, value);
            }
            return result;
        }

        [[nodiscard]] std::vector<Buff> ReadBuffs(const BuffListFn function, const char*, int) const
        {
            if (!HasHandle() || function == nullptr)
                return {};

            std::vector<RSDK_BuffHandle> handles{};
            function(handle_, &detail::VectorSnapshotCallback<RSDK_BuffHandle>, &handles);
            return ToBuffs(handles);
        }

        [[nodiscard]] std::vector<Buff> ReadBuffsByName(const BuffNameListFn function, const char* const name) const
        {
            if (!HasHandle() || function == nullptr)
                return {};

            std::vector<RSDK_BuffHandle> handles{};
            function(handle_, name, &detail::VectorSnapshotCallback<RSDK_BuffHandle>, &handles);
            return ToBuffs(handles);
        }

        [[nodiscard]] std::vector<Buff> ReadBuffsByType(const BuffTypeListFn function, const BuffType type) const
        {
            if (!HasHandle() || function == nullptr)
                return {};

            std::vector<RSDK_BuffHandle> handles{};
            function(handle_, static_cast<int8_t>(type), &detail::VectorSnapshotCallback<RSDK_BuffHandle>, &handles);
            return ToBuffs(handles);
        }

        const RSDK_Api* api_{};
        RSDK_ObjectHandle handle_{RSDK_INVALID_OBJECT};
    };

    inline Object Buff::Caster() const noexcept
    {
        return *this && api_->buffs->GetCaster ? Object(api_, api_->buffs->GetCaster(handle_)) : Object{};
    }

    class SpellData final
    {
    public:
        SpellData() = default;
        explicit SpellData(const RSDK_SpellDataHandle handle) noexcept
            : SpellData(detail::Runtime::Instance().Api(), handle)
        {
        }
        SpellData(const RSDK_Api* api, const RSDK_SpellDataHandle handle) noexcept : api_(api), handle_(handle) {}

        [[nodiscard]] explicit operator bool() const noexcept { return api_ && api_->spellData && handle_ != RSDK_INVALID_SPELL_DATA; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] std::uint8_t Level() const noexcept { return *this && api_->spellData->GetLevel ? api_->spellData->GetLevel(handle_) : 0u; }
        [[nodiscard]] float CooldownExpires() const noexcept { return *this && api_->spellData->GetCooldownExpires ? api_->spellData->GetCooldownExpires(handle_) : 0.0f; }
        [[nodiscard]] float CooldownExpires2() const noexcept { return *this && api_->spellData->GetCooldownExpires2 ? api_->spellData->GetCooldownExpires2(handle_) : 0.0f; }
        [[nodiscard]] float CooldownTime() const noexcept { return *this && api_->spellData->GetCooldownTime ? api_->spellData->GetCooldownTime(handle_) : 0.0f; }
        [[nodiscard]] std::uint8_t Ammo() const noexcept { return *this && api_->spellData->GetAmmo ? api_->spellData->GetAmmo(handle_) : 0u; }
        [[nodiscard]] std::uint8_t ToggleState() const noexcept { return *this && api_->spellData->GetToggleState ? api_->spellData->GetToggleState(handle_) : 0u; }
        [[nodiscard]] float ProjectileSpeed() const noexcept { return *this && api_->spellData->GetProjectileSpeed ? api_->spellData->GetProjectileSpeed(handle_) : 0.0f; }
        [[nodiscard]] float Range() const noexcept { return *this && api_->spellData->GetRange ? api_->spellData->GetRange(handle_) : 0.0f; }
        [[nodiscard]] float Radius() const noexcept { return *this && api_->spellData->GetRadius ? api_->spellData->GetRadius(handle_) : 0.0f; }
        [[nodiscard]] float Width() const noexcept { return *this && api_->spellData->GetWidth ? api_->spellData->GetWidth(handle_) : 0.0f; }
        [[nodiscard]] float BaseCooldownTime() const noexcept { return *this && api_->spellData->GetBaseCooldownTime ? api_->spellData->GetBaseCooldownTime(handle_) : 0.0f; }
        [[nodiscard]] std::string SpellName() const { return *this ? detail::ReadHandleString(handle_, api_->spellData->ReadSpellName) : std::string{}; }

    private:
        const RSDK_Api* api_{};
        RSDK_SpellDataHandle handle_{RSDK_INVALID_SPELL_DATA};
    };

    class SpellCast final
    {
    public:
        SpellCast() = default;
        explicit SpellCast(const RSDK_SpellCastHandle handle) noexcept
            : SpellCast(detail::Runtime::Instance().Api(), handle)
        {
        }
        SpellCast(const RSDK_Api* api, const RSDK_SpellCastHandle handle) noexcept : api_(api), handle_(handle) {}

        [[nodiscard]] explicit operator bool() const noexcept { return api_ && api_->spellCasts && handle_ != RSDK_INVALID_SPELL_CAST; }
        [[nodiscard]] std::uintptr_t Address() const noexcept { return static_cast<std::uintptr_t>(handle_); }
        [[nodiscard]] Object Caster() const noexcept { return *this && api_->spellCasts->GetCaster ? Object(api_, api_->spellCasts->GetCaster(handle_)) : Object{}; }
        [[nodiscard]] std::uint32_t MissileNetworkId() const noexcept { return *this && api_->spellCasts->GetMissileNetworkId ? api_->spellCasts->GetMissileNetworkId(handle_) : 0u; }
        [[nodiscard]] Object Target() const noexcept { return *this && api_->spellCasts->GetTarget ? Object(api_, api_->spellCasts->GetTarget(handle_)) : Object{}; }
        [[nodiscard]] float AttackCastDelay() const noexcept { return *this && api_->spellCasts->GetAttackCastDelay ? api_->spellCasts->GetAttackCastDelay(handle_) : 0.0f; }
        [[nodiscard]] float AttackDelay() const noexcept { return *this && api_->spellCasts->GetAttackDelay ? api_->spellCasts->GetAttackDelay(handle_) : 0.0f; }
        [[nodiscard]] std::uint8_t Slot() const noexcept { return *this && api_->spellCasts->GetSlot ? api_->spellCasts->GetSlot(handle_) : 0u; }
        [[nodiscard]] float CastTime() const noexcept { return *this && api_->spellCasts->GetCastTime ? api_->spellCasts->GetCastTime(handle_) : 0.0f; }
        [[nodiscard]] float CastEndTime() const noexcept { return *this && api_->spellCasts->GetCastEndTime ? api_->spellCasts->GetCastEndTime(handle_) : 0.0f; }
        [[nodiscard]] float CooldownExpires() const noexcept { return *this && api_->spellCasts->GetCooldownExpires ? api_->spellCasts->GetCooldownExpires(handle_) : 0.0f; }
        [[nodiscard]] Vec3 StartPos() const noexcept { return ReadVec3(api_ && api_->spellCasts ? api_->spellCasts->GetStartPosition : nullptr); }
        [[nodiscard]] Vec3 EndPos() const noexcept { return ReadVec3(api_ && api_->spellCasts ? api_->spellCasts->GetEndPosition : nullptr); }
        [[nodiscard]] Vec3 CastEndPos() const noexcept { return ReadVec3(api_ && api_->spellCasts ? api_->spellCasts->GetCastEndPosition : nullptr); }
        [[nodiscard]] std::string SpellName() const { return *this ? detail::ReadHandleString(handle_, api_->spellCasts->ReadSpellName) : std::string{}; }
        [[nodiscard]] bool IsAutoAttack() const noexcept { return *this && api_->spellCasts->IsAutoAttack != nullptr && api_->spellCasts->IsAutoAttack(handle_) != RSDK_FALSE; }
        [[nodiscard]] float ProjectileSpeed() const noexcept { return *this && api_->spellCasts->GetProjectileSpeed ? api_->spellCasts->GetProjectileSpeed(handle_) : 0.0f; }
        [[nodiscard]] float Range() const noexcept { return *this && api_->spellCasts->GetRange ? api_->spellCasts->GetRange(handle_) : 0.0f; }
        [[nodiscard]] float Radius() const noexcept { return *this && api_->spellCasts->GetRadius ? api_->spellCasts->GetRadius(handle_) : 0.0f; }
        [[nodiscard]] float Width() const noexcept { return *this && api_->spellCasts->GetWidth ? api_->spellCasts->GetWidth(handle_) : 0.0f; }

    private:
        using ReadVec3Fn = RSDK_Bool(RSDK_CALL*)(RSDK_SpellCastHandle, RSDK_Vec3*);
        [[nodiscard]] Vec3 ReadVec3(const ReadVec3Fn function) const noexcept
        {
            RSDK_Vec3 value{};
            return *this && function != nullptr && function(handle_, &value) != RSDK_FALSE ? Vec3(value) : Vec3{};
        }

        const RSDK_Api* api_{};
        RSDK_SpellCastHandle handle_{RSDK_INVALID_SPELL_CAST};
    };

    inline std::vector<SpellData> Object::GetAllSpells() const
    {
        if (!HasHandle() || api_->objects->GetAllSpells == nullptr)
            return {};

        std::vector<RSDK_SpellDataHandle> handles{};
        api_->objects->GetAllSpells(handle_, &detail::VectorSnapshotCallback<RSDK_SpellDataHandle>, &handles);

        std::vector<SpellData> result{};
        result.reserve(handles.size());

        for (const RSDK_SpellDataHandle handle : handles)
        {
            if (handle != RSDK_INVALID_SPELL_DATA)
                result.emplace_back(api_, handle);
        }

        return result;
    }

    inline SpellData Object::GetSpellData(const SpellSlot slot) const noexcept
    {
        return HasHandle() && api_->objects->GetSpellData != nullptr
            ? SpellData(api_, api_->objects->GetSpellData(handle_, static_cast<int8_t>(slot)))
            : SpellData{};
    }

    inline SpellCast Object::BasicAttack() const noexcept
    {
        return HasHandle() && api_->objects->GetBasicAttack != nullptr
            ? SpellCast(api_, api_->objects->GetBasicAttack(handle_))
            : SpellCast{};
    }

    inline SpellCast Object::GetActiveSpellCast() const noexcept
    {
        return HasHandle() && api_->objects->GetActiveSpellCast != nullptr
            ? SpellCast(api_, api_->objects->GetActiveSpellCast(handle_))
            : SpellCast{};
    }

    inline SpellSlot Object::GetSpellSlot(const char* const spellName) const noexcept
    {
        return HasHandle() && api_->objects->GetSpellSlot != nullptr
            ? static_cast<SpellSlot>(api_->objects->GetSpellSlot(handle_, spellName))
            : SpellSlot::_SlotUnknown;
    }

    inline std::string Object::GetSpellName(const SpellSlot slot) const
    {
        if (!HasHandle() || api_->objects->ReadSpellName == nullptr)
            return {};

        std::string result{};
        api_->objects->ReadSpellName(
            handle_,
            static_cast<int8_t>(slot),
            &detail::StringSnapshotCallback,
            &result);

        return result;
    }

    namespace Config
    {
        inline void SetUseRawPointer(const bool enabled) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();

            if (api == nullptr ||
                api->config == nullptr ||
                api->config->SetUseRawPointer == nullptr)
            {
                return;
            }

            api->config->SetUseRawPointer(
                enabled ? RSDK_TRUE : RSDK_FALSE);
        }

        [[nodiscard]] inline bool UseRawPointer() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();

            if (api == nullptr ||
                api->config == nullptr ||
                api->config->GetUseRawPointer == nullptr)
            {
                return false;
            }

            return api->config->GetUseRawPointer() != RSDK_FALSE;
        }
    }

    struct IssueOrderEvent
    {
        Object sender;
        GameObjectOrder order{};
        Vec3 position{};
        Object target;
        bool isAttackMove{};
        bool process{true};
    };

    struct NewPathEvent
    {
        Object sender;
        std::vector<Vec3> path{};
        bool isDash{};
        float dashSpeed{};
        std::uint8_t dashGravityCount{};
        std::uint32_t dashGravityFlags{};
        [[nodiscard]] bool IsBlink() const noexcept { return (dashGravityFlags & 0x01u) != 0u; }
    };

    struct ProcessSpellEvent
    {
        Object sender;
        SpellCast spellCast;
    };

    struct DoCastSpellEvent
    {
        SpellCast spellCast;
        Object sender;
    };

    struct PlayAnimationEvent
    {
        Object sender;
        std::string animationName;
    };

    struct StopCastEvent
    {
        Object sender;
        SpellCast spellCast;
        bool stopAnimation{};
        bool destroyMissile{};
        std::uint32_t missileNetworkId{};
    };

    struct CastSpellEvent
    {
        Object sender;
        Object target;
        SpellData spellData;
        Vec3 sourcePosition{};
        Vec3 destinationPosition{};
        Vec3 castEndPosition{};
        Vec3 unknownPosition{};
        std::uint32_t sourceId{};
        std::uint32_t targetId{};
        SpellSlot slot{SpellSlot::_SlotUnknown};
        bool process{true};
    };

    struct UpdateChargeableSpellEvent
    {
        Object sender;
        SpellData spellData;
        SpellSlot slot{};
        Vec3 position{};
        bool flag{};
        bool process{true};
    };

    struct BuffAddRemoveEvent
    {
        Object sender;
        Buff buff;
        bool added{};
    };

    struct UpdateBuffEvent
    {
        Object sender;
        Buff buff;
        std::int32_t stacks{};
    };

    struct SpellImpactEvent
    {
        Object sender;
        Object target;
        SpellCast spellCast;
        std::int32_t nativeArgument{};
    };

    // Lazy menu-localization descriptor. Pass rsdk::Tr("key", "Fallback") directly to
    // existing menu Add*/SetTooltip calls. The menu stores the key/fallback and resolves it
    // on every render, so changing the locale is immediate and does not require plugin reload.
    class LocalizedText final
    {
    public:
        LocalizedText(const char* key, const char* fallback = nullptr)
            : key_(key != nullptr ? key : ""), fallback_(fallback != nullptr ? fallback : "")
        {
        }

        [[nodiscard]] const std::string& Key() const noexcept { return key_; }
        [[nodiscard]] const std::string& Fallback() const noexcept { return fallback_; }

        [[nodiscard]] const char* Encoded() const
        {
            encoded_.clear();
            encoded_.reserve(10u + key_.size() + fallback_.size());
            encoded_ += "\036RSDK_TR\037";
            encoded_ += key_;
            encoded_.push_back('\x1F');
            encoded_ += fallback_;
            return encoded_.c_str();
        }

        [[nodiscard]] operator const char*() const { return Encoded(); }

    private:
        std::string key_;
        std::string fallback_;
        mutable std::string encoded_;
    };

    [[nodiscard]] inline LocalizedText Tr(const char* key, const char* fallback = nullptr)
    {
        return LocalizedText(key, fallback);
    }

    namespace Localization
    {
        [[nodiscard]] inline std::string Tr(const char* key, const char* fallback = nullptr)
        {
            std::string output;
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->localization == nullptr || api->localization->ReadText == nullptr)
                return fallback != nullptr && *fallback != '\0' ? std::string(fallback) : (key != nullptr ? std::string(key) : std::string{});

            api->localization->ReadText(key, fallback, &detail::StringSnapshotCallback, &output);
            return output;
        }

        [[nodiscard]] inline std::string Locale()
        {
            std::string output;
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->localization != nullptr && api->localization->ReadLocale != nullptr)
                api->localization->ReadLocale(&detail::StringSnapshotCallback, &output);
            return output.empty() ? std::string("en-US") : output;
        }

        [[nodiscard]] inline const char* Text(const char* key, const char* fallback = nullptr)
        {
            thread_local std::unordered_map<std::string, std::string> cache;
            const std::string locale = Locale();
            const std::string cacheKey = locale + '\x1F' + (key != nullptr ? key : "") + '\x1F' + (fallback != nullptr ? fallback : "");
            auto [it, inserted] = cache.try_emplace(cacheKey);
            (void)inserted;
            it->second = Tr(key, fallback);
            return it->second.c_str();
        }

        inline bool SetLocale(const char* locale) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr && api->localization != nullptr && api->localization->SetLocale != nullptr &&
                api->localization->SetLocale(locale) != RSDK_FALSE;
        }

        inline bool Reload() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr && api->localization != nullptr && api->localization->Reload != nullptr &&
                api->localization->Reload() != RSDK_FALSE;
        }
    }

    inline Object Missile::Caster() const noexcept
    {
        return *this && api_->missiles->GetCaster ? Object(api_, api_->missiles->GetCaster(handle_)) : Object{};
    }

    inline Object Missile::Target() const noexcept
    {
        return *this && api_->missiles->GetTarget ? Object(api_, api_->missiles->GetTarget(handle_)) : Object{};
    }

    namespace ObjectManager
    {
        using ListFunction = void(RSDK_CALL*)(uint8_t, RSDK_ObjectListCallback, void*);
        using ObjectListFunction = void(RSDK_CALL*)(RSDK_ObjectListCallback, void*);
        using MissileListFunction = void(RSDK_CALL*)(uint8_t, RSDK_MissileListCallback, void*);

        [[nodiscard]] inline uint8_t ToApiFilter(const TeamFilter filter) noexcept
        {
            return static_cast<uint8_t>(filter);
        }

        [[nodiscard]] inline std::vector<Object> ToObjects(
            const RSDK_Api* const api,
            const std::vector<RSDK_ObjectHandle>& handles)
        {
            std::vector<Object> result{};
            result.reserve(handles.size());
            for (const auto handle : handles)
            {
                if (handle != RSDK_INVALID_OBJECT)
                    result.emplace_back(api, handle);
            }
            return result;
        }

        [[nodiscard]] inline std::vector<Object> ReadList(const ListFunction function, const TeamFilter filter)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->objectManager == nullptr || function == nullptr)
                return {};

            std::vector<RSDK_ObjectHandle> handles{};
            function(ToApiFilter(filter), &detail::VectorSnapshotCallback<RSDK_ObjectHandle>, &handles);
            return ToObjects(api, handles);
        }

        [[nodiscard]] inline std::vector<Object> ReadObjects(const ObjectListFunction function)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->objectManager == nullptr || function == nullptr)
                return {};

            std::vector<RSDK_ObjectHandle> handles{};
            function(&detail::VectorSnapshotCallback<RSDK_ObjectHandle>, &handles);
            return ToObjects(api, handles);
        }

        [[nodiscard]] inline Object LocalPlayer() noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api != nullptr && api->objectManager != nullptr && api->objectManager->GetLocalPlayer != nullptr
                ? Object(api, api->objectManager->GetLocalPlayer()) : Object{};
        }

        [[nodiscard]] inline Object FindObjectByNetworkId(const uint32_t networkId) noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api != nullptr && api->objectManager != nullptr && api->objectManager->FindObjectByNetworkId != nullptr
                ? Object(api, api->objectManager->FindObjectByNetworkId(networkId)) : Object{};
        }

        [[nodiscard]] inline Object FindObjectById(const uint32_t id) noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api != nullptr && api->objectManager != nullptr && api->objectManager->FindObjectById != nullptr
                ? Object(api, api->objectManager->FindObjectById(id)) : Object{};
        }

        [[nodiscard]] inline Missile FindMissileByNetworkId(const uint32_t networkId) noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api != nullptr && api->objectManager != nullptr && api->objectManager->FindMissileByNetworkId != nullptr
                ? Missile(api, api->objectManager->FindMissileByNetworkId(networkId)) : Missile{};
        }

        [[nodiscard]] inline Missile FindMissileById(const uint32_t id) noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api != nullptr && api->objectManager != nullptr && api->objectManager->FindMissileById != nullptr
                ? Missile(api, api->objectManager->FindMissileById(id)) : Missile{};
        }

        [[nodiscard]] inline std::vector<Object> AllHeroes(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllHeroes, filter) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AttackableUnits(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllAttackableUnits, filter) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AllMinions(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllMinions, filter) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AllTurrets(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllTurrets, filter) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AllBuildingObjects(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllBuildingObjects, filter) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AllObjects() { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadObjects(api->objectManager->GetAllObjects) : std::vector<Object>{}; }
        [[nodiscard]] inline std::vector<Object> AllInhibitors(const TeamFilter filter = TeamFilter::All) { const auto* api = detail::Runtime::Instance().Api(); return api && api->objectManager ? ReadList(api->objectManager->GetAllInhibitors, filter) : std::vector<Object>{}; }

        [[nodiscard]] inline std::vector<Missile> AllMissiles(const TeamFilter filter = TeamFilter::All)
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->objectManager == nullptr || api->objectManager->GetAllMissiles == nullptr)
                return {};

            std::vector<RSDK_MissileHandle> handles{};
            api->objectManager->GetAllMissiles(
                ToApiFilter(filter),
                &detail::VectorSnapshotCallback<RSDK_MissileHandle>,
                &handles);

            std::vector<Missile> result{};
            result.reserve(handles.size());
            for (const auto value : handles)
            {
                if (value != RSDK_INVALID_MISSILE)
                    result.emplace_back(api, value);
            }
            return result;
        }

        [[nodiscard]] inline Object EnemyNexus() noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api && api->objectManager && api->objectManager->GetEnemyNexus ? Object(api, api->objectManager->GetEnemyNexus()) : Object{};
        }

        [[nodiscard]] inline Object TeamNexus() noexcept
        {
            const auto* api = detail::Runtime::Instance().Api();
            return api && api->objectManager && api->objectManager->GetTeamNexus ? Object(api, api->objectManager->GetTeamNexus()) : Object{};
        }
    }

    namespace Game
    {
        [[nodiscard]] inline float TimeGame() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->GetTime?a->game->GetTime():0.0f; }
        [[nodiscard]] inline float Time() noexcept { return TimeGame(); } // Compatibility alias.
        [[nodiscard]] inline int32_t Latency() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->GetLatency?a->game->GetLatency():0; }
        inline void PrintChat(const char* text) noexcept { const auto* a=detail::Runtime::Instance().Api(); if(a&&a->game&&a->game->PrintChat)a->game->PrintChat(text); }
        [[nodiscard]] inline Vec3 CursorPosition() noexcept { RSDK_Vec3 v{}; const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->GetCursorPosition&&a->game->GetCursorPosition(&v)!=RSDK_FALSE?Vec3(v):Vec3{}; }
        [[nodiscard]] inline int32_t State() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->GetState?a->game->GetState():-1; }
        [[nodiscard]] inline bool IsReady() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsReady&&a->game->IsReady()!=RSDK_FALSE; }
        [[nodiscard]] inline bool IsFocused() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsFocused&&a->game->IsFocused()!=RSDK_FALSE; }
        [[nodiscard]] inline bool IsChatOpen() noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsChatOpen&&a->game->IsChatOpen()!=RSDK_FALSE; }
        [[nodiscard]] inline uint16_t CollisionFlag(const Vec3& position) noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->GetCollisionFlag?a->game->GetCollisionFlag(position):0u; }
        [[nodiscard]] inline bool IsWall(const Vec3& position) noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsWall&&a->game->IsWall(position)!=RSDK_FALSE; }
        [[nodiscard]] inline bool IsGrass(const Vec3& position) noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsGrass&&a->game->IsGrass(position)!=RSDK_FALSE; }
        [[nodiscard]] inline bool IsInFow(const Vec3& position) noexcept { const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->IsInFow&&a->game->IsInFow(position)!=RSDK_FALSE; }
        [[nodiscard]] inline Vec2 WorldToScreen(const Vec3& position) noexcept { RSDK_Vec2 v{}; const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->WorldToScreen&&a->game->WorldToScreen(position,&v)!=RSDK_FALSE?Vec2(v):Vec2{}; }
        [[nodiscard]] inline Vec2 WorldToMinimap(const Vec3& position) noexcept { RSDK_Vec2 v{}; const auto* a=detail::Runtime::Instance().Api(); return a&&a->game&&a->game->WorldToMinimap&&a->game->WorldToMinimap(position,&v)!=RSDK_FALSE?Vec2(v):Vec2{}; }

    }


}
