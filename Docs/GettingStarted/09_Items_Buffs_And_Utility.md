# 09 — Items, Buffs and Utility

This chapter groups small public helpers that commonly appear in champion code.

[Open full example: `09_ItemsBuffsUtility.cpp`](../Examples/GettingStarted/09_ItemsBuffsUtility.cpp)

## Buff checks

```cpp
if (player.HasBuff(BuffType::Stun))
{
    // player has a stun-type buff
}
```

Name-based lookup is also available:

```cpp
if (target.HasBuff("SomeBuffName"))
{
    const Buff buff = target.GetFirstBuff("SomeBuffName");
    if (buff)
    {
        // use buff data
    }
}
```

Prefer type checks when the logic is about a buff category, and exact names when the champion/game mechanic requires a specific buff.

## Items

Find/check items through `Common/Items.h`:

```cpp
if (Items::HasItem(itemId) && Items::CanUseItem(itemId))
{
    (void)Items::UseItem(itemId, target);
}
```

There are overloads for owner/item queries and different cast forms. Use the overload matching the item behavior rather than guessing a spell slot yourself.

## Target validation

```cpp
if (Utility::IsValidTarget(target, 800.0f))
{
    // valid target in range
}
```

This is a common shared check for hand-written champion logic. TargetSelector and Spell already perform their own target-selection validation, so do not duplicate it everywhere.

## Raw mitigation helper

```cpp
const float afterResist = Utility::CalculateDamage(
    player,
    target,
    DamageType::Magical,
    100.0f);
```

This is a basic damage-mitigation helper. It does not model the future full Damage provider's champion passives, item/on-hit rules, staged spells, or champion-specific formulas.

## Paths

Common path helpers:

```cpp
const float length = Utility::PathLength(path);
const std::vector<Vec3> cut = Utility::CutPath(path, 250.0f);
```

The public API provides both `Vec2` and `Vec3` CutPath overloads in the current public API.

## Generic object names

When an event gives you an arbitrary game object, use `ObjectName()`/`Name()` unless you have already established that it is a hero/minion/turret with the richer layout. This matters for particles and `.troy` objects.

## Next

Continue with [Localization](10_Localization.md).

## RSDK 5.2.2: Utility facing and path-cutting helpers

`Utility::IsFacing(source, target)` and `Utility::IsBothFacing(source, target)` use the unit's facing vector and its target direction (LeagueSharp.SDK-style check).
`Utility::CutPath()` has both `std::vector<Vec2>` and `std::vector<Vec3>` overloads, suitable for Prediction and Orbwalker path helpers.
