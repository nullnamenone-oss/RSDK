#define Plugin_Id              "Guide.Xerath"
#define Plugin_Name            "Xerath - Getting Started Example"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Simple Xerath champion plugin tutorial for RSDK 5.2.0"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/Spell.h>
#include <Common/TargetSelector.h>

#include <limits>
#include <string>

using namespace rsdk;

class XerathExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.ChampionName() != "Xerath")
            return false;

        ConfigureSpells();
        if (!ConfigureMenu())
            return false;

        // For member handlers, Subscribe(object, method) keeps the registration readable
        // without introducing an inline lambda.
        Events::OnGameUpdate.Subscribe(this, &XerathExample::OnGameUpdate);
        Events::OnDraw.Subscribe(this, &XerathExample::OnDraw);

        Log::Info("[Xerath] Simple tutorial plugin loaded.");
        Log::Info("[Xerath] Q range=%.0f W range=%.0f E range=%.0f", q_.Range(), w_.Range(), e_.Range());
        Log::Info("[Xerath] Prediction=%d TargetSelector=%d Orbwalker=%d",
            Prediction::IsAvailable() ? 1 : 0, TargetSelector::IsAvailable() ? 1 : 0, Orbwalker::IsAvailable() ? 1 : 0);
        return true;
    }

private:
    void ConfigureSpells()
    {
        // Historical LeagueSharp Xerath values are used for tutorial structure.
        // Verify spell data against the current game before shipping a real champion plugin.
        q_.SetSkillshot(0.60f, 100.0f, std::numeric_limits<float>::max(), false, SkillshotType::Line)
          .SetMinHitChance(HitChance::High);

        w_.SetSkillshot(0.70f, 150.0f, std::numeric_limits<float>::max(), false, SkillshotType::Circle)
          .SetMinHitChance(HitChance::High);

        e_.SetSkillshot(0.25f, 70.0f, 1600.0f, true, SkillshotType::Line)
          .SetCollisionObjects(CollisionableObjects::Minions | CollisionableObjects::YasuoWall)
          .SetMinHitChance(HitChance::High);

        const bool chargedReady = q_.SetCharged(
            "XerathArcanopulseChargeUp",
            "XerathArcanopulseChargeUp",
            750.0f,
            1550.0f,
            1.50f);

        if (!chargedReady)
            Log::Warning("[Xerath] Q charged-spell tracking could not be initialized.");
        else
            Log::Info("[Xerath] Q charged-spell tracking initialized (750 -> 1550 range).");
    }

    bool ConfigureMenu()
    {
        page_ = Menu::CreatePage(
            "xerath_guide",
            Tr("xerath.title", "Xerath"),
            MenuCategory::Champion);

        if (!page_)
            return false;

        const MenuTab combo = page_.AddTab(
            "combo",
            Tr("xerath.tab.combo", "Combo"));

        useQCombo_ = combo.AddBool("use_q", Tr("xerath.combo.use_q", "Use Q"), true);
        useWCombo_ = combo.AddBool("use_w", Tr("xerath.combo.use_w", "Use W"), true);
        useECombo_ = combo.AddBool("use_e", Tr("xerath.combo.use_e", "Use E"), true);

        const MenuTab harass = page_.AddTab(
            "harass",
            Tr("xerath.tab.harass", "Harass"));

        useQHarass_ = harass.AddBool("use_q", Tr("xerath.harass.use_q", "Use Q"), true);
        useWHarass_ = harass.AddBool("use_w", Tr("xerath.harass.use_w", "Use W"), true);
        harassMana_ = harass.AddSliderInt(
            "min_mana",
            Tr("xerath.harass.min_mana", "Minimum mana %"),
            40,
            0,
            100);

        const MenuTab drawing = page_.AddTab(
            "drawing",
            Tr("xerath.tab.drawing", "Drawing"));

        drawQ_ = drawing.AddBool("draw_q", Tr("xerath.draw.q", "Draw Q range"), true);
        drawW_ = drawing.AddBool("draw_w", Tr("xerath.draw.w", "Draw W range"), false);
        drawE_ = drawing.AddBool("draw_e", Tr("xerath.draw.e", "Draw E range"), false);

        return true;
    }

    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.IsDead() || !Game::IsReady())
            return;

        switch (Orbwalker::ActiveMode())
        {
        case Orbwalker::Mode::Combo:
            Combo();
            break;
        case Orbwalker::Mode::Mixed:
            Harass();
            break;
        default:
            break;
        }
    }

    void Combo()
    {
        if (HandleChargedQ(useQCombo_.Value()))
            return;

        if (useWCombo_.Value() && w_.IsReady())
        {
            const Object target = w_.GetTarget();
            if (target && w_.CastIfHitchanceMinimum(target, HitChance::High))
            {
                Log::Info("[Xerath] Combo W cast on %s", target.ChampionName().c_str());
                return;
            }
        }

        if (useECombo_.Value() && e_.IsReady())
        {
            const Object target = e_.GetTarget();
            if (target && e_.CastIfHitchanceMinimum(target, HitChance::High))
            {
                Log::Info("[Xerath] Combo E cast on %s", target.ChampionName().c_str());
                return;
            }
        }

        StartQIfPossible(useQCombo_.Value());
    }

    void Harass()
    {
        // Finish an already-started Q before applying the mana gate. Otherwise
        // a menu threshold crossed mid-charge could leave Q charging while we return early.
        if (HandleChargedQ(useQHarass_.Value()))
            return;

        const Object player = ObjectManager::LocalPlayer();
        if (!player || player.ManaPercent() < static_cast<float>(harassMana_.Value()))
            return;

        if (useWHarass_.Value() && w_.IsReady())
        {
            const Object target = w_.GetTarget();
            if (target && w_.CastIfHitchanceMinimum(target, HitChance::High))
            {
                Log::Info("[Xerath] Harass W cast on %s", target.ChampionName().c_str());
                return;
            }
        }

        StartQIfPossible(useQHarass_.Value());
    }

    bool HandleChargedQ(const bool enabled)
    {
        if (!q_.IsCharging())
            return false;

        // Once Q is charging, stop the normal Combo/Harass chain so W/E are not
        // fired while the charged-spell helper owns the cast flow. If the menu
        // option was disabled mid-charge, simply keep the charge isolated here.
        if (!enabled)
            return true;

        const Object target = q_.GetTarget();
        if (!target)
            return true; // Q is already charging; do not start unrelated casts here.

        const PredictionOutput prediction = q_.GetPrediction(target);
        if (prediction.Hitchance < HitChance::High)
            return true;

        if (!q_.IsInRange(prediction.CastPosition))
            return true;

        const bool released = q_.ReleaseCharge(prediction.CastPosition);
        Log::Info("[Xerath] Q release target=%s hitchance=%d result=%d",
            target.ChampionName().c_str(), static_cast<int>(prediction.Hitchance), released ? 1 : 0);
        return true;
    }

    void StartQIfPossible(const bool enabled)
    {
        if (!enabled || !q_.IsReady() || q_.IsCharging())
            return;

        const Object target = q_.GetTarget();
        if (target)
        {
            const bool started = q_.StartCharging();
            Log::Info("[Xerath] Q charge start target=%s result=%d", target.ChampionName().c_str(), started ? 1 : 0);
        }
    }

    void OnDraw()
    {
        if (!Render::IsReady())
            return;

        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return;

        const Vec3 position = player.Position();
        if (drawQ_.Value())
            Render::Circle3D(position, q_.Range(), Color(90, 190, 255, 220), 2.0f, 96u, true);
        if (drawW_.Value())
            Render::Circle3D(position, w_.Range(), Color(255, 180, 80, 220), 2.0f, 96u, true);
        if (drawE_.Value())
            Render::Circle3D(position, e_.Range(), Color(255, 100, 100, 220), 2.0f, 96u, true);
    }

private:
    Spell q_{SpellSlot::_Q, 1550.0f, DamageType::Magical};
    Spell w_{SpellSlot::_W, 1100.0f, DamageType::Magical};
    Spell e_{SpellSlot::_E, 1050.0f, DamageType::Magical};

    MenuPage page_{};
    MenuBool useQCombo_{};
    MenuBool useWCombo_{};
    MenuBool useECombo_{};
    MenuBool useQHarass_{};
    MenuBool useWHarass_{};
    MenuSliderInt harassMana_{};
    MenuBool drawQ_{};
    MenuBool drawW_{};
    MenuBool drawE_{};
};

RSDK_REGISTER_PLUGIN(XerathExample);
