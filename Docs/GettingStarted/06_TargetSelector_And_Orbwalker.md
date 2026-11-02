# 06 — TargetSelector and Orbwalker

TargetSelector chooses combat targets consistently across plugins. Orbwalker owns shared attack/movement timing and orbwalking modes. Champion code should cooperate with these providers rather than implementing competing timing loops.

[Open full example: `06_TargetSelectorOrbwalker.cpp`](../Examples/GettingStarted/06_TargetSelectorOrbwalker.cpp)

## TargetSelector basics

```cpp
if (!TargetSelector::IsAvailable())
    return;

const Object target = TargetSelector::GetTarget(
    1200.0f,
    DamageType::Magical);
```

`Spell::GetTarget()` is normally more convenient when you already have a configured spell.

## Custom filter

The current C++ public filter is a normal function pointer:

```cpp
using TargetFilter = bool(*)(const Object& target);
```

So this works:

```cpp
bool LowHealthTarget(const Object& target)
{
    return target.HealthPercent() < 60.0f;
}

const Object target = TargetSelector::GetTarget(
    1200.0f,
    DamageType::Magical,
    true,
    nullptr,
    LowHealthTarget);
```

A capturing lambda cannot be passed as `TargetFilter` in this SDK. If you need captured state, filter the resulting/candidate logic in your own code or use stable external/plugin state through a normal function.

## Ignored champions and no-collision helper

TargetSelector exposes overloads that accept a vector of ignored champions. `TargetSelector::GetTargetNoCollision(spell, ...)` is also available when you specifically want a target whose current spell prediction is not collision-blocked.

## Orbwalker active mode

```cpp
switch (Orbwalker::ActiveMode())
{
case Orbwalker::Mode::Combo:
    Combo();
    break;
case Orbwalker::Mode::Mixed:
    Harass();
    break;
case Orbwalker::Mode::LaneClear:
    LaneClear();
    break;
case Orbwalker::Mode::LastHit:
    LastHit();
    break;
default:
    break;
}
```

Other public modes include `Freeze`, `CustomMode`, and `None`.

## Forced target

```cpp
if (target)
    Orbwalker::ForceTarget(target);
else
    Orbwalker::ClearForcedTarget();
```

Use forced targets deliberately. Do not leave a stale forced target active after the feature no longer needs it.

## Attack and movement state

Useful reads:

```cpp
Orbwalker::CanAttack();
Orbwalker::CanMove();
Orbwalker::InAutoAttackRange(target);
Orbwalker::AttackEnabled();
Orbwalker::MovementEnabled();
```

Control switches:

```cpp
Orbwalker::SetAttack(false);
Orbwalker::SetMovement(false);
```

Restore them when your feature finishes. Do not permanently disable shared orbwalker behavior from a short-lived action.

## Movement

Use the provider movement path when your logic wants orbwalker-compatible movement:

```cpp
Orbwalker::MoveTo(Game::CursorPosition());
```

`MoveTo()` shares Orbwalker movement throttling/state. Calling raw move orders from a separate loop can fight the orbwalker.

`Orbwalker::ShouldWait()` is useful around farming logic when the shared orbwalker has a reason to wait rather than attack another target.

## Orbwalker attack events

```cpp
Orbwalker::BeforeAttack += [](BeforeAttackEvent& event)
{
    if (!event.Target)
        event.Process = false;
};
```

`BeforeAttack` can cancel an attack with `event.Process = false`.

Other public events include:

```text
OnAttack
AfterAttack
OnNonKillableMinion
OnTargetChange
```

Use attack events for logic that is genuinely tied to the orbwalker's attack lifecycle instead of approximating it with arbitrary timers.

## Timing values

Orbwalker gameplay timestamps are seconds. `SetMovementDelay()` also takes seconds even though the menu displays movement delay in milliseconds.

## Next

Continue with [HealthPrediction and Farming](07_HealthPrediction_And_Farming.md).

## RSDK 5.2.2: Flee mode and global Spell Farming toggle

`Orbwalker::Mode::Flee` is mode 7 (`RSDK_ORBWALKER_MODE_FLEE`). Orbwalker only moves toward the cursor in Flee; champion logic handles dash, movement-buff or CC spells itself. The default Flee key is Z (Press).

`Orbwalker::SpellFarmEnabled()` reads the shared Core-backed G (Toggle) value.
It only gates **champion-written spell farming**, not automatic attacks. Mixed (C) and LaneClear (V) remain their original modes.

```cpp
const auto mode = Orbwalker::ActiveMode();
if (mode == Orbwalker::Mode::Flee)
    FleeLogic(); // Champion-defined

if (Orbwalker::SpellFarmEnabled() && mode == Orbwalker::Mode::LaneClear)
    LaneClearSpellLogic(); // Champion-defined
```
