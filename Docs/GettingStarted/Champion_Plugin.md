# Champion Plugin — Simple Xerath Example

This chapter combines the previous topics into one small champion plugin. It is intentionally not a full competitive Xerath implementation; the goal is to show how the public RSDK APIs fit together in code that is easy to read and extend.

[Open full source: `Xerath_Simple.cpp`](../Examples/GettingStarted/Xerath_Simple.cpp)

[Open Xerath sample language catalogs](../Examples/GettingStarted/Languages/Guide.Xerath/)

## LeagueSharp reference used

The example structure was compared with the historical LeagueSharp Xerath code under:

```text
Hellsing/LeagueSharp-master/Xerath/SpellManager.cs
Hellsing/LeagueSharp-master/Xerath/ActiveModes.cs
```

That source used the familiar structure:

- Q: charged line spell;
- W: circle skillshot;
- E: collision line skillshot;
- Combo/Harass mode handlers;
- while Q is charging, finish Q handling before unrelated casts.

The spell values in `Xerath_Simple.cpp` are historical reference/tutorial values from that old LeagueSharp source. They are **not a guarantee of current live-game spell data**. Revalidate range/delay/width/speed and game spell/buff names before publishing a real champion plugin.

## Why the example omits R

Xerath R involves additional state/charge/channel behavior that would make the first end-to-end example much longer. The sample focuses on Q/W/E so the important RSDK patterns stay visible.

## 1. Reject the wrong champion

```cpp
const Object player = ObjectManager::LocalPlayer();
if (!player || player.ChampionName() != "Xerath")
    return false;
```

This is appropriate inside a champion-specific DLL. The Plugin Manager already loads champion-folder DLLs according to the current champion, but keeping the in-plugin guard makes the plugin robust when loaded manually during development.

## 2. Declare spells as plugin state

```cpp
Spell q_{SpellSlot::_Q, 1550.0f, DamageType::Magical};
Spell w_{SpellSlot::_W, 1100.0f, DamageType::Magical};
Spell e_{SpellSlot::_E, 1050.0f, DamageType::Magical};
```

They live with the plugin so charge/prediction configuration is not rebuilt every update.

## 3. Configure skillshots once

```cpp
q_.SetSkillshot(0.60f, 100.0f, std::numeric_limits<float>::max(), false, SkillshotType::Line)
  .SetMinHitChance(HitChance::High);

w_.SetSkillshot(0.70f, 150.0f, std::numeric_limits<float>::max(), false, SkillshotType::Circle)
  .SetMinHitChance(HitChance::High);

 e_.SetSkillshot(0.25f, 70.0f, 1600.0f, true, SkillshotType::Line)
  .SetCollisionObjects(CollisionableObjects::Minions | CollisionableObjects::YasuoWall)
  .SetMinHitChance(HitChance::High);
```

E demonstrates explicit collision objects. W demonstrates an effectively instant/no-projectile-speed circle by using `std::numeric_limits<float>::max()` as the historical reference speed.

## 4. Configure charged Q

```cpp
const bool chargedReady = q_.SetCharged(
    "XerathArcanopulseChargeUp",
    "XerathArcanopulseChargeUp",
    750.0f,
    1550.0f,
    1.50f);
```

Check the result so a failed charged-spell setup does not silently look valid during testing.

The core charged-Q flow is split into two functions in the example:

```text
HandleChargedQ()
    -> if already charging, try to predict and release
    -> stop the rest of Combo/Harass while Q owns the flow

StartQIfPossible()
    -> when not charging, find a target and start charging
```

This separation is important. A charged spell is stateful; treating it like a normal one-shot skillshot often leads to W/E/movement logic interfering while Q is still charging.

## 5. Menu uses localization from the start

```cpp
page_ = Menu::CreatePage(
    "xerath_guide",
    Tr("xerath.title", "Xerath"),
    MenuCategory::Champion);

const MenuTab combo = page_.AddTab(
    "combo",
    Tr("xerath.tab.combo", "Combo"));

useQCombo_ = combo.AddBool(
    "use_q",
    Tr("xerath.combo.use_q", "Use Q"),
    true);
```

Notice that `"use_q"` is not translated. It is the stable item ID. Only the visible label uses `Tr()`.

## 6. Drive champion logic from Orbwalker mode

```cpp
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
```

This keeps the champion script aligned with the shared orbwalker key/mode state instead of creating a competing set of mode keys.

## 7. Simple Combo order

The example does:

```text
If Q is already charging -> handle/release Q and return
Else try W
Else try E
Else start Q charge
```

This mirrors the high-level idea in the historical LeagueSharp script without porting all of its champion-specific heuristics.

W/E use:

```cpp
const Object target = w_.GetTarget();
if (target && w_.CastIfHitchanceMinimum(target, HitChance::High))
    return;
```

This is the concise production-style path when you only need a target plus minimum hitchance.

## 8. Why Harass handles charging before mana

The historical LeagueSharp Xerath code explicitly allowed an already-charging Q to finish before the Harass mana check. The RSDK example keeps that behavior:

```cpp
if (HandleChargedQ(useQHarass_.Value()))
    return;

const Object player = ObjectManager::LocalPlayer();
if (!player || player.ManaPercent() < static_cast<float>(harassMana_.Value()))
    return;
```

If the mana check came first, a threshold crossed during the charge could make the function return without giving the charge/release flow a chance to finish.

## 9. Charged-Q release decision

```cpp
const Object target = q_.GetTarget();
if (!target)
    return true;

const PredictionOutput prediction = q_.GetPrediction(target);
if (prediction.Hitchance < HitChance::High)
    return true;

if (!q_.IsInRange(prediction.CastPosition))
    return true;

q_.ReleaseCharge(prediction.CastPosition);
```

`q_.Range()`/`q_.IsInRange()` follow the Spell charged-range state, so the example waits until the predicted point is currently reachable.

This is deliberately simpler than the old LeagueSharp script's extra preferred-range math.

## 10. Draw ranges in `OnDraw`

```cpp
if (drawQ_.Value())
    Render::Circle3D(player.Position(), q_.Range(), Color(90, 190, 255, 220), 2.0f, 96u, true);
```

The Q range changes while charging, so drawing `q_.Range()` is more useful than hard-coding the max range.

## What to add next in a real Xerath plugin

After this skeleton works in-game, add features one at a time:

- current-game spell data validation;
- better W/E ordering and CC-aware logic;
- lane clear/farming;
- R state/charge handling;
- interrupt/anti-gapclose logic if needed;
- damage-based decisions after the full Damage provider is available;
- extra menu settings and tooltips;
- champion-specific tests around collision and charged Q timing.

Keep the same architecture: configure once, use shared providers, drive logic from events/modes, and localize visible UI from the beginning.
