#define Plugin_Id              "Guide.Events"
#define Plugin_Name            "Getting Started - Events"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "RSDK event subscription example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

namespace guide
{
    float g_nextHeartbeat{};

    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.IsDead())
            return;

        const float now = Game::TimeGame();
        if (now < g_nextHeartbeat)
            return;
        g_nextHeartbeat = now + 5.0f;

        Log::Info("[OnGameUpdate] time=%.2f player=%s HP=%.1f%%", now, player.ChampionName().c_str(), player.HealthPercent());
    }

    void OnProcessSpell(const ProcessSpellEvent& event)
    {
        if (!event.sender || !event.spellCast || !event.sender.IsMe())
            return;

        const Vec3 start = event.spellCast.StartPos();
        const Vec3 end = event.spellCast.EndPos();
        Log::Info("[OnProcessSpell] name=%s slot=%u autoAttack=%d missileNet=%u",
            event.spellCast.SpellName().c_str(), event.spellCast.Slot(), event.spellCast.IsAutoAttack() ? 1 : 0,
            event.spellCast.MissileNetworkId());
        Log::Info("[OnProcessSpell] start=(%.1f, %.1f, %.1f) end=(%.1f, %.1f, %.1f)",
            start.x, start.y, start.z, end.x, end.y, end.z);
    }

    void OnNewPath(const NewPathEvent& event)
    {
        if (!event.sender || !event.sender.IsMe())
            return;

        Log::Info("[OnNewPath] points=%zu dash=%d dashSpeed=%.0f gravity=%u",
            event.path.size(), event.isDash ? 1 : 0, event.dashSpeed, event.dashGravityCount);

        if (!event.path.empty())
        {
            const Vec3& destination = event.path.back();
            Log::Info("[OnNewPath] destination=(%.1f, %.1f, %.1f)", destination.x, destination.y, destination.z);
        }
    }

    void OnIssueOrder(IssueOrderEvent& event)
    {
        if (!event.sender || !event.sender.IsMe())
            return;

        Log::Info("[OnIssueOrder] order=%d attackMove=%d process=%d pos=(%.1f, %.1f, %.1f)",
            static_cast<int>(event.order), event.isAttackMove ? 1 : 0, event.process ? 1 : 0,
            event.position.x, event.position.y, event.position.z);

        // Cancellable events expose event.process. Only set it to false when your
        // feature intentionally owns and blocks that action.
        // event.process = false;
    }

    void OnStopCast(const StopCastEvent& event)
    {
        if (!event.sender || !event.sender.IsMe())
            return;

        Log::Info("[OnStopCast] spell=%s stopAnimation=%d destroyMissile=%d missileNet=%u",
            event.spellCast ? event.spellCast.SpellName().c_str() : "<none>",
            event.stopAnimation ? 1 : 0, event.destroyMissile ? 1 : 0, event.missileNetworkId);
    }

    void RegisterEvents()
    {
        // Recommended style for normal lifetime handlers: named functions are easy
        // to find, debug, reuse, and compare with the event signature.
        Events::OnGameUpdate += OnGameUpdate;
        Events::OnProcessSpell += OnProcessSpell;
        Events::OnNewPath += OnNewPath;
        Events::OnIssueOrder += OnIssueOrder;
        Events::OnStopCast += OnStopCast;

        // Inline lambdas are also supported and can be convenient for tiny handlers:
        // Events::OnNewPath += [](const NewPathEvent& event) { /* ... */ };
    }
}

class EventsExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        guide::RegisterEvents();
        Log::Info("[Events] Registered OnGameUpdate, OnProcessSpell, OnNewPath, OnIssueOrder and OnStopCast.");
        return true;
    }
};

RSDK_REGISTER_PLUGIN(EventsExample);
