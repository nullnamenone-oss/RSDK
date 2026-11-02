#define Plugin_Id              "Guide.TargetSelectorOrbwalker"
#define Plugin_Name            "Getting Started - TargetSelector and Orbwalker"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "TargetSelector and Orbwalker API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/TargetSelector.h>

using namespace rsdk;

namespace guide
{
    std::uint32_t g_lastTargetNetworkId{};
    float g_nextStatusLog{};

    bool LowHealthTarget(const Object& target)
    {
        return target.HealthPercent() < 60.0f;
    }

    void OnBeforeAttack(BeforeAttackEvent& event)
    {
        if (!event.Target)
        {
            event.Process = false;
            Log::Info("[Orbwalker] BeforeAttack cancelled because target is invalid.");
            return;
        }

        Log::Info("[Orbwalker] BeforeAttack target=%s netId=%u",
            event.Target.ObjectName().c_str(), event.Target.NetworkId());
    }

    void OnGameUpdate()
    {
        if (!TargetSelector::IsAvailable() || !Orbwalker::IsAvailable())
            return;

        if (Orbwalker::ActiveMode() != Orbwalker::Mode::Combo)
        {
            if (g_lastTargetNetworkId != 0u)
            {
                Orbwalker::ClearForcedTarget();
                g_lastTargetNetworkId = 0u;
            }
            return;
        }

        const Object target = TargetSelector::GetTarget(1200.0f, DamageType::Magical, true, nullptr, LowHealthTarget);
        if (target)
        {
            Orbwalker::ForceTarget(target);
            if (target.NetworkId() != g_lastTargetNetworkId)
            {
                g_lastTargetNetworkId = target.NetworkId();
                Log::Info("[TargetSelector] forced target=%s netId=%u HP=%.1f%%",
                    target.ChampionName().c_str(), target.NetworkId(), target.HealthPercent());
            }
        }
        else
        {
            Orbwalker::ClearForcedTarget();
            g_lastTargetNetworkId = 0u;
        }

        const float now = Game::TimeGame();
        if (now >= g_nextStatusLog)
        {
            g_nextStatusLog = now + 2.0f;
            Log::Info("[Orbwalker] mode=%d canAttack=%d canMove=%d shouldWait=%d",
                static_cast<int>(Orbwalker::ActiveMode()), Orbwalker::CanAttack() ? 1 : 0,
                Orbwalker::CanMove() ? 1 : 0, Orbwalker::ShouldWait() ? 1 : 0);
        }

        if (!Orbwalker::ShouldWait())
            (void)Orbwalker::MoveTo(Game::CursorPosition());
    }
}

class TargetSelectorOrbwalkerExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        Orbwalker::BeforeAttack += guide::OnBeforeAttack;
        Events::OnGameUpdate += guide::OnGameUpdate;
        Log::Info("[TargetSelector] Example loaded. Filter accepts targets below 60%% HP.");
        return true;
    }
};

RSDK_REGISTER_PLUGIN(TargetSelectorOrbwalkerExample);
