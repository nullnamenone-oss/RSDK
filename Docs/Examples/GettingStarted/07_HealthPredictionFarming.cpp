#define Plugin_Id              "Guide.HealthPredictionFarming"
#define Plugin_Name            "Getting Started - HealthPrediction and Farming"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "HealthPrediction and farm helper example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/Spell.h>
#include <Common/MinionManager.h>
#include <Common/Utility.h>

#include <limits>
#include <vector>

using namespace rsdk;

class HealthPredictionFarmingExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        w_.SetSkillshot(0.50f, 250.0f, std::numeric_limits<float>::max(), false, SkillshotType::Circle);
        Events::OnGameUpdate.Subscribe(this, &HealthPredictionFarmingExample::OnGameUpdate);

        Log::Info("[Farming] W configured: range=%.0f delay=%.2fs radius=%.0f", w_.Range(), w_.Delay(), w_.Width());
        Log::Info("[Farming] HealthPrediction available=%d", HealthPrediction::IsAvailable() ? 1 : 0);
        return true;
    }

private:
    void OnGameUpdate()
    {
        if (Orbwalker::ActiveMode() != Orbwalker::Mode::LaneClear || !w_.IsReady())
            return;

        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return;

        std::vector<Object> minions;
        for (const Object& minion : ObjectManager::AllMinions(TeamFilter::Enemy))
        {
            if (Utility::IsValidTarget(minion, w_.Range()))
                minions.push_back(minion);
        }

        const MinionManager::FarmLocation farm = w_.GetCircularFarmLocation(minions);
        const float now = Game::TimeGame();
        if (now >= nextFarmLog_)
        {
            nextFarmLog_ = now + 1.5f;
            Log::Info("[Farming] candidates=%zu bestHitCount=%d farmPos=(%.1f, %.1f, %.1f)",
                minions.size(), farm.MinionsHit, farm.Position.x, farm.Position.y, farm.Position.z);
        }

        if (!minions.empty() && HealthPrediction::IsAvailable())
        {
            const Object sample = minions.front();
            const float normal = HealthPrediction::GetHealthPrediction(sample, 0.50f);
            const float lane = HealthPrediction::LaneClearHealthPrediction(sample, 0.50f);

            if (now >= nextHealthLog_)
            {
                nextHealthLog_ = now + 2.0f;
                Log::Info("[HealthPrediction] minion=%s current=%.1f normal(0.5s)=%.1f laneClear(0.5s)=%.1f",
                    sample.ObjectName().c_str(), sample.Health(), normal, lane);
            }
        }

        if (farm.MinionsHit >= 3)
        {
            const bool casted = w_.Cast(farm.Position);
            Log::Info("[Farming] W cast for %d minions, result=%d", farm.MinionsHit, casted ? 1 : 0);
        }
    }

    Spell w_{SpellSlot::_W, 900.0f, DamageType::Magical};
    float nextFarmLog_{};
    float nextHealthLog_{};
};

RSDK_REGISTER_PLUGIN(HealthPredictionFarmingExample);
