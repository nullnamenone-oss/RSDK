#define Plugin_Id              "Guide.FirstPlugin"
#define Plugin_Name            "Getting Started - First Plugin"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Minimal RSDK 5.2.0 plugin example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

namespace guide
{
    bool g_printedPlayer{};

    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.IsDead())
            return;

        // Keep the first example quiet after it has proved that the event works.
        if (g_printedPlayer)
            return;

        g_printedPlayer = true;
        const Vec3 pos = player.Position();
        Log::Info("[FirstPlugin] Player=%s netId=%u level=%d", player.ChampionName().c_str(), player.NetworkId(), player.Level());
        Log::Info("[FirstPlugin] HP=%.0f/%.0f Mana=%.0f/%.0f", player.Health(), player.MaxHealth(), player.Mana(), player.MaxMana());
        Log::Info("[FirstPlugin] Position=(%.1f, %.1f, %.1f) gameTime=%.2f latency=%dms", pos.x, pos.y, pos.z, Game::TimeGame(), Game::Latency());
    }
}

class FirstPlugin final : public Plugin
{
public:
    bool OnLoad() override
    {
        Log::Info("[FirstPlugin] Loading %s v%s", Plugin_Name, Plugin_Version);

        // Preferred style for a lifetime handler: give the handler a name.
        Events::OnGameUpdate += guide::OnGameUpdate;

        // An inline lambda is also valid for small one-off handlers:
        // Events::OnGameUpdate += []() { /* ... */ };

        Log::Info("[FirstPlugin] OnGameUpdate registered.");
        return true;
    }

    void OnUnload() override
    {
        Log::Info("[FirstPlugin] Unloading.");
    }
};

RSDK_REGISTER_PLUGIN(FirstPlugin);
