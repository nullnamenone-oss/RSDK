#define Plugin_Id              "Guide.Localization"
#define Plugin_Name            "Getting Started - Localization"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Localization API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

#include <string>

using namespace rsdk;

class LocalizationExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        page_ = Menu::CreatePage("localization", Tr("guide.localization.title", "Localization"), MenuCategory::Utility);
        if (!page_)
            return false;

        const MenuTab tab = page_.AddTab("general", Tr("guide.localization.tab.general", "General"));
        message_ = tab.AddBool("show_message", Tr("guide.localization.show_message", "Show message"), true);

        Events::OnGameUpdate.Subscribe(this, &LocalizationExample::OnGameUpdate);

        lastLocale_ = Localization::Locale();
        Log::Info("[Localization] current locale=%s", lastLocale_.c_str());
        Log::Info("[Localization] immediate string=%s", Localization::Tr("guide.localization.ready", "Ready").c_str());
        Log::Info("[Localization] menu labels use Tr(...) descriptors so they can update when locale changes.");
        return true;
    }

private:
    void OnGameUpdate()
    {
        const std::string locale = Localization::Locale();
        if (locale != lastLocale_)
        {
            Log::Info("[Localization] locale changed: %s -> %s", lastLocale_.c_str(), locale.c_str());
            lastLocale_ = locale;
        }

        if (!message_.Value())
            return;

        const float now = Game::TimeGame();
        if (now < nextLogTime_)
            return;
        nextLogTime_ = now + 3.0f;

        // Use Localization::Tr() when you need an immediate std::string outside the menu.
        const std::string ready = Localization::Tr("guide.localization.ready", "Ready");
        Log::Info("[Localization] locale=%s text=%s", locale.c_str(), ready.c_str());
    }

    MenuPage page_{};
    MenuBool message_{};
    std::string lastLocale_{};
    float nextLogTime_{};
};

RSDK_REGISTER_PLUGIN(LocalizationExample);
