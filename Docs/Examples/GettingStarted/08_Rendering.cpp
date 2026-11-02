#define Plugin_Id              "Guide.Rendering"
#define Plugin_Name            "Getting Started - Rendering"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Render API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

class RenderingExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        Events::OnDraw.Subscribe(this, &RenderingExample::OnDraw);
        Events::OnGameUpdate.Subscribe(this, &RenderingExample::OnGameUpdate);
        Log::Info("[Rendering] OnDraw and OnGameUpdate registered.");
        return true;
    }

private:
    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return;

        const float now = Game::TimeGame();
        if (now < nextLogTime_)
            return;
        nextLogTime_ = now + 3.0f;

        const Vec3 world = player.Position();
        const Vec2 screen = Game::WorldToScreen(world);
        const Vec2 minimap = Game::WorldToMinimap(world);

        Log::Info("[Rendering] world=(%.1f, %.1f, %.1f)", world.x, world.y, world.z);
        Log::Info("[Rendering] screen=(%.1f, %.1f) valid=%d minimap=(%.1f, %.1f)",
            screen.x, screen.y, screen.IsValid() ? 1 : 0, minimap.x, minimap.y);
    }

    void OnDraw()
    {
        if (!Render::IsReady())
            return;

        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return;

        const Vec3 world = player.Position();
        const Vec2 screen = Game::WorldToScreen(world);

        Render::Circle3D(world, 650.0f, Color(80, 190, 255, 220), 2.0f, 96u, true);

        if (!screen.IsValid())
            return;

        const Vec2 textPos = screen + Vec2(20.0f, -60.0f);
        Render::Text(textPos, Localization::Text("guide.rendering.label", "Rendering example"), Color(255, 255, 255), 18.0f);
        Render::Line2D(screen + Vec2(-50.0f, -35.0f), screen + Vec2(50.0f, -35.0f), Color(255, 255, 255), 1.5f);
    }

    float nextLogTime_{};
};

RSDK_REGISTER_PLUGIN(RenderingExample);
