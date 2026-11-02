# 10 — Localization

RSDK 5.2.2 supports plugin-local language catalogs. The recommended rule is simple: stable IDs stay untranslated; user-visible text should normally be localized.

[Open full example: `10_Localization.cpp`](../Examples/GettingStarted/10_Localization.cpp)

[Open sample language catalogs for `Guide.Localization`](../Examples/GettingStarted/Languages/Guide.Localization/)

## Plugin domain

With:

```cpp
#define Plugin_Id "Guide.Localization"
```

RSDK looks in:

```text
Languages/Guide.Localization/en-US.json
Languages/Guide.Localization/vi-VN.json
Languages/Guide.Localization/zh-CN.json
```

Catalogs are flat JSON maps:

```json
{
  "guide.localization.title": "Localization",
  "guide.localization.ready": "Ready"
}
```

## `Tr()` for menu/UI descriptors

Use `rsdk::Tr()` for menu labels and other API calls that keep the text descriptor:

```cpp
MenuPage page = Menu::CreatePage(
    "localization",
    Tr("guide.localization.title", "Localization"),
    MenuCategory::Utility);
```

`Tr()` is especially useful for menu controls because the menu resolves the descriptor while rendering. A language change can therefore update labels without rebuilding the menu tree.

The same applies to combo options:

```cpp
MenuCombo mode = tab.AddCombo(
    "mode",
    Tr("guide.mode", "Mode"),
    {
        Tr("guide.mode.safe", "Safe"),
        Tr("guide.mode.fast", "Fast")
    },
    0);
```

## Immediate translated strings

When you need a `std::string` now:

```cpp
const std::string text = Localization::Tr(
    "guide.localization.ready",
    "Ready");
```

When an API expects an immediate `const char*`:

```cpp
Render::Text(
    screen,
    Localization::Text("guide.localization.ready", "Ready"),
    Color(255, 255, 255),
    18.0f);
```

## ID vs localization key vs fallback

Example:

```cpp
tab.AddBool(
    "show_message",
    Tr("guide.localization.show_message", "Show message"),
    true);
```

- `show_message` — menu/config identity; keep stable.
- `guide.localization.show_message` — translation lookup key; keep stable.
- `Show message` — fallback English shown when a translation is unavailable.

Do not translate `show_message`. Do not use the visible English label as your persistence identity.

## Locale control

The public localization facade also exposes helpers for reading/setting the locale and reloading catalogs. Normally a plugin should respect the Core-selected language instead of overriding it for every user.

## Common mistakes

**Using `Localization::Tr()` where a lazy `Tr()` descriptor is preferable.** Immediate strings are useful outside menu storage, but menu labels should normally use `rsdk::Tr()`.

**Only translating the label of a Combo.** Its option strings are visible UI too; localize them as well.

**Changing keys during wording edits.** Keep keys stable and change the translated/fallback value.

## Next

Continue with the end-to-end [Champion Plugin: Xerath](Champion_Plugin.md).
