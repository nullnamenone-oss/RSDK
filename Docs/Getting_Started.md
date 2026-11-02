# EonCrest / RSDK 5.2.2 — Getting Started

This is the entry point for C++ plugin authors using the public RSDK 5.2.2 API.

The guide is intentionally split into small chapters. `Getting_Started.md` stays short; detailed explanations and examples live under `GettingStarted/`, and complete `.cpp` examples live under `Examples/GettingStarted/`. Everything in this guide is kept inside the public `Docs/` folder.

For exhaustive signatures, use `API_Reference.md`. For version changes, use `CHANGELOG.md`.

## Requirements

Recommended build environment:

- Visual Studio 2022
- x64
- C++20
- the `Public/` headers from the same RSDK release

A normal plugin includes public headers and must not depend on Core implementation headers, game offsets, or private provider classes.

Most plugins start with:

```cpp
#include <RSDK.h>
```

Add Common helpers only when needed:

```cpp
#include <Common/Spell.h>
#include <Common/TargetSelector.h>
#include <Common/HeroManager.h>
#include <Common/Items.h>
#include <Common/MinionManager.h>
#include <Common/Utility.h>
```

## Runtime modules

Some public facades are backed by provider DLLs at runtime:

| Feature | Runtime provider |
|---|---|
| Object, Game, Events, Menu, Render | Core |
| Prediction and collision prediction | `Prediction.dll` |
| Target selection | `TargetSelector.dll` |
| Orbwalking | `Orbwalker.dll` |
| Health prediction | `Orbwalker.dll` |
| Damage provider | `Damage.dll` (available in this source/runtime) |

If a plugin can run without an optional provider, check availability before using it:

```cpp
if (!Prediction::IsAvailable())
    return;

if (!TargetSelector::IsAvailable())
    return;

if (!Orbwalker::IsAvailable())
    return;
```

## Minimal plugin

```cpp
#define Plugin_Id              "MyChampion.Plugin"
#define Plugin_Name            "My Champion"
#define Author_Name            "YourName"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Champion plugin"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

void OnGameUpdate()
{
    const Object player = ObjectManager::LocalPlayer();
    if (!player || player.IsDead())
        return;

    // Champion logic.
}

class MyPlugin final : public Plugin
{
public:
    bool OnLoad() override
    {
        // Preferred style for a lifetime handler: use a named function.
        Events::OnGameUpdate += OnGameUpdate;
        return true;
    }
};

RSDK_REGISTER_PLUGIN(MyPlugin);
```

`Plugin_Id` is the stable identity of the plugin. Keep it unchanged when the visible plugin name changes. It is also the localization/config domain used by the runtime.

## Plugin folders

The Plugin Manager discovers normal user plugins from:

```text
Plugins/
├─ Champion/
│  └─ <ChampionName>/
│     └─ *.dll
├─ Gameplay/
│  └─ *.dll
├─ Utility/
│  └─ *.dll
└─ Misc/
   └─ *.dll
```

The filesystem folder controls discovery/grouping. The plugin itself still chooses the category of any menu page it creates.

> **5.2.2:** `Plugins\Misc\` is supported by Plugin Manager, and plugins can use `MenuCategory::Misc` for `Menu → Misc` pages.

## The three rules worth learning first

### 1. Gameplay time is normally seconds

```cpp
const float now = Game::TimeGame();
```

Spell delay, prediction delay, HealthPrediction horizons, and Orbwalker movement delay use seconds.

`Game::Latency()` is different: it returns milliseconds.

`Delay::After()` and `Delay::Every()` also take milliseconds.

### 2. Prefer event-driven logic

Use `Events::OnGameUpdate` for normal gameplay logic and `Events::OnDraw` for rendering. Named handlers such as `Events::OnNewPath += OnNewPath;` are the recommended tutorial style; inline lambdas are also supported for small one-off handlers. Subscribe to object/spell events only when the feature needs them.

### 3. Localize UI text from the beginning

Stable IDs remain plain strings:

```cpp
"min_mana"
```

Visible UI text should normally use `Tr()`:

```cpp
MenuSliderInt minMana = tab.AddSliderInt(
    "min_mana",
    Tr("mychamp.min_mana", "Minimum mana %"),
    30,
    0,
    100);
```

Here:

- `"min_mana"` is the stable menu/config ID.
- `"mychamp.min_mana"` is the stable localization key.
- `"Minimum mana %"` is the fallback text.

## Recommended reading order

1. [First Plugin](GettingStarted/01_First_Plugin.md) — plugin metadata, lifecycle, headers and loading.
2. [Objects and Game](GettingStarted/02_Objects_And_Game.md) — `Object`, `ObjectManager`, positions, lists and game state.
3. [Events](GettingStarted/03_Events.md) — update/draw/spell/path events and RAII subscriptions.
4. [Menu](GettingStarted/04_Menu.md) — page/tab/section/group/item hierarchy, persistence, linked items and localization.
5. [Spell and Prediction](GettingStarted/05_Spell_And_Prediction.md) — skillshots, target selection, prediction, collision and charged spells.
6. [TargetSelector and Orbwalker](GettingStarted/06_TargetSelector_And_Orbwalker.md) — shared target selection and movement/attack ownership.
7. [HealthPrediction and Farming](GettingStarted/07_HealthPrediction_And_Farming.md) — predicted health and farm locations.
8. [Rendering](GettingStarted/08_Rendering.md) — world/screen coordinates and draw calls.
9. [Items, Buffs and Utility](GettingStarted/09_Items_Buffs_And_Utility.md) — common gameplay helpers.
10. [Localization](GettingStarted/10_Localization.md) — `Tr()`, locale catalogs and live UI translation.
11. [Champion Plugin: Xerath](GettingStarted/Champion_Plugin.md) — a small end-to-end champion example.

If you are porting an existing LeagueSharp champion, chapters 4–7 and the Xerath example are usually the fastest path.

## Complete example sources

```text
Examples/GettingStarted/
├─ 01_FirstPlugin.cpp
├─ 02_ObjectsAndGame.cpp
├─ 03_Events.cpp
├─ 04_Menu.cpp
├─ 05_SpellPrediction.cpp
├─ 06_TargetSelectorOrbwalker.cpp
├─ 07_HealthPredictionFarming.cpp
├─ 08_Rendering.cpp
├─ 09_ItemsBuffsUtility.cpp
├─ 10_Localization.cpp
└─ Xerath_Simple.cpp
```

The examples in this package were syntax-checked against the public headers shipped in `RSDK_Pro_v5.2.2`. A final VS2022 x64 build is still recommended before distributing a plugin.

## Where to look when something is unclear

Use this order:

1. Read the relevant Getting Started chapter for usage and common mistakes.
2. Open its `.cpp` example for a complete working shape.
3. Check `API_Reference.md` for the exact signature.
4. Check the public header itself if you need template/overload details.

Do not copy internal Core/provider implementation code into user plugins. Public headers are the supported contract.


## RSDK 5.2.2 quick start — new APIs

### Object: emotes, special states and shield

```cpp
const Object player = ObjectManager::LocalPlayer();
if (player)
{
    const bool teleporting = player.IsTeleport(); // renamed from IsRecalling()
    const bool zombie = player.IsZombie();
    const bool clone = player.IsClone();
    const float physicalShield = player.PhysicalShield();
    // player.DoEmote(Emotes::<emote>); // choose a valid Emotes enumerator
}
```

`DoEmote` is intended for the local player. Review the enum in the current Public headers.

### Flee and shared Spell Farming toggle

```cpp
if (Orbwalker::IsAvailable())
{
    if (Orbwalker::ActiveMode() == Orbwalker::Mode::Flee)
    {
        // Champion plugin decides which defensive/movement spells to cast.
    }
    if (Orbwalker::SpellFarmEnabled())
    {
        // Permit champion-defined spell farming in Mixed (C) / LaneClear (V).
    }
}
```

The default Flee hotkey is Z (Press). The default shared Spell Farming toggle is G.
Spell Farming is **not** a new OrbwalkingMode and does not disable ordinary auto-attacks.

### OnNewPath raw flags and blink classification

```cpp
void OnNewPath(const NewPathEvent& event)
{
    const std::uint32_t flags = event.dashGravityFlags;
    if (event.IsBlink())
    {
        // bit 0 is set (Flash, Ezreal E and other tested blinks).
    }
    (void)flags; // other bits remain available for future investigation.
}
```

The new event field and Object API additions require the matching SDK/Core **ABI 48**.
Orbwalker facade/provider API version is **6**. Rebuild plugins against the 5.2.2 Public headers.
