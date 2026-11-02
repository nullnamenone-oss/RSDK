# 01 — Creating Your First Plugin

This chapter covers the smallest useful RSDK plugin and explains the parts that are easy to misunderstand.

[Open full example: `01_FirstPlugin.cpp`](../Examples/GettingStarted/01_FirstPlugin.cpp)

## Metadata comes before `RSDK.h`

```cpp
#define Plugin_Id              "MyChampion.Plugin"
#define Plugin_Name            "My Champion"
#define Author_Name            "YourName"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Champion plugin"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
```

`Plugin_Id` is the important stable identifier. Treat it like a package name: do not change it simply because you renamed the visible title.

RSDK uses the persistent plugin ID to isolate configuration and localization. For example, a plugin with:

```cpp
#define Plugin_Id "MyCompany.Xerath"
```

uses language catalogs under:

```text
Languages/MyCompany.Xerath/<locale>.json
```

## Plugin lifecycle

A plugin derives from `rsdk::Plugin`:

```cpp
class MyPlugin final : public Plugin
{
public:
    bool OnLoad() override
    {
        return true;
    }

    void OnUnload() override
    {
    }
};

RSDK_REGISTER_PLUGIN(MyPlugin);
```

Use `OnLoad()` for normal setup:

- configure `Spell` objects;
- create menu pages;
- subscribe to events;
- initialize plugin-owned state.

Return `false` when the plugin cannot start safely. A champion plugin can use this to reject the wrong champion:

```cpp
const Object player = ObjectManager::LocalPlayer();
if (!player || player.ChampionName() != "Xerath")
    return false;
```

Use `OnUnload()` only for cleanup you own. Framework-tracked event subscriptions and Delay tasks are released when the plugin unloads.

## Where should gameplay logic run?

For normal champion logic, start with a named `OnGameUpdate` handler:

```cpp
void OnGameUpdate()
{
    const Object player = ObjectManager::LocalPlayer();
    if (!player || player.IsDead())
        return;

    // Combo/Harass/LaneClear decisions.
}

// Register once from OnLoad().
Events::OnGameUpdate += OnGameUpdate;
```

This style keeps the registration line easy to scan and the handler easy to find while debugging. An inline lambda is also valid for a tiny one-off handler, but the guide uses named handlers by default. Keep `OnDraw` for rendering rather than gameplay decisions.

## Public headers

`RSDK.h` exposes the main runtime layer and public provider facades. Add Common helpers when you use them:

```cpp
#include <Common/Spell.h>
#include <Common/TargetSelector.h>
#include <Common/Items.h>
#include <Common/Utility.h>
```

Do not include files under `Source/` from a user plugin.

## Plugin placement

Champion plugins normally go under:

```text
Plugins/Champion/Xerath/MyXerath.dll
```

General gameplay or utility plugins go under `Plugins/Gameplay/` or `Plugins/Utility/`. `Plugins/Misc/` is also discovered (including in 5.2.2).

The folder does not force your menu category. Menu placement is chosen explicitly with `Menu::CreatePage(..., MenuCategory::...)`.

## Common mistakes

**Changing `Plugin_Id` during every rename.** That gives the plugin a new persistent identity and can separate it from its existing config/language domain.

**Doing substantial work in `DllMain`.** Use the RSDK plugin lifecycle instead.

**Depending on private implementation headers.** Plugin code should compile against `Public/` only.

**Assuming every provider is present.** If Prediction/TargetSelector/Orbwalker are optional for your plugin, call their `IsAvailable()` helper before relying on them.

## Next

Continue with [Objects and Game](02_Objects_And_Game.md).
