#define Plugin_Id              "Guide.ObjectsAndGame"
#define Plugin_Name            "Getting Started - Objects and Game"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "ObjectManager and Game API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/HeroManager.h>
#include <Common/Utility.h>

using namespace rsdk;

namespace guide
{
    float g_nextLogTime{};

    Vec3 ServerOrClientPosition(const Object& object)
    {
        if (!object)
            return {};
        const Vec3 server = object.ServerPos();
        return server.IsValid() ? server : object.Position();
    }

    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.IsDead())
            return;

        const float now = Game::TimeGame();
        if (now < g_nextLogTime)
            return;
        g_nextLogTime = now + 2.0f;

        const Vec3 position = player.Position();
        const Vec3 serverPosition = ServerOrClientPosition(player);
        const auto enemies = ObjectManager::AllHeroes(TeamFilter::Enemy);

        Log::Info("[Objects] time=%.2f player=%s enemies=%zu", now, player.ChampionName().c_str(), enemies.size());
        Log::Info("[Objects] clientPos=(%.1f, %.1f, %.1f) serverPos=(%.1f, %.1f, %.1f)",
            position.x, position.y, position.z, serverPosition.x, serverPosition.y, serverPosition.z);
        Log::Info("[Objects] HP=%.1f%% Mana=%.1f%% moveSpeed=%.0f attackRange=%.0f",
            player.HealthPercent(), player.ManaPercent(), player.MovementSpeed(), player.AttackRange());

        std::size_t validTargets{};
        for (const Object& enemy : enemies)
        {
            if (!Utility::IsValidTarget(enemy, 1200.0f))
                continue;

            ++validTargets;
            const Vec3 enemyPosition = ServerOrClientPosition(enemy);
            Log::Info("[Objects] target=%s netId=%u HP=%.1f%% distance=%.0f visible=%d",
                enemy.ChampionName().c_str(), enemy.NetworkId(), enemy.HealthPercent(),
                serverPosition.Distance2D(enemyPosition), enemy.IsVisible() ? 1 : 0);
        }

        Log::Info("[Objects] valid targets within 1200 = %zu | HeroManager enemies = %zu",
            validTargets, HeroManager::Enemies().size());
    }
}

class ObjectsAndGameExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        Events::OnGameUpdate += guide::OnGameUpdate;
        Log::Info("[Objects] Example loaded; status is printed every 2 seconds.");
        return true;
    }
};

RSDK_REGISTER_PLUGIN(ObjectsAndGameExample);
