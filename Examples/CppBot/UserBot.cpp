// Define stable metadata and implement your event logic below.
#define Plugin_Id              "CppBot.Example"
#define Plugin_Name            "My_Test"
#define Author_Name            "Author_Name"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Example plugin built with the RSDK 5.2.0 C++ framework"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/Items.h>
#include <Common/HeroManager.h>
#include <Common/TargetSelector.h>

#include <Windows.h>

#include "DebugPrint.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

using namespace rsdk;

namespace userbot::events
{
    namespace
    {
        bool g_isClearLog = false;

        rsdk::MenuPage g_menuPage;
        rsdk::MenuBool g_menuEnabled;
        rsdk::MenuLinks g_menuLinks;
        rsdk::MenuSliderInt g_targetPriority;
        rsdk::MenuSliderFloat g_spellRange;
        rsdk::MenuCombo g_mode;
        rsdk::MenuKeyBind g_comboKey;
        rsdk::MenuButton g_testButton;
        rsdk::MenuBool g_drawExamples;
        rsdk::MenuBool g_rainbowGlow;
        rsdk::MenuColor g_textColor;
        rsdk::MenuColor g_rangeColor;
        rsdk::MenuColor g_healthColor;
        rsdk::MenuColor g_iconBorderColor;
        rsdk::MenuCombo g_iconShape;
        rsdk::Texture g_exampleTexture;
        bool g_textureLoadAttempted{};

        // Tiny PNG kept in the plugin binary to demonstrate Texture::LoadFromMemory().
        constexpr std::uint8_t kExampleIconPng[] = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7A, 0x7A, 0xF4, 0x00, 0x00, 0x01, 0x11, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0xED, 0x57, 0x3B, 0x12, 0xC2, 0x20, 0x10, 0x0D, 0x0C, 0x6D, 0xEE, 0xE2, 0x58, 0x39, 0x93, 0x13, 0x78, 0x00, 0x6A, 0xAB, 0x74, 0x39, 0x8E, 0x5D, 0x2A, 0xEB, 0x1C, 0xC0, 0x13, 0xD8, 0x7A, 0x19, 0x0F, 0xA0, 0x15, 0x23, 0x22, 0x6F, 0x59, 0x96, 0x64, 0xA0, 0x08, 0x55, 0x98, 0xFD, 0xBC, 0x97, 0x05, 0xF6, 0xA3, 0x0E, 0x27, 0xFB, 0xEE, 0x2A, 0x2E, 0xDD, 0x55, 0x5E, 0xD5, 0x09, 0x18, 0x89, 0xD1, 0xE3, 0xFA, 0x82, 0xB2, 0x61, 0xEA, 0xB7, 0x21, 0x40, 0x81, 0x22, 0x3D, 0x0E, 0x19, 0x23, 0x01, 0xBF, 0x3C, 0xEF, 0x50, 0xF7, 0x76, 0x3C, 0xFF, 0xD8, 0xA5, 0x48, 0xA8, 0xD4, 0x2B, 0xF0, 0xC1, 0x29, 0x60, 0x8A, 0x08, 0x45, 0x82, 0x24, 0xE0, 0xC0, 0x73, 0x80, 0x11, 0x11, 0x44, 0x42, 0x6F, 0x09, 0xEE, 0xDB, 0xA3, 0x3B, 0x14, 0x8D, 0x00, 0x02, 0x5F, 0xC6, 0xEF, 0xB7, 0x9D, 0xFF, 0x9D, 0x51, 0x72, 0x14, 0x09, 0xCD, 0xBD, 0xED, 0xBE, 0x73, 0xC9, 0x1E, 0xF9, 0xD7, 0xA9, 0xD0, 0xAD, 0xB5, 0x90, 0xBF, 0xB6, 0x52, 0x31, 0x75, 0xF1, 0xC2, 0x33, 0xCD, 0xDD, 0xA3, 0x0B, 0x99, 0x95, 0x8A, 0x63, 0x4E, 0x73, 0xE4, 0x7B, 0x35, 0xDC, 0x09, 0x88, 0x09, 0x2C, 0x23, 0x4E, 0x2C, 0x94, 0x4D, 0x36, 0x01, 0x97, 0x26, 0xFD, 0x4A, 0x26, 0x71, 0xEA, 0xF4, 0x42, 0xFD, 0x58, 0x3A, 0xD6, 0x6B, 0x3C, 0xBF, 0x12, 0x3B, 0xCD, 0xA9, 0xE7, 0xBE, 0x33, 0xEA, 0x38, 0x7C, 0x19, 0x2A, 0x46, 0xC9, 0x8E, 0x68, 0x98, 0x7A, 0x58, 0x90, 0xEC, 0x8C, 0xC3, 0xCB, 0xFD, 0xF3, 0xB0, 0x1A, 0xC2, 0x86, 0x24, 0xD5, 0x0F, 0xC4, 0x08, 0x20, 0x60, 0xAA, 0x29, 0xA9, 0xDE, 0x11, 0xB5, 0xDD, 0x13, 0x96, 0x76, 0xC5, 0x9C, 0xD6, 0x5C, 0x71, 0x67, 0x43, 0xEE, 0x5C, 0x90, 0x3B, 0xA4, 0x18, 0x89, 0xB3, 0x2A, 0x93, 0x51, 0x09, 0x48, 0xD3, 0xC5, 0xE8, 0x03, 0xAB, 0x70, 0x96, 0xCE, 0x9A, 0x8B, 0x20, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
        };

        void CreateExampleMenu()
        {
            g_menuPage = rsdk::Menu::CreatePage("myTest", rsdk::Tr("cppbot.title", "My_Test"), rsdk::MenuCategory::Utility);
            if (!g_menuPage)
                return;

            const rsdk::MenuTab comboTab = g_menuPage.AddTab("combo", rsdk::Tr("cppbot.tab.combo", "Combo"));
            const rsdk::MenuSection general = comboTab.AddSection("general", rsdk::Tr("cppbot.section.general", "General"));

            const rsdk::MenuGroup basic = general.AddGroup("basic", "Basic");
            g_menuEnabled = basic.AddBool("enabled", rsdk::Tr("cppbot.enabled", "Enabled"), true);
            g_mode = basic.AddLinkedCombo(g_menuLinks.Combos, "mode", "Mode", {"Combo", "Harass", "Lane Clear", "Flee"}, 0);

            const rsdk::MenuGroup keys = general.AddGroup("keys", "Keys");
            g_comboKey = keys.AddLinkedKeyBind(g_menuLinks.KeyBinds, "combo_key", "Combo key", VK_SPACE, rsdk::KeyBindType::Press);
            keys.AddLinkedKeyBind(g_menuLinks.KeyBinds, "only_selected", "Only selected target", 'Z', rsdk::KeyBindType::Toggle);

            const rsdk::MenuGroup targeting = general.AddGroup("targeting", "Targeting");
            g_targetPriority = targeting.AddLinkedSliderInt(g_menuLinks.SliderInts, "target_priority", "Target priority", 3, 1, 5);
            const rsdk::MenuGroup selectedTarget = targeting.AddGroup("selected_target", "Selected Target");
            selectedTarget.AddLinkedBool(g_menuLinks.Bools, "prefer_selected", "Prefer selected target", true);
            const rsdk::MenuGroup advanced = selectedTarget.AddGroup("advanced", "Advanced", false);
            advanced.AddLinkedBool(g_menuLinks.Bools, "ignore_shields", "Ignore shields", false);
            (void)advanced.AddSliderInt("min_health", "Min target health %", 15, 0, 100);

            const rsdk::MenuSection spells = comboTab.AddSection("spells", "Spells");
            const rsdk::MenuGroup qSettings = spells.AddGroup("q", "Q Settings");
            qSettings.AddLinkedBool(g_menuLinks.Bools, "combo_q", "Use Q / 目标选择", true);
            g_spellRange = qSettings.AddLinkedSliderFloat(g_menuLinks.SliderFloats, "q_range", "Q range", 900.0f, 100.0f, 2000.0f);
            const rsdk::MenuGroup qAdvanced = qSettings.AddGroup("advanced", "Advanced", false);
            (void)qAdvanced.AddBool("only_killable", "Only cast on killable target", false);

            const rsdk::MenuTab harassTab = g_menuPage.AddTab("harass", "Harass");
            const rsdk::MenuSection harass = harassTab.AddSection("general", "General");
            harass.AddLinkedBool(g_menuLinks.Bools, "harass_enabled", "Enabled", false);

            const rsdk::MenuTab drawingTab = g_menuPage.AddTab("drawing", "Drawing");
            const rsdk::MenuSection drawing = drawingTab.AddSection("examples", "Render API Examples");
            const rsdk::MenuGroup drawingGeneral = drawing.AddGroup("general", "General");
            g_drawExamples = drawingGeneral.AddBool("enabled", "Draw examples", true);
            g_rainbowGlow = drawingGeneral.AddBool("rainbow_glow", "Rainbow circle glow", true);
            drawingGeneral.AddText("note", "All 2D/minimap positions below are derived from LocalPlayer.Position() through Game::WorldToScreen/WorldToMinimap.", rsdk::Color(255, 210, 80));

            const rsdk::MenuGroup colors = drawing.AddGroup("colors", "Colors");
            g_textColor = colors.AddLinkedColor(g_menuLinks.Colors, "text_color", "Text color", rsdk::Color(255, 230, 90));
            g_rangeColor = colors.AddLinkedColor(g_menuLinks.Colors, "range_color", "Line / circle color", rsdk::Color(80, 190, 255, 210));
            g_healthColor = colors.AddLinkedColor(g_menuLinks.Colors, "health_color", "Health fill color", rsdk::Color(45, 220, 85));
            g_iconBorderColor = colors.AddLinkedColor(g_menuLinks.Colors, "icon_border", "Icon border color", rsdk::Color(255, 205, 70));

            const rsdk::MenuGroup icon = drawing.AddGroup("icon", "Minimap Icon");
            g_iconShape = icon.AddCombo("shape", "Shape", {"Square", "Rounded", "Circle"}, 2);
            icon.AddText("memory", "The example icon is loaded directly from PNG bytes with Texture::LoadFromMemory(). Texture::LoadFromFile() is also available.");

            const rsdk::MenuTab debugTab = g_menuPage.AddTab("debug", "Debug");
            const rsdk::MenuSection debug = debugTab.AddSection("general", "Debug");
            debug.AddText("hint", "MenuGroup is a collapsible tree inside a Section. Nesting is limited to three group levels.");
            g_testButton = debug.AddButton("test_button", "Write test log");
        }

        void ReleaseDrawResources() noexcept
        {
            g_exampleTexture.Reset();
            g_textureLoadAttempted = false;
        }

        const char* BoolText(const bool value) noexcept
        {
            return value ? "true" : "false";
        }

    }

    void OnTick()
    {
        // Menu values are read directly; there is no need to query the config/menu tree again.
        if (g_testButton && g_testButton.ConsumePressed())
        {
            rsdk::Log::Info("[Menu] enabled=%s useQ=%s priority=%d range=%.1f mode=%d comboKey=0x%X active=%s",
                BoolText(g_menuEnabled.Value()), BoolText(g_menuLinks.Bools["combo_q"].Value()), g_targetPriority.Value(),
                g_spellRange.Value(), g_mode.Value(), g_comboKey.Key(), BoolText(g_comboKey.Active()));
        }
    }

    void OnDraw()
    {
        if (!g_drawExamples.Value() || !rsdk::Render::IsReady())
            return;

        const rsdk::Object player = rsdk::ObjectManager::LocalPlayer();
        if (!player)
            return;
        
        //player.GetPath(..);
        //player.TeleportState();
        //player.CanUseSpell(SpellSlot::_Q);
        
        const Vec3 playerPos = player.Position();
        //const Vec2 screen = rsdk::Game::WorldToScreen(playerPos);
        const Vec2 screen = player.HealthBarScreenPosition();
        const Vec2 minimap = rsdk::Game::WorldToMinimap(playerPos);
        const rsdk::Color textColor = g_textColor.Value();
        const rsdk::Color rangeColor = g_rangeColor.Value();

        // 2D primitives: all screen-space coordinates originate from the player's 3D world position.
        if (screen.IsValid())
        {
            const Vec2 textPos = screen + Vec2(-128.0f, -72.0f);
            const Vec2 lineStart = screen + Vec2(-65.0f, -42.0f);
            const Vec2 lineEnd = screen + Vec2(65.0f, -42.0f);
            const Vec2 circleCenter = screen + Vec2(92.0f, -18.0f);
            //rsdk::Render::Text(textPos, "RSDK 4.8 colored text", textColor, 22.0f);
            //rsdk::Render::Line2D(lineStart, lineEnd, rangeColor, 2.0f);
            //rsdk::Render::Circle2D(circleCenter, 150.0f, rangeColor, 1.0f, 90u);

            // FilledRect + Rect can be combined into HP/Mana/progress bars.
            const Vec2 barPos = screen + Vec2(-60.0f, 34.0f);
            const Vec2 barSize(100.0f, 10.0f);
            
            const float maxHealth = player.MaxHealth();
            const float healthRatio = maxHealth > 0.0f ? std::clamp(player.Health() / maxHealth, 0.0f, 1.0f) : 0.0f;
            rsdk::Render::FilledRect(barPos, barSize, rsdk::Color(15, 18, 24, 220), 3.0f);
            rsdk::Render::FilledRect(barPos, Vec2(barSize.x * healthRatio, barSize.y), g_healthColor.Value(), 3.0f);
            rsdk::Render::Rect(barPos, barSize, rsdk::Color(0, 0, 0, 230), 1.0f, 3.0f);
        }

        // 3D/world-space primitives. Circle3D draws on the ground XZ plane and projects internally.
        rsdk::Render::Line3D(playerPos + Vec3(-90.0f, 0.0f, 70.0f), playerPos + Vec3(90.0f, 0.0f, 70.0f), rangeColor, 2.0f);
        rsdk::Render::Circle3D(playerPos, 600.0f, rangeColor, 1.0f, 96u, true);
        if (g_rainbowGlow.Value())
        {
            rsdk::Render::CircleGlow3D(playerPos, 700.0f, rsdk::Color::Rainbow(0.10f, 150), 2.0f, 12.0f, 4u, 160u, true);
            rsdk::Render::RainbowCircle3D(playerPos, 500.0f, 2.0f, 160u, 0.10f, true, 220u, true);
        }

        // Minimap image: the icon position is derived from the same player 3D position.
        if (minimap.IsValid())
        {
            if (!g_exampleTexture && !g_textureLoadAttempted)
            {
                g_textureLoadAttempted = true;
                g_exampleTexture = rsdk::Texture::LoadFromMemory(kExampleIconPng, sizeof(kExampleIconPng));
                // 
                // File variant example: 
                //g_exampleTexture = rsdk::Texture::LoadFromFile("D:\\__FUKING\\RSDK_Pro_v2.6_RawP\\bin\\Release\\Assets\\Images\\Blitz.png");
            }

            if (g_exampleTexture)
            {
                rsdk::ImageStyle style;
                style.shape = static_cast<rsdk::ImageShape>(std::clamp(g_iconShape.Value(), 0, 2));
                style.borderColor = g_iconBorderColor.Value();
                style.borderThickness = 2.0f;
                style.rounding = 7.0f;
                const Vec2 iconSize(35.0f, 35.0f);
                const Vec2 iconPos = minimap - iconSize * 0.5f + Vec2(34.0f, -8.0f);
                rsdk::Render::Image(g_exampleTexture, iconPos, iconSize, style);
            }
        }
    }

    void OnIssueOrder(rsdk::IssueOrderEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;
        const rsdk::Object& target = eventData.target;
        const Vec3& position = eventData.position;

        rsdk::Log::Info("[OnIssueOrder] Sender = %llX, Team: %X, Id = %X, NetworkId = %X, Name: %s",
            static_cast<unsigned long long>(sender.Address()), sender.Team(), sender.Id(), sender.NetworkId(), sender.Name().c_str());

        rsdk::Log::Info("  - Order = %u, IsAttackMove = %s, Process = %s",
            static_cast<unsigned>(eventData.order), BoolText(eventData.isAttackMove), BoolText(eventData.process));

        rsdk::Log::Info("  - Position = (%.3f, %.3f, %.3f)", position.x, position.y, position.z);

        rsdk::Log::Info("  - Target = %llX, Team: %X, TargetId = %X, TargetNetworkId = %X\n", static_cast<unsigned long long>(target.Address()), target.Team(), target.Id(), target.NetworkId(), target.Name().c_str());
    }

    void OnNewPath(const rsdk::NewPathEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;
        if (!sender.IsMe())
            return;

        rsdk::Log::Info("[OnNewPath] Sender = %llX, Team: %X, Id = %X, NetworkId = %X, Name: %s",
            static_cast<unsigned long long>(sender.Address()), sender.Team(), sender.Id(), sender.NetworkId(), sender.Name().c_str());

        rsdk::Log::Info("  - PathCount = %zu, IsDash = %s, DashSpeed = %.3f, DashGravityCount=%u",
            eventData.path.size(), BoolText(eventData.isDash), eventData.dashSpeed, static_cast<unsigned>(eventData.dashGravityCount));

        const std::size_t printCount = std::min<std::size_t>(eventData.path.size(), 20);
        for ( std::size_t i = 0; i < printCount; ++i)
        {
            const Vec3& point = eventData.path[i];
            rsdk::Log::Info("    + Path[%zu] = (%.3f, %.3f, %.3f)", i, point.x, point.y, point.z);
        }
    }

    void OnProcessSpell(const rsdk::ProcessSpellEvent& eventData)
    {
        //return;

        const rsdk::Object& sender = eventData.sender;
        const rsdk::SpellCast& spellCast = eventData.spellCast;
        const rsdk::Object caster = spellCast.Caster();
        const rsdk::Object target = spellCast.Target();
        const std::uint32_t missileNetworkId = spellCast.MissileNetworkId();        
        const Vec3 startPos = spellCast.StartPos();
        const Vec3 endPos = spellCast.EndPos();
        const Vec3 castEndPos = spellCast.CastEndPos();

        rsdk::Log::Info("[OnProcessSpell] Sender = %llX, Team: %X, Id = %X, NetworkId = %X, IsMe = %s, ChampionName: %s",
            static_cast<unsigned long long>(sender.Address()), sender.Team(), sender.Id(), sender.NetworkId(), BoolText(sender.IsMe()), sender.ChampionName().c_str());

        rsdk::Log::Info("  - SpellCast = %llX, SpellName = %s, Slot = %u, IsAutoAttack = %s",
            static_cast<unsigned long long>(spellCast.Address()), spellCast.SpellName(), static_cast<unsigned>(spellCast.Slot()), BoolText(spellCast.IsAutoAttack()));

        rsdk::Log::Info("  - Caster = %llX, Target = %llX, missileNetworkId = %X",
            static_cast<unsigned long long>(caster.Address()), static_cast<unsigned long long>(target.Address()), missileNetworkId);
        
        rsdk::Log::Info("  - StartPos = (%.3f, %.3f, %.3f), EndPos = (%.3f, %.3f, %.3f)", startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z);
        rsdk::Log::Info("  - CastEndPos = (%.3f, %.3f, %.3f)", castEndPos.x, castEndPos.y, castEndPos.z);

        rsdk::Log::Info("  - CastTime = %.4f, CastEndTime = %.4f", spellCast.CastTime(), spellCast.CastEndTime());
        rsdk::Log::Info("  - AttackCastDelay = %.3f, AttackDelay = %.3f, CooldownExpires = %.3f", spellCast.AttackCastDelay(), spellCast.AttackDelay(), spellCast.CooldownExpires());
        rsdk::Log::Info("  - ProjectileSpeed = %.3f, Range = %.3f, Radius = %.3f, Width = %.3f\n", spellCast.ProjectileSpeed(), spellCast.Range(), spellCast.Radius(), spellCast.Width());
    }

    void OnDoCast(const rsdk::DoCastSpellEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;
        const rsdk::SpellCast& spellCast = eventData.spellCast;
        const rsdk::Object target = spellCast.Target();

        const Vec3 startPos = spellCast.StartPos();
        const Vec3 endPos = spellCast.EndPos();
        const Vec3 castEndPos = spellCast.CastEndPos();

        rsdk::Log::Info("[OnDoCast] Sender = %llX, Team: %X, Id = %X, NetworkId = %X, IsMe = %s, ChampionName: %s",
            static_cast<unsigned long long>(sender.Address()), sender.Team(), sender.Id(), sender.NetworkId(), BoolText(sender.IsMe()), sender.ChampionName().c_str());

        rsdk::Log::Info("  - SpellCast = %llX, SpellName = %s, Slot = %u, IsAutoAttack = %s",
            static_cast<unsigned long long>(spellCast.Address()), spellCast.SpellName(), static_cast<unsigned>(spellCast.Slot()), BoolText(spellCast.IsAutoAttack()));

        rsdk::Log::Info("  - Target = %llX, Team: %X, TargetId = %X, TargetNetworkId = %X, ChampionName: %s",
            static_cast<unsigned long long>(target.Address()), sender.Team(), target.Id(), target.NetworkId(), target.ChampionName().c_str());

        rsdk::Log::Info("  - StartPos = (%.3f, %.3f, %.3f), EndPos = (%.3f, %.3f, %.3f)", startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z);
        rsdk::Log::Info("  - CastEndPos = (%.3f, %.3f, %.3f)", castEndPos.x, castEndPos.y, castEndPos.z);

        rsdk::Log::Info("  - CastTime = %.4f, CastEndTime = %.4f", spellCast.CastTime(), spellCast.CastEndTime());
        rsdk::Log::Info("  - AttackCastDelay = %.4f, AttackDelay = %.4f, CooldownExpires = %.4f", spellCast.AttackCastDelay(), spellCast.AttackDelay(), spellCast.CooldownExpires());
        rsdk::Log::Info("  - ProjectileSpeed = %.4f, Range = %.3f, Radius = %.3f, Width = %.3f\n", spellCast.ProjectileSpeed(), spellCast.Range(), spellCast.Radius(), spellCast.Width());
    }

    void OnPlayAnimation(const rsdk::PlayAnimationEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;

        rsdk::Log::Info("[OnPlayAnimation] Sender = %llX, Id = %X, NetworkId = %X, AnimationName = %s, ChampionName: %s\n",
            static_cast<unsigned long long>(sender.Address()), sender.Id(), sender.NetworkId(), eventData.animationName.c_str(), sender.ChampionName().c_str());
    }

    void OnStopCast(const rsdk::StopCastEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;
      
        rsdk::Log::Info("[OnStopCast] Sender = %llX, Id = %X, NetworkId = %X, ChampionName: %s",
            static_cast<unsigned long long>(sender.Address()), sender.Id(), sender.NetworkId(), sender.ChampionName().c_str());

        if (eventData.spellCast)
        {
            rsdk::Log::Info("  - SpellCast = %llX, SpellName = %s, Slot = %u, IsAutoAttack = %s",
                static_cast<unsigned long long>(eventData.spellCast.Address()), eventData.spellCast.SpellName().c_str(),
                static_cast<unsigned>(eventData.spellCast.Slot()), BoolText(eventData.spellCast.IsAutoAttack()));
        }

        rsdk::Log::Info("  - StopAnimation = %s, DestroyMissile = %s, MissileNetworkId = %X\n",
            BoolText(eventData.stopAnimation), BoolText(eventData.destroyMissile), eventData.missileNetworkId);
    }

    void OnUpdateChargeableSpell(const rsdk::UpdateChargeableSpellEvent& eventData)
    {
		return;
        const rsdk::Object& sender = eventData.sender;
        const rsdk::SpellData& spellData = eventData.spellData;

        rsdk::Log::Info(
            "[OnUpdateChargeableSpell] Sender = %llX, ChampionName = %s, SpellData = %llX, SpellName = %s, Slot = %d, Pos = {%.2f, %.2f, %.2f}, Flag = %s\n",
            static_cast<unsigned long long>(sender.Address()), sender.ChampionName().c_str(),
            static_cast<unsigned long long>(spellData.Address()), spellData.SpellName().c_str(),
            static_cast<int>(eventData.slot), eventData.position.x, eventData.position.y, eventData.position.z, BoolText(eventData.flag));
    }

    void OnBuffAddRemove(const rsdk::BuffAddRemoveEvent& eventData)
    {
		return;
		
        const rsdk::Object& sender = eventData.sender;
        const rsdk::Buff& buff = eventData.buff;

        rsdk::Log::Info(
            "[OnBuffAddRemove] Sender = %llX, ChampionName: %s, Buff = %llX, BuffName = %s, Type: %d, Added = %s\n",
            static_cast<unsigned long long>(sender.Address()), sender.ChampionName().c_str(),
            static_cast<unsigned long long>(buff.Address()), buff.Name().c_str(), buff.Type(), BoolText(eventData.added));
    }

    void OnUpdateBuff(const rsdk::UpdateBuffEvent& eventData)
    {
        return;

        const rsdk::Object& sender = eventData.sender;
        const rsdk::Buff& buff = eventData.buff;

        rsdk::Log::Info(
            "[OnUpdateBuff] Sender = %llX, ChampionName: %s, Buff = %llX, BuffName = %s, Type: %d, stacks = %d\n",
            static_cast<unsigned long long>(sender.Address()), sender.ChampionName().c_str(),
            static_cast<unsigned long long>(buff.Address()), buff.Name().c_str(), buff.Type(), eventData.stacks);
    }
    
    void OnGameUpdate()
    {
        //return;

        // for (const rsdk::Object& target : rsdk::HeroManager::Enemies())
        // {
        //     if (!target || target.IsDead() || !target.IsVisible())
        //         continue;
        //
        //     // Gameplay logic...
        // }

        
        const rsdk::Object player = rsdk::ObjectManager::LocalPlayer();
        if (!player)
            return;

        //player.GetNavPath().DashSpeed();
        //if (player.IsMoving())
        //    player.CurrentPath();

        /*const SpellCast activeSpell = player.GetActiveSpellCast();
        if (activeSpell)
        {
            rsdk::Log::Info("activeSpell = %llX, SpellName = %s, [Slot = %u], isAA: %d",
                static_cast<unsigned long long>(activeSpell.Address()), activeSpell.SpellName(), static_cast<unsigned>(activeSpell.Slot()), activeSpell.IsAutoAttack());
            rsdk::Log::Info("  - Delay: %.4f, CastDelay: %.4f, CastTime = %.4f, CastEndTime = %.4f, CooldownExpires: %.4f",
                activeSpell.AttackDelay(), activeSpell.AttackCastDelay(), activeSpell.CastTime(), activeSpell.CastEndTime(), activeSpell.CooldownExpires());
        }*/

        return;

        static std::uint32_t updateCount = 0u;
        ++updateCount;

        // Log the first occurrence to confirm the hook, then reduce log spam.
        if (updateCount == 1u || (updateCount % 300u) == 0u)
            rsdk::Log::Info("[OnGameUpdate] Game.Time=%.3f, UpdateCount=%u\n", rsdk::Game::Time(), updateCount);
    }

    void OnSpellImpact(const rsdk::SpellImpactEvent& eventData)
    {
        return;

    }
    
    void OnCreateObject(const std::uintptr_t address, const std::uint32_t networkId)
    {
        //return;

        const rsdk::Object object(static_cast<RSDK_ObjectHandle>(address));

        rsdk::Log::Info("[OnCreateObject] Address=0x%llX, NetworkId=0x%X, Name: %s\n", static_cast<unsigned long long>(address), networkId, object.Name());
    }

    void OnDeleteObject(const std::uintptr_t address)
    {
        return;

        //rsdk::Log::Info("[OnDeleteObject] Address=0x%llX\n", static_cast<unsigned long long>(address));

        const rsdk::Missile missile(static_cast<RSDK_MissileHandle>(address));
        if (!missile || !missile.IsMissile())
            return;
    }

    __declspec(noinline) void TriggerSentryTestCrash()
    {
        ULONG_PTR args[2]{};
        args[0] = 1; // write
        args[1] = 0; // address 0x0

        RaiseException(
            EXCEPTION_ACCESS_VIOLATION,
            EXCEPTION_NONCONTINUABLE,
            2,
            args);
    }

    void OnWndProc(const RSDK_WndProcEvent& eventData)
    {
        
        //if (player.IsMoving())
        //    ;
        if (eventData.message == WM_KEYDOWN)
        {
            if (eventData.wParam == 0xC0)
            {
                //const rsdk::Object player = rsdk::ObjectManager::LocalPlayer();
                //player.CurrentPath();

                const rsdk::Object target = rsdk::TargetSelector::GetTarget(1200.0f, rsdk::TargetSelector::DamageType::Physical);
                if (target)
                {
                    //target.Direction().Perpendicular();
                    rsdk::Log::Info(
                        "[TS] Target=%s, Address=0x%llX,\n   HP=%.2f, Priority=%.2f",
                        target.ChampionName().c_str(),
                        static_cast<unsigned long long>(target.Address()),
                        target.Health(),
                        rsdk::TargetSelector::GetPriority(target)
                    );
                }
                else
                {
                    rsdk::Log::Info("[TargetSelector] No target.");
                }

                const rsdk::Object selected = rsdk::TargetSelector::GetSelectedTarget();

                if (selected)
                {
                    rsdk::Log::Info(
                        "[TS] Selected=%s, NetworkId=0x%X,\n   HP=%.2f",
                        selected.ChampionName().c_str(),
                        selected.NetworkId(), selected.Health()
                    );
                }
            }
        }

        if (eventData.message != WM_KEYUP)
            return;

        if (eventData.wParam == VK_HOME)
        {
            if (g_isClearLog)
            {
                g_isClearLog = false;
                rsdk::Log::Clear();
            }

          
            //rsdk::Game::PrintChat("WM_KEYUP -> VK_HOME");
            //userbot::debug::PrintPlayer();

            //rsdk::Log::Warning("[SENTRY TEST] Triggering intentional access violation.");
            OutputDebugStringA("[SENTRY TEST] Triggering intentional access violation.");

            //TriggerSentryTestCrash();

            //volatile int* crash = nullptr;
            //*crash = 123;
        }

        if (eventData.wParam == VK_F7)
        {
            if (g_isClearLog)
            {
                g_isClearLog = false;
                rsdk::Log::Clear();
            }

            userbot::debug::PrintHeroes();
            userbot::debug::PrintMinions();
            userbot::debug::PrintTurrets();
            userbot::debug::PrintBuildings();
            userbot::debug::PrintAllObjects();
        }

        if (eventData.wParam == VK_END)
            g_isClearLog = true;
    }


    void RegisterEvents()
    {
        rsdk::Events::OnTick += OnTick;
        rsdk::Events::OnWndProc += OnWndProc;
        rsdk::Events::OnDraw += OnDraw;

        //rsdk::Events::OnIssueOrder += OnIssueOrder;
        rsdk::Events::OnNewPath += OnNewPath;
        rsdk::Events::OnProcessSpell += OnProcessSpell;
        rsdk::Events::OnDoCast += OnDoCast;
        //rsdk::Events::OnPlayAnimation += OnPlayAnimation;
        rsdk::Events::OnStopCast += OnStopCast;
        //rsdk::Events::OnUpdateChargeableSpell += OnUpdateChargeableSpell;
        rsdk::Events::OnBuffAddRemove += OnBuffAddRemove;
        rsdk::Events::OnUpdateBuff += OnUpdateBuff;
        rsdk::Events::OnGameUpdate += OnGameUpdate;
        rsdk::Events::OnSpellImpact += OnSpellImpact;

        rsdk::Events::OnCreateObject += OnCreateObject;
        rsdk::Events::OnDeleteObject += OnDeleteObject;
    }


}

class UserBot final : public rsdk::Plugin
{
public:
    bool OnLoad() override
    {

        userbot::events::CreateExampleMenu();
        userbot::events::RegisterEvents();

        // TargetSelector is a shared Gameplay module in v4.7. It owns its menu/state and may load before or after this plugin.
        // Consumer calls fail-safe to an empty Object while TargetSelector.dll is unavailable.
        rsdk::TargetSelector::Mode = rsdk::TargetSelector::TargetingMode::AutoPriority;
        rsdk::Log::Info("TargetSelector available = %s", rsdk::TargetSelector::IsAvailable() ? "true" : "false");

        rsdk::Delay::After(1000u, []()
        {
            rsdk::Log::Info("The delayed action was executed after 1000 ms.");
        });

        rsdk::Config::SetUseRawPointer(true);
        rsdk::Log::Info("Plugin loaded. Raw pointer access = %s", rsdk::Config::UseRawPointer() ? "true" : "false");
        rsdk::Log::Info("SHIFT = RSDK menu, HOME = print LocalPlayer, F7 = print object arrays, END = clear log.");
        rsdk::Log::Info("Game.Time = %.3f", rsdk::Game::Time());
        
        return true;
    }

    void OnUnload() override
    {
        userbot::events::ReleaseDrawResources();
        rsdk::Log::Info("Plugin unloading.");
    }
};

RSDK_REGISTER_PLUGIN(UserBot);
