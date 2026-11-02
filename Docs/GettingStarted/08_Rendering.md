# 08 — Rendering

Render from `Events::OnDraw`, check `Render::IsReady()`, and keep gameplay decisions out of the draw path.

[Open full example: `08_Rendering.cpp`](../Examples/GettingStarted/08_Rendering.cpp)

[Open sample language catalogs for `Guide.Rendering`](../Examples/GettingStarted/Languages/Guide.Rendering/)

## Draw a world-space circle

```cpp
void OnDraw()
{
    if (!Render::IsReady())
        return;

    const Object player = ObjectManager::LocalPlayer();
    if (!player)
        return;

    Render::Circle3D(
        player.Position(),
        650.0f,
        Color(80, 190, 255, 220),
        2.0f,
        96u,
        true);
}

Events::OnDraw += OnDraw;
```

The final argument enables terrain clipping for this overload. Choose it based on the visual you want; it is not a gameplay collision test.

## World to screen

```cpp
const Vec2 screen = Game::WorldToScreen(player.Position());

if (screen.IsValid())
{
    Render::Text(
        screen + Vec2(20.0f, -60.0f),
        Localization::Text("guide.rendering.label", "Rendering example"),
        Color(255, 255, 255),
        18.0f);
}
```

Use `WorldToMinimap()` for minimap coordinates.

## Common render functions

The public facade includes helpers such as:

```text
Text
Line2D / Circle2D
Rect / FilledRect
Line3D / Circle3D
CircleGlow3D
RainbowCircle3D
Image
```

Textures can be loaded through public `Texture::LoadFromFile()` or `Texture::LoadFromMemory()` helpers.

## Colors

`Color` is the public color type used by menu and rendering. Helpers such as HSV conversion, random color, and rainbow color are available in the public color API.

## What not to do in `OnDraw`

Avoid running TargetSelector, large object scans, farming decisions, or prediction loops solely because Draw runs often. Make the decision in gameplay/update events and draw the resulting state.

## Next

Continue with [Items, Buffs and Utility](09_Items_Buffs_And_Utility.md).
