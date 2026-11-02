# 04 — Menu

The menu API is hierarchical and plugin-owned. This chapter uses localization from the first example so copied code is already suitable for language catalogs.

[Open full example: `04_Menu.cpp`](../Examples/GettingStarted/04_Menu.cpp)

[Open sample language catalogs for `Guide.Menu`](../Examples/GettingStarted/Languages/Guide.Menu/)

## Hierarchy

```text
MenuCategory
└─ MenuPage
   └─ MenuTab
      ├─ Item
      └─ MenuSection        // optional
         ├─ Item
         └─ MenuGroup       // optional/collapsible
            ├─ Item
            └─ MenuGroup    // nested groups, max depth 3
```

A `MenuSection` is optional in 5.x. For a small tab, add controls directly to `MenuTab`. For a large tab, use sections/groups to keep it readable.

Current public menu categories include:

```cpp
MenuCategory::Champion
MenuCategory::Gameplay
MenuCategory::Utility
MenuCategory::Core
MenuCategory::Debug
MenuCategory::Misc
```

`MenuCategory::Combat` is a legacy alias of `Gameplay`.

> Use `MenuCategory::Misc` when registering a page for a plugin under `Plugins\Misc\`. Plugin folders control discovery; menu category is chosen explicitly when creating a page.

## Create a page and tab

```cpp
MenuPage page = Menu::CreatePage(
    "xerath",
    Tr("xerath.title", "Xerath"),
    MenuCategory::Champion);

if (!page)
    return;

MenuTab combo = page.AddTab(
    "combo",
    Tr("xerath.tab.combo", "Combo"));
```

There are three different identities in this code:

- `"xerath"` / `"combo"`: stable menu IDs used for identity/persistence.
- `"xerath.title"` / `"xerath.tab.combo"`: stable localization keys.
- `"Xerath"` / `"Combo"`: English fallback text.

Do not translate the stable IDs themselves.

## Direct controls on a tab

```cpp
MenuBool useQ = combo.AddBool(
    "use_q",
    Tr("xerath.combo.use_q", "Use Q"),
    true);

MenuSliderInt minMana = combo.AddSliderInt(
    "min_mana",
    Tr("xerath.combo.min_mana", "Minimum mana %"),
    30,
    0,
    100);
```

This is the simplest recommended form for a small tab.

## Sections and groups

```cpp
MenuSection behavior = combo.AddSection(
    "behavior",
    Tr("xerath.section.behavior", "Behavior"));

MenuGroup q = behavior.AddGroup(
    "q",
    Tr("xerath.group.q", "Q Settings"));
```

Groups can be nested:

```cpp
MenuGroup advanced = q.AddGroup(
    "advanced",
    Tr("xerath.group.q.advanced", "Advanced"),
    false); // closed by default
```

The public menu limits nested group depth to three. If the layout requires deeper nesting, the menu is probably better split into another section or tab.

## Item types

The same item types are available from `MenuTab`, `MenuSection`, and `MenuGroup`:

```cpp
MenuBool enabled = q.AddBool(...);
MenuSliderInt count = q.AddSliderInt(...);
MenuSliderFloat scale = q.AddSliderFloat(...);
MenuCombo mode = q.AddCombo(...);
MenuKeyBind key = q.AddKeyBind(...);
MenuColor color = q.AddColor(...);
MenuButton button = q.AddButton(...);
q.AddText(...);
q.AddSeparator();
```

### Combo options can be localized too

```cpp
MenuCombo mode = q.AddCombo(
    "mode",
    Tr("xerath.mode", "Mode"),
    {
        Tr("xerath.mode.safe", "Safe"),
        Tr("xerath.mode.normal", "Normal"),
        Tr("xerath.mode.aggressive", "Aggressive")
    },
    1);
```

RSDK resolves the visible option text when the menu is rendered, so localized combo entries can follow a runtime language change just like labels.

## Reading and changing values

```cpp
if (enabled.Value())
{
    const int mana = minMana.Value();
    const float value = scale.Value();
    const int selectedIndex = mode.Value();
}
```

Change values with `SetValue()`:

```cpp
enabled.SetValue(false);
minMana.SetValue(50);
```

`MenuCombo::Value()` is the selected option index.

## Keybinds

```cpp
MenuKeyBind comboKey = q.AddKeyBind(
    "combo_key",
    Tr("xerath.combo_key", "Combo key"),
    0x20, // VK_SPACE
    KeyBindType::Press);
```

Use `Active()` to ask whether the bind is active:

```cpp
if (comboKey.Active())
{
    // action while key is active
}
```

`KeyBindType::Press` follows press state. `KeyBindType::Toggle` changes persistent on/off state when the key is activated.

The bound virtual key is available through `Key()` and can be changed with `SetKey()`.

## Buttons

Buttons are edge-triggered and use `ConsumePressed()`:

```cpp
MenuButton reset = q.AddButton(
    "reset",
    Tr("xerath.reset", "Reset settings"));

if (reset.ConsumePressed())
{
    enabled.SetValue(true);
    minMana.SetValue(30);
}
```

Do not treat a button like a bool setting; it does not have a persistent `Value()` state.

## Tooltips, persistence and same-line layout

```cpp
enabled.SetTooltip(Tr(
    "xerath.tooltip.enabled",
    "Enables this feature."));
```

Menu items are persistent by default. For transient/debug state:

```cpp
debugToggle.SetPersistent(false);
```

Compact controls can request same-line layout:

```cpp
color.SetSameLine(true, 8.0f);
```

These are item properties; they do not change the stable item ID.

## Linked items

`MenuLinks` is a convenience collection when ID-based lookup is useful:

```cpp
MenuLinks links;

MenuBool useQ = q.AddLinkedBool(
    links.Bools,
    "use_q",
    Tr("xerath.use_q", "Use Q"),
    true);
```

Later:

```cpp
if (links.Bools["use_q"].Value())
{
    // ...
}
```

Linked variants exist for bool, combo, int/float sliders, keybind and color. Keeping normal member handles such as `MenuBool useQ_` is equally valid; use whichever keeps your plugin clearer.

## Localization catalog

For:

```cpp
#define Plugin_Id "Guide.Menu"
```

the catalog is:

```text
Languages/Guide.Menu/en-US.json
Languages/Guide.Menu/vi-VN.json
Languages/Guide.Menu/zh-CN.json
```

Catalogs are flat JSON objects:

```json
{
  "guide.menu.title": "Menu Guide",
  "guide.menu.enabled": "Enabled"
}
```

For more details, see [Localization](10_Localization.md).

## Common mistakes

**Hard-coding visible labels everywhere.** Plain strings work, but production examples should use `Tr()` so the menu participates in runtime language switching.

**Changing IDs when translating a label.** Translate the `Tr()` value, not the item ID.

**Using too many nested groups.** Prefer another section/tab instead of forcing deep hierarchy.

**Polling buttons with a nonexistent value.** Use `ConsumePressed()`.

**Assuming filesystem category equals menu category.** They are separate systems.

## Next

Continue with [Spell and Prediction](05_Spell_And_Prediction.md).
