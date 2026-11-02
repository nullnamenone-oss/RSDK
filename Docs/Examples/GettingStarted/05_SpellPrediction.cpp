#define Plugin_Id              "Guide.SpellPrediction"
#define Plugin_Name            "Getting Started - Spell and Prediction"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Spell and Prediction API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/Spell.h>
#include <Common/TargetSelector.h>

using namespace rsdk;

class SpellPredictionExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        q_.SetSkillshot(0.25f, 70.0f, 1700.0f, true, SkillshotType::Line)
          .SetCollisionObjects(CollisionableObjects::Minions | CollisionableObjects::YasuoWall)
          .SetMinHitChance(HitChance::High);

        Events::OnGameUpdate.Subscribe(this, &SpellPredictionExample::OnGameUpdate);

        Log::Info("[Spell] Q configured: range=%.0f delay=%.2fs width=%.0f speed=%.0f",
            q_.Range(), q_.Delay(), q_.Width(), q_.Speed());
        Log::Info("[Spell] Prediction available=%d TargetSelector available=%d",
            Prediction::IsAvailable() ? 1 : 0, TargetSelector::IsAvailable() ? 1 : 0);
        return true;
    }

private:
    void OnGameUpdate()
    {
        if (Orbwalker::ActiveMode() != Orbwalker::Mode::Combo || !q_.IsReady())
            return;

        const Object target = q_.GetTarget();
        if (!target)
            return;

        const PredictionOutput prediction = q_.GetPrediction(target);
        const float now = Game::TimeGame();
        if (now >= nextPredictionLog_)
        {
            nextPredictionLog_ = now + 1.0f;
            Log::Info("[Spell] target=%s HP=%.1f%% hitchance=%d collisionCount=%zu",
                target.ChampionName().c_str(), target.HealthPercent(), static_cast<int>(prediction.Hitchance),
                prediction.CollisionObjects.size());
            Log::Info("[Spell] unitPos=(%.1f, %.1f, %.1f) castPos=(%.1f, %.1f, %.1f)",
                prediction.UnitPosition.x, prediction.UnitPosition.y, prediction.UnitPosition.z,
                prediction.CastPosition.x, prediction.CastPosition.y, prediction.CastPosition.z);
        }

        if (prediction.Hitchance < HitChance::High || !prediction.CollisionObjects.empty())
            return;

        const bool casted = q_.Cast(prediction.CastPosition);
        Log::Info("[Spell] Cast attempted on %s, result=%d", target.ChampionName().c_str(), casted ? 1 : 0);
    }

    Spell q_{SpellSlot::_Q, 1200.0f, DamageType::Magical};
    float nextPredictionLog_{};
};

RSDK_REGISTER_PLUGIN(SpellPredictionExample);
