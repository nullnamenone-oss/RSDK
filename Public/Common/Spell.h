#pragma once

#include "../RSDK.h"
#include "MinionManager.h"
#include "HealthPrediction.h"
#include "HeroManager.h"
#include "Prediction.h"
#include "TargetSelector.h"
#include "Utility.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace rsdk
{
    enum class SpellCastState : std::uint8_t
    {
        SuccessfullyCasted,
        NotReady,
        NotCasted,
        InvalidTarget,
        OutOfRange,
        Collision,
        NotEnoughTargets,
        LowHitChance,
        PredictionUnavailable,
        Throttled
    };

    enum class ChargedReleaseMode : std::uint8_t
    {
        UpdateOnly,
        UpdateThenCast
    };

    class Spell final
    {
    public:
        Spell() = default;

        explicit Spell(
            const SpellSlot slot,
            const float range = std::numeric_limits<float>::max(),
            const DamageType damageType = DamageType::Physical) noexcept
            : slot_(slot), damageType_(damageType), range_(range)
        {
        }

        Spell(const Spell&) = delete;
        Spell& operator=(const Spell&) = delete;
        Spell(Spell&&) = delete;
        Spell& operator=(Spell&&) = delete;

        ~Spell() = default;

        [[nodiscard]] SpellSlot Slot() const noexcept { return slot_; }
        void SetSlot(const SpellSlot value) noexcept { slot_ = value; }

        [[nodiscard]] DamageType GetDamageType() const noexcept { return damageType_; }
        void SetDamageType(const DamageType value) noexcept { damageType_ = value; }

        [[nodiscard]] bool IsSkillshot() const noexcept { return isSkillshot_; }
        [[nodiscard]] bool IsChargedSpell() const noexcept { return isChargedSpell_; }
        [[nodiscard]] ChargedReleaseMode GetChargedReleaseMode() const noexcept { return chargedReleaseMode_; }
        [[nodiscard]] bool IsChannelProtected() const noexcept { return isChannelProtected_; }
        [[nodiscard]] bool IsChanneling() const noexcept
        {
            return isChannelProtected_ && isChanneling_ && !channelManualCancelRequested_;
        }
        [[nodiscard]] bool Collision() const noexcept { return collision_; }
        [[nodiscard]] float Delay() const noexcept { return delay_; }
        [[nodiscard]] float Width() const noexcept { return width_; }
        [[nodiscard]] float Speed() const noexcept { return speed_; }
        [[nodiscard]] SkillshotType Type() const noexcept { return type_; }
        [[nodiscard]] HitChance MinHitChance() const noexcept { return minHitChance_; }
        [[nodiscard]] CollisionableObjects CollisionObjects() const noexcept { return collisionObjects_; }
        [[nodiscard]] float LastCastAttemptTime() const noexcept { return lastCastAttemptTime_; }
        [[nodiscard]] float LastCastRequestTime() const noexcept { return lastCastRequestTime_; }
        [[nodiscard]] float CastThrottle() const noexcept { return castThrottle_; }

        // Interval is applied to this Spell instance, not globally across DLLs.
        // Cast(..., 0.0f) bypasses this interval; champion logic must prevent repeated recasts.
        Spell& SetCastThrottle(const float seconds) noexcept
        {
            if (std::isfinite(seconds))
                castThrottle_ = std::max(0.0f, seconds);
            return *this;
        }

        // Dynamic spell parameters. Values use the same units/semantics as SetSkillshot/SetTargeted.
        // SetRange changes the normal base range; charged spells use chargedMinRange_/chargedMaxRange_.
        Spell& SetRange(const float value) noexcept { range_ = value; return *this; }
        Spell& SetWidth(const float value) noexcept { width_ = value; return *this; }
        Spell& SetDelay(const float value) noexcept { delay_ = value; return *this; }
        Spell& SetSpeed(const float value) noexcept { speed_ = value; return *this; }

        Spell& SetMinHitChance(const HitChance value) noexcept { minHitChance_ = value; return *this; }
        Spell& SetCollisionObjects(const CollisionableObjects value) noexcept { collisionObjects_ = value; return *this; }
        Spell& SetAddSelfHitboxToRange(const bool value) noexcept { addSelfHitboxToRange_ = value; return *this; }
        Spell& SetAddEnemyHitboxToRange(const bool value) noexcept { addEnemyHitboxToRange_ = value; return *this; }
        Spell& SetChargedReleaseMode(const ChargedReleaseMode value) noexcept { chargedReleaseMode_ = value; return *this; }

        [[nodiscard]] Vec3 From() const noexcept
        {
            if (from_.IsValid())
                return from_;
            const Object player = ObjectManager::LocalPlayer();
            if (!player)
                return {};
            const Vec3 serverPosition = player.ServerPos();
            return serverPosition.IsValid() ? serverPosition : player.Position();
        }

        [[nodiscard]] Vec3 RangeCheckFrom() const noexcept
        {
            if (rangeCheckFrom_.IsValid())
                return rangeCheckFrom_;
            const Object player = ObjectManager::LocalPlayer();
            if (!player)
                return {};
            const Vec3 serverPosition = player.ServerPos();
            return serverPosition.IsValid() ? serverPosition : player.Position();
        }

        Spell& UpdateSourcePosition(Vec3 from = {}, Vec3 rangeCheckFrom = {}) noexcept
        {
            from_ = from;
            rangeCheckFrom_ = rangeCheckFrom;
            return *this;
        }

        [[nodiscard]] float Range() const noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            float baseRange = range_;
            if (addSelfHitboxToRange_ && player)
                baseRange += player.BoundingRadius();

            if (!isChargedSpell_)
                return baseRange;

            if (!IsCharging())
                return chargedMaxRange_;

            const float duration = std::max(chargeDuration_, 1.0e-4f);
            const float elapsed = std::max(0.0f, Game::TimeGame() - chargedCastedTime_);
            const float delta = chargedMaxRange_ - chargedMinRange_;
            // LeagueSharp subtracts 150 range units while charging to avoid edge casts.
            return chargedMinRange_ + std::min(delta, elapsed * delta / duration - 150.0f);
        }

        [[nodiscard]] float RangeSqr() const noexcept
        {
            const float value = Range();
            return value * value;
        }

        [[nodiscard]] float WidthSqr() const noexcept { return width_ * width_; }

        [[nodiscard]] bool IsReady() const noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            return player && player.CanUseSpell(slot_);
        }

        [[nodiscard]] float GetRange(const Object& target) const noexcept
        {
            float result = Range();
            if (addEnemyHitboxToRange_ && target)
                result += target.BoundingRadius();
            return result;
        }

        [[nodiscard]] float GetRangeSqr(const Object& target) const noexcept
        {
            const float result = GetRange(target);
            return result * result;
        }

        [[nodiscard]] bool CanCast(const Object& unit) const noexcept
        {
            const Vec3 from = RangeCheckFrom();
            return IsReady() && Utility::IsValidTarget(unit, Range(), true, &from);
        }

        [[nodiscard]] std::uint8_t Level() const noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            const SpellData data = player ? player.GetSpellData(slot_) : SpellData{};
            return data ? data.Level() : 0u;
        }

        [[nodiscard]] float ManaCost() const noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            return player ? player.ManaSpell(slot_) : 0.0f;
        }

        [[nodiscard]] float Cooldown() const noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            const SpellData data = player ? player.GetSpellData(slot_) : SpellData{};
            if (!data)
                return 0.0f;
            return std::max(0.0f, data.CooldownExpires() - Game::TimeGame());
        }

        Spell& SetSkillshot(
            const float delay,
            const float width,
            const float speed,
            const bool collision,
            const SkillshotType type,
            Vec3 from = {},
            Vec3 rangeCheckFrom = {}) noexcept
        {
            delay_ = delay;
            width_ = width;
            speed_ = speed;
            collision_ = collision;
            type_ = type;
            from_ = from;
            rangeCheckFrom_ = rangeCheckFrom;
            isSkillshot_ = true;
            return *this;
        }

        Spell& SetTargeted(
            const float delay,
            const float speed,
            Vec3 from = {},
            Vec3 rangeCheckFrom = {}) noexcept
        {
            delay_ = delay;
            speed_ = speed;
            from_ = from;
            rangeCheckFrom_ = rangeCheckFrom;
            isSkillshot_ = false;
            return *this;
        }

        bool SetCharged(
            std::string spellName,
            std::string buffName,
            const float minRange,
            const float maxRange,
            const float deltaTime)
        {
            chargedSpellName_ = std::move(spellName);
            chargedBuffName_ = std::move(buffName);
            chargedMinRange_ = minRange;
            chargedMaxRange_ = maxRange;
            chargeDuration_ = deltaTime;
            chargedCastedTime_ = 0.0f;
            isChargedSpell_ = true;

            processSpellConnection_.Disconnect();
            updateChargeConnection_.Disconnect();
            chargedCastSpellConnection_.Disconnect();

            processSpellConnection_ = Events::OnProcessSpell.Connect([this](const ProcessSpellEvent& event)
            {
                OnProcessSpell(event);
            });
            updateChargeConnection_ = Events::OnUpdateChargeableSpell.Connect([this](UpdateChargeableSpellEvent& event)
            {
                OnUpdateChargeableSpell(event);
            });
            chargedCastSpellConnection_ = Events::OnCastSpell.Connect([this](CastSpellEvent& event)
            {
                OnChargedCastSpell(event);
            });

            const bool connected = static_cast<bool>(processSpellConnection_) && static_cast<bool>(updateChargeConnection_) &&
                static_cast<bool>(chargedCastSpellConnection_);
            if (!connected)
            {
                processSpellConnection_.Disconnect();
                updateChargeConnection_.Disconnect();
                chargedCastSpellConnection_.Disconnect();
                isChargedSpell_ = false;
            }
            return connected;
        }

        bool SetChannelProtection(
            const bool allowUserCancel,
            const bool usePositionCast,
            const bool allowOtherSpellsToCancel = false)
        {
            allowUserChannelCancel_ = allowUserCancel;
            channelUsePositionCast_ = usePositionCast;
            allowOtherSpellsToCancelChannel_ = allowOtherSpellsToCancel;
            isChanneling_ = false;
            channelManualCancelRequested_ = false;
            activeChannelSpellName_ = {};
            activeChannelEndObjectName_ = {};
            activeChannelEndsOnCreate_ = false;
            channelCastRequestTime_ = -1000.0f;

            channelDoCastConnection_.Disconnect();
            channelStopCastConnection_.Disconnect();
            channelCreateObjectConnection_.Disconnect();
            channelDeleteObjectConnection_.Disconnect();
            channelIssueOrderConnection_.Disconnect();
            channelCastSpellConnection_.Disconnect();
            channelWndProcConnection_.Disconnect();

            channelDoCastConnection_ = Events::OnDoCast.Connect([this](const DoCastSpellEvent& event)
            {
                OnChannelDoCast(event);
            });
            channelStopCastConnection_ = Events::OnStopCast.Connect([this](const StopCastEvent& event)
            {
                OnChannelStopCast(event);
            });
            channelDeleteObjectConnection_ = Events::OnDeleteObject.Connect([this](const std::uintptr_t address)
            {
                OnChannelDeleteObject(address);
            });
            channelIssueOrderConnection_ = Events::OnIssueOrder.Connect([this](IssueOrderEvent& event)
            {
                OnChannelIssueOrder(event);
            });
            channelCastSpellConnection_ = Events::OnCastSpell.Connect([this](CastSpellEvent& event)
            {
                OnChannelCastSpell(event);
            });

            const Object player = ObjectManager::LocalPlayer();
            const bool needsCreateObject = player && player.ChampionName() == "Fiddlesticks";
            if (needsCreateObject)
            {
                channelCreateObjectConnection_ = Events::OnCreateObject.Connect([this](const std::uintptr_t address, const std::uint32_t networkId)
                {
                    (void)networkId;
                    OnChannelCreateObject(address);
                });
            }

            if (allowUserChannelCancel_)
            {
                channelWndProcConnection_ = Events::OnWndProc.Connect([this](const RSDK_WndProcEvent& event)
                {
                    OnChannelWndProc(event);
                });
            }

            const bool connected = static_cast<bool>(channelDoCastConnection_) && static_cast<bool>(channelStopCastConnection_) &&
                static_cast<bool>(channelDeleteObjectConnection_) && static_cast<bool>(channelIssueOrderConnection_) &&
                static_cast<bool>(channelCastSpellConnection_) &&
                (!needsCreateObject || static_cast<bool>(channelCreateObjectConnection_)) &&
                (!allowUserChannelCancel_ || static_cast<bool>(channelWndProcConnection_));
            if (!connected)
            {
                channelDoCastConnection_.Disconnect();
                channelStopCastConnection_.Disconnect();
                channelCreateObjectConnection_.Disconnect();
                channelDeleteObjectConnection_.Disconnect();
                channelIssueOrderConnection_.Disconnect();
                channelCastSpellConnection_.Disconnect();
                channelWndProcConnection_.Disconnect();
                isChannelProtected_ = false;
                isChanneling_ = false;
                activeChannelSpellName_ = {};
                activeChannelEndObjectName_ = {};
                activeChannelEndsOnCreate_ = false;
                return false;
            }

            isChannelProtected_ = true;
            return true;
        }

        void DisableChannelProtection() noexcept
        {
            channelDoCastConnection_.Disconnect();
            channelStopCastConnection_.Disconnect();
            channelCreateObjectConnection_.Disconnect();
            channelDeleteObjectConnection_.Disconnect();
            channelIssueOrderConnection_.Disconnect();
            channelCastSpellConnection_.Disconnect();
            channelWndProcConnection_.Disconnect();
            isChannelProtected_ = false;
            isChanneling_ = false;
            channelManualCancelRequested_ = false;
            activeChannelSpellName_ = {};
            activeChannelEndObjectName_ = {};
            activeChannelEndsOnCreate_ = false;
        }

        [[nodiscard]] bool IsCharging() const noexcept
        {
            if (!isChargedSpell_ || !IsReady())
                return false;

            const Object player = ObjectManager::LocalPlayer();
            if (!player)
                return false;

            if (!chargedBuffName_.empty() && player.HasBuff(chargedBuffName_.c_str()))
                return true;

            const float grace = 0.300f + static_cast<float>(Game::Latency()) / 1000.0f;
            return Game::TimeGame() - chargedCastedTime_ < grace;
        }

        bool StartCharging() noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            if (!player || IsCharging())
                return false;

            const float now = Game::TimeGame();
            if (now - chargedRequestTime_ <= 0.400f + static_cast<float>(Game::Latency()) / 1000.0f)
                return false;

            const bool result = player.Cast(slot_);
            if (result)
                chargedRequestTime_ = now;
            return result;
        }

        bool StartCharging(const Vec3& position) noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            if (!player || IsCharging())
                return false;

            const float now = Game::TimeGame();
            if (now - chargedRequestTime_ <= 0.400f + static_cast<float>(Game::Latency()) / 1000.0f)
                return false;

            const bool result = player.Cast(slot_, position);
            if (result)
                chargedRequestTime_ = now;
            return result;
        }

        bool ReleaseCharge(const Vec3& position, const bool releaseCast = true) noexcept
        {
            const Object player = ObjectManager::LocalPlayer();
            if (!player || !isChargedSpell_)
                return false;

            const bool updated = player.ReleaseCharge(slot_, position, releaseCast, SpellEventMode::Bypass);
            if (chargedReleaseMode_ != ChargedReleaseMode::UpdateThenCast)
                return updated;

            // LeagueSharp ShootChargedSpell follows UpdateChargedSpell with CastSpell(..., false).
            // Keep that behavior opt-in because most charged spells only need the update/release call.
            const bool casted = player.Cast(slot_, position, SpellEventMode::Bypass);
            return updated || casted;
        }

        [[nodiscard]] bool IsInRange(const Vec3& point, const float range = -1.0f) const noexcept
        {
            const float useRange = range < 0.0f ? Range() : range;
            return RangeCheckFrom().DistanceToSqr(point) < useRange * useRange;
        }

        [[nodiscard]] bool IsInRange(const Object& object, const float range = -1.0f) const noexcept
        {
            if (!object)
                return false;
            const float useRange = range < 0.0f ? GetRange(object) : range;
            const Vec3 objectPosition = object.ServerPos().IsValid() ? object.ServerPos() : object.Position();
            return RangeCheckFrom().DistanceToSqr(objectPosition) <= useRange * useRange;
        }

        [[nodiscard]] Object GetTarget(
            const float extraRange = 0.0f,
            const TargetSelector::TargetFilter filter = nullptr) const
        {
            const Vec3 from = From();
            return TargetSelector::GetTarget(Range() + extraRange, damageType_, true, &from, filter);
        }

        [[nodiscard]] Object GetTarget(
            const float extraRange,
            const std::vector<Object>& ignoredChamps,
            const TargetSelector::TargetFilter filter = nullptr) const
        {
            const Vec3 from = From();
            return TargetSelector::GetTarget(Range() + extraRange, damageType_, true, ignoredChamps, &from, filter);
        }

        [[nodiscard]] PredictionOutput GetPrediction(const Object& unit, const bool aoe = false, const float overrideRange = -1.0f) const
        {
            return GetPrediction(unit, aoe, overrideRange, collisionObjects_);
        }

        [[nodiscard]] PredictionOutput GetPrediction(
            const Object& unit,
            const bool aoe,
            const float overrideRange,
            const CollisionableObjects collisionObjects) const
        {
            PredictionInput input{};
            input.Unit = unit;
            input.Aoe = aoe;
            input.Collision = collision_;
            input.CollisionObjects = collisionObjects;
            input.Delay = delay_;
            input.Radius = width_;
            input.Range = overrideRange >= 0.0f ? overrideRange : Range();
            input.Speed = speed_;
            input.Type = type_;
            input.From = From();
            input.RangeCheckFrom = RangeCheckFrom();
            return Prediction::GetPrediction(input);
        }

        [[nodiscard]] std::vector<Object> GetCollision(
            const Vec3& from,
            const std::vector<Vec3>& positions,
            const float delayOverride = -1.0f) const
        {
            PredictionInput input{};
            input.Collision = true;
            input.CollisionObjects = collisionObjects_;
            input.Delay = delayOverride > 0.0f ? delayOverride : delay_;
            input.Radius = width_;
            input.Range = Range();
            input.Speed = speed_;
            input.Type = type_;
            input.From = from;
            input.RangeCheckFrom = RangeCheckFrom();
            return Prediction::GetCollision(positions, input);
        }

        [[nodiscard]] std::vector<Object> GetCollision(
            const Vec3& from,
            const Vec3& position,
            const float delayOverride = -1.0f) const
        {
            return GetCollision(from, std::vector<Vec3>{position}, delayOverride);
        }

        [[nodiscard]] SpellCastState Cast(const Object& unit, const bool aoe = false)
        {
            return CastInternal(unit, minHitChance_, false, aoe, -1, castThrottle_);
        }

        // Keep the existing bool aoe overload unambiguous; pass throttle as the third argument.
        [[nodiscard]] SpellCastState Cast(const Object& unit, const bool aoe, const float throttleSeconds)
        {
            return CastInternal(unit, minHitChance_, false, aoe, -1, throttleSeconds);
        }

        [[nodiscard]] SpellCastState Cast(const Object& unit, const HitChance minimumHitChance, const bool aoe = false)
        {
            return CastInternal(unit, minimumHitChance, false, aoe, -1, castThrottle_);
        }

        [[nodiscard]] SpellCastState Cast(const Object& unit, const HitChance minimumHitChance, const bool aoe, const float throttleSeconds)
        {
            return CastInternal(unit, minimumHitChance, false, aoe, -1, throttleSeconds);
        }

        [[nodiscard]] bool CastIfHitchanceEquals(const Object& unit, const HitChance hitChance)
        {
            return CastInternal(unit, hitChance, true, false, -1, castThrottle_) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] bool CastIfHitchanceEquals(const Object& unit, const HitChance hitChance, const float throttleSeconds)
        {
            return CastInternal(unit, hitChance, true, false, -1, throttleSeconds) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] bool CastIfHitchanceMinimum(const Object& unit, const HitChance hitChance)
        {
            return CastInternal(unit, hitChance, false, false, -1, castThrottle_) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] bool CastIfHitchanceMinimum(const Object& unit, const HitChance hitChance, const float throttleSeconds)
        {
            return CastInternal(unit, hitChance, false, false, -1, throttleSeconds) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] bool CastIfWillHit(const Object& unit, const int minTargets = 5)
        {
            return CastInternal(unit, minHitChance_, false, true, minTargets, castThrottle_) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] bool CastIfWillHit(const Object& unit, const int minTargets, const float throttleSeconds)
        {
            return CastInternal(unit, minHitChance_, false, true, minTargets, throttleSeconds) == SpellCastState::SuccessfullyCasted;
        }

        [[nodiscard]] SpellCastState CastOnBestTarget(const float extraRange = 0.0f, const bool aoe = false)
        {
            return CastOnBestTarget(extraRange, aoe, castThrottle_);
        }

        [[nodiscard]] SpellCastState CastOnBestTarget(const float extraRange, const bool aoe, const float throttleSeconds)
        {
            if (!Game::IsFocused() || Game::IsChatOpen())
                return SpellCastState::NotCasted;

            const Object target = GetTarget(extraRange);
            return target ? Cast(target, aoe, throttleSeconds) : SpellCastState::NotCasted;
        }

        [[nodiscard]] bool CastOnUnit(const Object& unit)
        {
            return CastOnUnit(unit, castThrottle_);
        }

        [[nodiscard]] bool CastOnUnit(const Object& unit, const float throttleSeconds)
        {
            if (!unit || !Game::IsFocused() || Game::IsChatOpen())
                return false;

            if (!IsReady() || RangeCheckFrom().DistanceToSqr(unit.Position()) > GetRangeSqr(unit))
                return false;

            const Object player = ObjectManager::LocalPlayer();
            if (!player)
                return false;

            if (isChannelProtected_ && !CanIssueChannelCast())
                return false;
            if (!BeginCastRequest(throttleSeconds))
                return false;

            const bool result = player.Cast(slot_, unit);
            if (isChannelProtected_ && result)
                channelCastRequestTime_ = Game::TimeGame();
            return result;
        }

        [[nodiscard]] bool Cast()
        {
            return Cast(castThrottle_);
        }

        [[nodiscard]] bool Cast(const float throttleSeconds)
        {
            if (isChannelProtected_)
                return CastChannelSpell(throttleSeconds);

            // LeagueSharp Spell.Cast() is the self-targeted convenience overload for normal spells.
            const Object player = ObjectManager::LocalPlayer();
            return player && CastOnUnit(player, throttleSeconds);
        }

        [[nodiscard]] bool Cast(const Vec3& position)
        {
            return Cast(position, castThrottle_);
        }

        [[nodiscard]] bool Cast(const Vec3& position, const float throttleSeconds)
        {
            if (!IsReady() || !Game::IsFocused() || Game::IsChatOpen())
                return false;

            // Charged start/release has its own lifecycle and existing guard.
            // Never delay a release because of the ordinary cast throttle.
            if (isChargedSpell_)
            {
                lastCastAttemptTime_ = Game::TimeGame();
                return IsCharging() ? ReleaseCharge(position) : StartCharging();
            }
            if (isChannelProtected_)
                return channelUsePositionCast_ ? CastChannelSpell(position, throttleSeconds) : CastChannelSpell(throttleSeconds);

            const Object player = ObjectManager::LocalPlayer();
            if (!player || !BeginCastRequest(throttleSeconds))
                return false;
            return player.Cast(slot_, position);
        }

        [[nodiscard]] bool Cast(const Vec3& fromPosition, const Vec3& toPosition)
        {
            return Cast(fromPosition, toPosition, castThrottle_);
        }

        [[nodiscard]] bool Cast(const Vec3& fromPosition, const Vec3& toPosition, const float throttleSeconds)
        {
            if (!IsReady() || !Game::IsFocused() || Game::IsChatOpen())
                return false;

            const Object player = ObjectManager::LocalPlayer();
            if (!player || !BeginCastRequest(throttleSeconds))
                return false;
            return player.Cast(slot_, fromPosition, toPosition);
        }

        [[nodiscard]] bool WillHit(const Object& unit, const Vec3& castPosition, const float extraWidth = 0.0f,
            const HitChance minHitChance = HitChance::High) const
        {
            if (!unit)
                return false;

            const PredictionOutput prediction = GetPrediction(unit);
            return static_cast<std::uint8_t>(prediction.Hitchance) >= static_cast<std::uint8_t>(minHitChance) &&
                WillHit(prediction.UnitPosition, castPosition, extraWidth);
        }

        [[nodiscard]] bool WillHit(const Vec3& point, const Vec3& castPosition, const float extraWidth = 0.0f) const noexcept
        {
            switch (type_)
            {
            case SkillshotType::Circle:
                // LeagueSharp Spell.WillHit uses WidthSqr for circles; extraWidth is only applied to line checks.
                return point.To2D().DistanceToSqr(castPosition.To2D()) < width_ * width_;
            case SkillshotType::Line:
                return geometry::DistanceToSegment(point.To2D(), From().To2D(), castPosition.To2D()) < width_ + extraWidth;
            case SkillshotType::Cone:
            {
                // Prediction/LeagueSharp cone width is an angle in radians. Avoid Vec2::AngleBetween here
                // because that helper intentionally returns degrees for Orbwalker/LeagueSharp Geometry parity.
                const Vec2 from = From().To2D();
                const Vec2 direction = castPosition.To2D() - from;
                const Vec2 v = point.To2D() - from;
                if (!direction.IsValid() || !v.IsValid() || v.LengthSqr() >= RangeSqr())
                    return false;

                const Vec2 edge1 = direction.Rotated(-width_ * 0.5f);
                const Vec2 edge2 = edge1.Rotated(width_);
                return geometry::Cross(edge1, v) > 0.0f && geometry::Cross(v, edge2) > 0.0f;
            }
            default:
                return false;
            }
        }

        [[nodiscard]] int CountHits(const std::vector<Object>& units, const Vec3& castPosition) const
        {
            // LeagueSharp first predicts every unit position, then feeds those points to CountHits.
            std::vector<Vec3> points{};
            points.reserve(units.size());
            for (const Object& unit : units)
            {
                if (unit)
                    points.push_back(GetPrediction(unit).UnitPosition);
            }
            return CountHits(points, castPosition);
        }

        [[nodiscard]] int CountHits(const std::vector<Object>& units, const Vec3& castPosition,
            const float extraWidth, const HitChance minHitChance) const
        {
            int count = 0;
            for (const Object& unit : units)
            {
                if (WillHit(unit, castPosition, extraWidth, minHitChance))
                    ++count;
            }
            return count;
        }

        [[nodiscard]] int CountHits(const std::vector<Vec3>& points, const Vec3& castPosition,
            const float extraWidth = 0.0f) const noexcept
        {
            int count = 0;
            for (const Vec3& point : points)
            {
                if (WillHit(point, castPosition, extraWidth))
                    ++count;
            }
            return count;
        }

        [[nodiscard]] float GetHealthPrediction(const Object& unit) const noexcept
        {
            if (!unit || !HealthPrediction::IsAvailable())
                return 0.0f;

            float travelTime = 0.0f;
            if (speed_ > 1.0e-4f && std::isfinite(speed_))
                travelTime = From().DistanceTo(unit.ServerPos().IsValid() ? unit.ServerPos() : unit.Position()) / speed_;

            const float time = std::max(0.0f, delay_ + travelTime - 0.100f);
            return HealthPrediction::GetHealthPrediction(unit, time);
        }

        [[nodiscard]] int GetHitCount(const HitChance hitChance = HitChance::High) const
        {
            int count = 0;
            for (const Object& enemy : HeroManager::Enemies())
            {
                if (!enemy)
                    continue;
                const PredictionOutput prediction = GetPrediction(enemy);
                if (static_cast<std::uint8_t>(prediction.Hitchance) >= static_cast<std::uint8_t>(hitChance))
                    ++count;
            }
            return count;
        }

        [[nodiscard]] MinionManager::FarmLocation GetCircularFarmLocation(
            const std::vector<Object>& minions,
            const float overrideWidth = -1.0f) const
        {
            const float radius = overrideWidth >= 0.0f ? overrideWidth : width_;
            const std::vector<Vec3> positions = MinionManager::GetMinionsPredictedPositions(
                minions, delay_, width_, speed_, From(), Range(), false, SkillshotType::Circle, RangeCheckFrom());

            return MinionManager::GetBestCircularFarmLocation(positions, radius, Range(), From());
        }

        [[nodiscard]] MinionManager::FarmLocation GetCircularFarmLocation(
            const std::vector<Vec3>& positions,
            const float overrideWidth = -1.0f) const
        {
            const float radius = overrideWidth >= 0.0f ? overrideWidth : width_;
            return MinionManager::GetBestCircularFarmLocation(positions, radius, Range(), From());
        }

        [[nodiscard]] MinionManager::FarmLocation GetLineFarmLocation(
            const std::vector<Object>& minions,
            const float overrideWidth = -1.0f) const
        {
            const float radius = overrideWidth >= 0.0f ? overrideWidth : width_;
            const std::vector<Vec3> positions = MinionManager::GetMinionsPredictedPositions(
                minions, delay_, width_, speed_, From(), Range(), false, SkillshotType::Line, RangeCheckFrom());
            return MinionManager::GetBestLineFarmLocation(positions, radius, Range(), From());
        }

        [[nodiscard]] MinionManager::FarmLocation GetLineFarmLocation(
            const std::vector<Vec3>& positions,
            const float overrideWidth = -1.0f) const
        {
            const float radius = overrideWidth >= 0.0f ? overrideWidth : width_;
            return MinionManager::GetBestLineFarmLocation(positions, radius, Range(), From());
        }

    private:
        // A throttle override describes the minimum elapsed time since the previous
        // request. Zero deliberately bypasses the Spell throttle for valid recasts.
        [[nodiscard]] bool IsCastThrottled(const float throttleSeconds) const noexcept
        {
            const float throttle = std::isfinite(throttleSeconds) ? std::max(0.0f, throttleSeconds) : castThrottle_;
            return throttle > 0.0f && Game::TimeGame() - lastCastRequestTime_ < throttle;
        }

        // Reserve BEFORE entering the native cast, so reentrant callbacks cannot spam.
        // A false native return is not a server ACK; do not roll back this timestamp.
        [[nodiscard]] bool BeginCastRequest(const float throttleSeconds) noexcept
        {
            if (IsCastThrottled(throttleSeconds))
                return false;
            const float now = Game::TimeGame();
            lastCastRequestTime_ = now;
            lastCastAttemptTime_ = now;
            return true;
        }

        [[nodiscard]] SpellCastState CastInternal(
            const Object& unit,
            const HitChance hitChance,
            const bool exactHitChance,
            bool aoe,
            const int minTargets,
            const float throttleSeconds)
        {
            if (!unit)
                return SpellCastState::InvalidTarget;

            if (!Game::IsFocused() || Game::IsChatOpen())
                return SpellCastState::NotCasted;

            if (!IsReady())
                return SpellCastState::NotReady;

            const Object player = ObjectManager::LocalPlayer();
            if (!player)
                return SpellCastState::NotCasted;

            // Avoid expensive prediction work when the same Spell has just issued a cast.
            // Chargeable spells retain their separate start/release timing behavior.
            if (!isChargedSpell_ && IsCastThrottled(throttleSeconds))
                return SpellCastState::Throttled;

            // Targeted spell.
            if (!isSkillshot_)
            {
                //Target out of range
                if (!IsInRange(unit))
                    return SpellCastState::OutOfRange;

                if (isChannelProtected_ && !CanIssueChannelCast())
                    return SpellCastState::NotCasted;
                if (!BeginCastRequest(throttleSeconds))
                    return SpellCastState::Throttled;

                const bool casted = player.Cast(slot_, unit);
                if (isChannelProtected_ && casted)
                    channelCastRequestTime_ = Game::TimeGame();
                return casted ? SpellCastState::SuccessfullyCasted : SpellCastState::NotCasted;
            }

            if (!Prediction::IsAvailable())
                return SpellCastState::PredictionUnavailable;

            if (minTargets >= 0)
                aoe = true;

            //Get the best position to cast the spell.
            const PredictionOutput prediction = GetPrediction(unit, aoe);
            if (minTargets >= 0 && static_cast<int>(prediction.AoeTargetsHitCount) < minTargets)
                return SpellCastState::NotEnoughTargets;

            //Skillshot collides.
            if (!prediction.CollisionObjects.empty())
                return SpellCastState::Collision;

            //Target out of range.
            if (!IsInRange(prediction.CastPosition))
                return SpellCastState::OutOfRange;

            //The hitchance is too low.
            const std::uint8_t predicted = static_cast<std::uint8_t>(prediction.Hitchance);
            const std::uint8_t required = static_cast<std::uint8_t>(hitChance);
            if (predicted < required || (exactHitChance && predicted != required))
                return SpellCastState::LowHitChance;

            if (isChargedSpell_)
            {
                lastCastAttemptTime_ = Game::TimeGame();
                const bool casted = IsCharging() ? ReleaseCharge(prediction.CastPosition) : StartCharging();
                return casted ? SpellCastState::SuccessfullyCasted : SpellCastState::NotCasted;
            }
            if (isChannelProtected_)
            {
                if (!CanIssueChannelCast())
                    return SpellCastState::NotCasted;
                if (IsCastThrottled(throttleSeconds))
                    return SpellCastState::Throttled;
                const bool casted = channelUsePositionCast_ ? CastChannelSpell(prediction.CastPosition, throttleSeconds) : CastChannelSpell(throttleSeconds);
                return casted ? SpellCastState::SuccessfullyCasted : SpellCastState::NotCasted;
            }

            if (!BeginCastRequest(throttleSeconds))
                return SpellCastState::Throttled;
            return player.Cast(slot_, prediction.CastPosition)
                ? SpellCastState::SuccessfullyCasted
                : SpellCastState::NotCasted;
        }

        void OnProcessSpell(const ProcessSpellEvent& event)
        {
            if (!event.sender.IsMe() || !event.spellCast)
                return;

            if (event.spellCast.SpellName() == chargedSpellName_)
                chargedCastedTime_ = Game::TimeGame();
        }

        void OnUpdateChargeableSpell(UpdateChargeableSpellEvent& event)
        {
            if (!event.sender.IsMe() || event.slot != slot_)
                return;

            if (Game::TimeGame() - chargedRequestTime_ < 3.0f && event.flag)
                event.process = false;
        }

        void OnChargedCastSpell(CastSpellEvent& event)
        {
            if (!event.sender.IsMe() || event.slot != slot_)
                return;

            if (Game::TimeGame() - chargedRequestTime_ > 0.500f && IsCharging())
            {
                const Vec3 position = event.destinationPosition.IsValid() ? event.destinationPosition : event.castEndPosition;
                if (position.IsValid())
                    (void)Cast(position);
            }
        }

        [[nodiscard]] bool CanIssueChannelCast() const noexcept
        {
            if (!isChannelProtected_ || IsChanneling())
                return false;
            const float throttle = 0.400f + static_cast<float>(Game::Latency()) / 1000.0f;
            return Game::TimeGame() - channelCastRequestTime_ > throttle;
        }

        bool CastChannelSpell(const float throttleSeconds) noexcept
        {
            if (!Game::IsFocused() || Game::IsChatOpen())
                return false;
            const Object player = ObjectManager::LocalPlayer();
            if (!player || !CanIssueChannelCast() || !BeginCastRequest(throttleSeconds))
                return false;

            const bool result = player.Cast(slot_);
            if (result)
                channelCastRequestTime_ = Game::TimeGame();
            return result;
        }

        bool CastChannelSpell(const Vec3& position, const float throttleSeconds) noexcept
        {
            if (!Game::IsFocused() || Game::IsChatOpen())
                return false;
            const Object player = ObjectManager::LocalPlayer();
            if (!player || !CanIssueChannelCast() || !BeginCastRequest(throttleSeconds))
                return false;

            const bool result = player.Cast(slot_, position);
            if (result)
                channelCastRequestTime_ = Game::TimeGame();
            return result;
        }

        struct ChannelSpellInfo final
        {
            std::string_view spellName;
            std::string_view endObjectName;
            bool endOnCreate;
        };

        inline static constexpr std::array<ChannelSpellInfo, 6> kChannelSpells
        {{
            { "DrainChannel", "Fiddlesticks_Base_Drain.troy", false },
            { "KatarinaR", "katarina_deathLotus_tar.troy", false },
            { "Crowstorm", "Fiddlesticks_Base_Base_Crowstorm_green_cas.troy", true },
            { "GalioIdolOfDurand", "Galio_Base_R_explo.troy", false },
            { "AlZaharNetherGrasp", "Malzahar_Base_R_Beam.troy", false },
            { "ReapTheWhirlwind", "ReapTheWhirlwind_green_cas.troy", false }
        }};

        [[nodiscard]] static const ChannelSpellInfo* FindChannelSpellInfo(const std::string_view spellName) noexcept
        {
            const auto it = std::find_if(kChannelSpells.begin(), kChannelSpells.end(), [spellName](const ChannelSpellInfo& info)
            {
                return info.spellName == spellName;
            });
            return it != kChannelSpells.end() ? &*it : nullptr;
        }

        void ClearChannelState() noexcept
        {
            isChanneling_ = false;
            channelManualCancelRequested_ = false;
            activeChannelSpellName_ = {};
            activeChannelEndObjectName_ = {};
            activeChannelEndsOnCreate_ = false;
        }

        void OnChannelDoCast(const DoCastSpellEvent& event) noexcept
        {
            if (!isChannelProtected_ || !event.sender.IsMe() || !event.spellCast ||
                event.spellCast.Slot() != static_cast<std::uint8_t>(slot_))
                return;

            const std::string spellName = event.spellCast.SpellName();
            const ChannelSpellInfo* const info = FindChannelSpellInfo(spellName);
            if (info == nullptr)
                return;

            channelManualCancelRequested_ = false;
            isChanneling_ = true;
            activeChannelSpellName_ = info->spellName;
            activeChannelEndObjectName_ = info->endObjectName;
            activeChannelEndsOnCreate_ = info->endOnCreate;
        }

        void OnChannelStopCast(const StopCastEvent& event) noexcept
        {
            if (!isChannelProtected_ || !event.sender.IsMe() || !isChanneling_)
                return;
            if (!event.spellCast || event.spellCast.Slot() == static_cast<std::uint8_t>(slot_))
                ClearChannelState();
        }

        void OnChannelCreateObject(const std::uintptr_t address) noexcept
        {
            if (!isChannelProtected_ || !isChanneling_ || !activeChannelEndsOnCreate_ || address == 0u)
                return;

            const Object object(address);
            if (object.ObjectName() == activeChannelEndObjectName_)
                ClearChannelState();
        }

        void OnChannelDeleteObject(const std::uintptr_t address) noexcept
        {
            if (!isChannelProtected_ || !isChanneling_ || activeChannelEndsOnCreate_ || address == 0u)
                return;

            const Object object(address);
            if (object.ObjectName() == activeChannelEndObjectName_)
                ClearChannelState();
        }

        void OnChannelIssueOrder(IssueOrderEvent& event) const noexcept
        {
            if (!event.sender.IsMe() || !IsChanneling())
                return;

            switch (event.order)
            {
            case GameObjectOrder::MoveTo:
            case GameObjectOrder::AttackTo:
            case GameObjectOrder::AttackUnit:
            case GameObjectOrder::AutoAttack:
                event.process = false;
                break;
            default:
                break;
            }
        }

        void OnChannelCastSpell(CastSpellEvent& event) const noexcept
        {
            if (!isChannelProtected_ || allowOtherSpellsToCancelChannel_ || !event.sender.IsMe() || !IsChanneling())
                return;
            event.process = false;
        }

        void OnChannelWndProc(const RSDK_WndProcEvent& event) noexcept
        {
            constexpr std::uint32_t kWmRButtonUp = 0x0205u;     // WM_RBUTTONUP
            if (allowUserChannelCancel_ && event.message == kWmRButtonUp && IsChanneling())
            {
                channelManualCancelRequested_ = true;
                isChanneling_ = false;
                activeChannelSpellName_ = {};
                activeChannelEndObjectName_ = {};
                activeChannelEndsOnCreate_ = false;
            }
        }

        SpellSlot slot_{SpellSlot::_SlotUnknown};
        DamageType damageType_{DamageType::Physical};
        HitChance minHitChance_{HitChance::VeryHigh};
        SkillshotType type_{SkillshotType::Line};
        CollisionableObjects collisionObjects_{CollisionableObjects::Minions | CollisionableObjects::YasuoWall};

        float delay_{};
        float width_{};
        float speed_{std::numeric_limits<float>::max()};
        float range_{std::numeric_limits<float>::max()};
        Vec3 from_{};
        Vec3 rangeCheckFrom_{};
        bool collision_{};
        bool isSkillshot_{};
        bool addSelfHitboxToRange_{};
        bool addEnemyHitboxToRange_{};

        bool isChargedSpell_{};
        ChargedReleaseMode chargedReleaseMode_{ChargedReleaseMode::UpdateOnly};
        std::string chargedSpellName_{};
        std::string chargedBuffName_{};
        float chargedMinRange_{};
        float chargedMaxRange_{};
        float chargeDuration_{};
        float chargedCastedTime_{-1000.0f};
        float chargedRequestTime_{-1000.0f};

        bool isChannelProtected_{};
        bool isChanneling_{};
        bool allowUserChannelCancel_{};
        bool channelUsePositionCast_{};
        bool channelManualCancelRequested_{};
        bool allowOtherSpellsToCancelChannel_{};
        std::string_view activeChannelSpellName_{};
        std::string_view activeChannelEndObjectName_{};
        bool activeChannelEndsOnCreate_{};
        float channelCastRequestTime_{-1000.0f};
        float lastCastAttemptTime_{};
        float castThrottle_{0.26f};
        float lastCastRequestTime_{-1000.0f};

        EventConnection processSpellConnection_{};
        EventConnection updateChargeConnection_{};
        EventConnection chargedCastSpellConnection_{};
        EventConnection channelDoCastConnection_{};
        EventConnection channelStopCastConnection_{};
        EventConnection channelCreateObjectConnection_{};
        EventConnection channelDeleteObjectConnection_{};
        EventConnection channelIssueOrderConnection_{};
        EventConnection channelCastSpellConnection_{};
        EventConnection channelWndProcConnection_{};
    };

    inline Object TargetSelector::GetTargetNoCollision(
        const Spell& spell,
        const bool ignoreShield,
        const std::vector<Object>* const ignoredChamps,
        const Vec3* const rangeCheckFrom)
    {
        const Object target = ignoredChamps != nullptr
            ? TargetSelector::GetTarget(spell.Range(), spell.GetDamageType(), ignoreShield, *ignoredChamps, rangeCheckFrom)
            : TargetSelector::GetTarget(spell.Range(), spell.GetDamageType(), ignoreShield, rangeCheckFrom);
        if (!target || !spell.Collision())
            return {};
        return spell.GetPrediction(target).Hitchance != HitChance::Collision ? target : Object{};
    }
}
