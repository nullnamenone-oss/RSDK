#define Plugin_Id              "Guide.Menu"
#define Plugin_Name            "Getting Started - Menu"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Menu and localization example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

class MenuExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        page_ = Menu::CreatePage("guide_menu", Tr("guide.menu.title", "Menu Guide"), MenuCategory::Utility);
        if (!page_)
            return false;

        const MenuTab generalTab = page_.AddTab("general", Tr("guide.menu.tab.general", "General"));

        // Section is optional: small controls may be added directly to the tab.
        enabled_ = generalTab.AddLinkedBool(links_.Bools, "enabled", Tr("guide.menu.enabled", "Enabled"), true);

        const MenuSection behavior = generalTab.AddSection("behavior", Tr("guide.menu.section.behavior", "Behavior"));
        const MenuGroup casting = behavior.AddGroup("casting", Tr("guide.menu.group.casting", "Casting"));

        minMana_ = casting.AddSliderInt("min_mana", Tr("guide.menu.min_mana", "Minimum mana %"), 30, 0, 100);
        mode_ = casting.AddCombo(
            "mode",
            Tr("guide.menu.mode", "Mode"),
            {
                Tr("guide.menu.mode.safe", "Safe"),
                Tr("guide.menu.mode.normal", "Normal"),
                Tr("guide.menu.mode.aggressive", "Aggressive")
            },
            1);
        rangeScale_ = casting.AddSliderFloat("range_scale", Tr("guide.menu.range_scale", "Range scale"), 1.0f, 0.5f, 1.5f);
        comboKey_ = casting.AddKeyBind("combo_key", Tr("guide.menu.combo_key", "Combo key"), 0x20, KeyBindType::Press); // VK_SPACE

        const MenuGroup advanced = casting.AddGroup("advanced", Tr("guide.menu.group.advanced", "Advanced"), false);
        debugToggle_ = advanced.AddKeyBind("debug_toggle", Tr("guide.menu.debug_toggle", "Debug overlay"), 'K', KeyBindType::Toggle);
        drawColor_ = casting.AddColor("draw_color", Tr("guide.menu.draw_color", "Draw color"), Color(80, 190, 255, 220));

        enabled_.SetTooltip(Tr("guide.menu.tooltip.enabled", "Enables this example feature."));

        // Menu items are persistent by default. Mark transient/debug-only state
        // non-persistent when you do not want it saved to config.
        debugToggle_.SetPersistent(false);

        // Same-line is useful for compact pairs of controls.
        drawColor_.SetSameLine(true, 8.0f);

        resetButton_ = behavior.AddButton("reset", Tr("guide.menu.reset", "Reset settings"));
        behavior.AddText("hint", Tr("guide.menu.hint", "IDs stay stable; visible text can be translated."), Color(220, 220, 220));
        behavior.AddSeparator();

        // Named member handler without an inline lambda.
        Events::OnGameUpdate.Subscribe(this, &MenuExample::OnGameUpdate);

        Log::Info("[Menu] page=guide_menu locale=%s", Localization::Locale().c_str());
        Log::Info("[Menu] defaults: enabled=%d minMana=%d mode=%d rangeScale=%.2f",
            enabled_.Value() ? 1 : 0, minMana_.Value(), mode_.Value(), rangeScale_.Value());
        Log::Info("[Menu] comboKey=0x%X debugToggle=0x%X", comboKey_.Key(), debugToggle_.Key());
        return true;
    }

private:
    void OnGameUpdate()
    {
        if (resetButton_.ConsumePressed())
        {
            enabled_.SetValue(true);
            minMana_.SetValue(30);
            mode_.SetValue(1);
            rangeScale_.SetValue(1.0f);
            Log::Info("[Menu] Reset button pressed; defaults restored.");
        }

        if (!enabled_.Value() || !comboKey_.Active())
            return;

        const float now = Game::TimeGame();
        if (now < nextSettingsLog_)
            return;
        nextSettingsLog_ = now + 1.0f;

        const bool linkedEnabled = links_.Bools["enabled"].Value();
        Log::Info("[Menu] enabled=%d linkedEnabled=%d mode=%d minMana=%d rangeScale=%.2f debug=%d",
            enabled_.Value() ? 1 : 0, linkedEnabled ? 1 : 0, mode_.Value(), minMana_.Value(), rangeScale_.Value(),
            debugToggle_.Active() ? 1 : 0);
    }

    MenuPage page_{};
    MenuLinks links_{};
    MenuBool enabled_{};
    MenuSliderInt minMana_{};
    MenuSliderFloat rangeScale_{};
    MenuCombo mode_{};
    MenuKeyBind comboKey_{};
    MenuKeyBind debugToggle_{};
    MenuColor drawColor_{};
    MenuButton resetButton_{};
    float nextSettingsLog_{};
};

RSDK_REGISTER_PLUGIN(MenuExample);
