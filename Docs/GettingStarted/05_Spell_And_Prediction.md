# 05 — Spell and Prediction

`Spell` is the Common helper used by most champion scripts. It combines spell configuration, range checks, TargetSelector, Prediction, collision, charged spells and several farming helpers without exposing provider internals.

[Open full example: `05_SpellPrediction.cpp`](../Examples/GettingStarted/05_SpellPrediction.cpp)

## Direct cast or `Spell`?

The local player can cast directly when your logic already knows exactly what to cast:

```cpp
const Object player = ObjectManager::LocalPlayer();
player.Cast(SpellSlot::_Q);
player.Cast(SpellSlot::_W, target);
player.Cast(SpellSlot::_E, position);
```

Use `Spell` when you want reusable range/prediction/collision behavior.

## Create and configure a skillshot

```cpp
Spell q{SpellSlot::_Q, 1200.0f, DamageType::Magical};

q.SetSkillshot(
     0.25f,              // delay, seconds
     70.0f,              // width/radius
     1700.0f,            // missile speed
     true,               // collision enabled
     SkillshotType::Line)
 .SetCollisionObjects(
     CollisionableObjects::Minions |
     CollisionableObjects::YasuoWall)
 .SetMinHitChance(HitChance::High);
```

The `SetSkillshot()` delay uses seconds. Use the spell's actual current-game data; old LeagueSharp values are only porting references.

For a circle:

```cpp
w.SetSkillshot(
    0.50f,
    250.0f,
    std::numeric_limits<float>::max(),
    false,
    SkillshotType::Circle);
```

For `SkillshotType::Cone`, the width parameter is the cone angle in radians.

## Target selection through Spell

```cpp
const Object target = q.GetTarget();
if (!target)
    return;
```

`Spell::GetTarget()` delegates to the public TargetSelector provider using the spell range and damage type. Therefore TargetSelector must be available for this workflow.

You can temporarily extend the search range:

```cpp
const Object target = q.GetTarget(150.0f);
```

## Inspect prediction

```cpp
const PredictionOutput prediction = q.GetPrediction(target);

if (prediction.Hitchance < HitChance::High)
    return;
```

Useful output fields include:

```cpp
prediction.CastPosition;
prediction.UnitPosition;
prediction.Hitchance;
prediction.CollisionObjects;
prediction.AoeTargetsHit;
prediction.AoeTargetsHitCount;
```

For a line skillshot with collision enabled, a normal explicit flow is:

```cpp
if (!prediction.CollisionObjects.empty())
    return;

(void)q.Cast(prediction.CastPosition);
```

## Let Spell perform prediction for you

Often you do not need to inspect every field yourself:

```cpp
q.SetMinHitChance(HitChance::High);

const Object target = q.GetTarget();
if (target)
    q.Cast(target);
```

Or require a hitchance for one cast:

```cpp
if (target)
    q.CastIfHitchanceMinimum(target, HitChance::VeryHigh);
```

Use the explicit `GetPrediction()` path when you need custom decisions based on collision, predicted unit position, AOE results, or other output.

## Generic collision queries

`Spell::GetCollision()` accepts arbitrary endpoint positions:

```cpp
std::vector<Object> blockers = q.GetCollision(
    q.From(),
    prediction.CastPosition);
```

When you already have your own `PredictionInput`, use the provider directly:

```cpp
PredictionInput input{};
input.Unit = target;
input.From = player.ServerPos();
input.Delay = 0.25f;
input.Radius = 70.0f;
input.Range = 1200.0f;
input.Speed = 1700.0f;
input.Type = SkillshotType::Line;
input.CollisionObjects = CollisionableObjects::Minions;

const auto blockers = Prediction::GetCollision(target.Position(), input);
```

## Hitbox range behavior

For spells whose actual range includes caster or target hitbox, `Spell` exposes:

```cpp
q.SetAddSelfHitboxToRange(true);
q.SetAddEnemyHitboxToRange(true);
```

Do not enable these only because they make a range test easier. They should match the spell's real game behavior.

## Charged spells

Configure once:

```cpp
q.SetCharged(
    "ChargeSpellName",
    "ChargeBuffName",
    750.0f,
    1550.0f,
    1.50f);
```

Runtime flow:

```cpp
if (!q.IsCharging())
    q.StartCharging();
else
    q.ReleaseCharge(castPosition);
```

Default charged release mode is `ChargedReleaseMode::UpdateOnly`. `UpdateThenCast` is an opt-in compatibility mode for spells that have been verified to require the additional cast. Do not enable it globally.

A practical rule: once a charged spell is already charging, let its charge/release flow finish before running unrelated casts that could interfere with it.

## Channel protection

`Spell::SetChannelProtection()` exists for known channels that can be accidentally cancelled by scripted movement, attacks, or other casts. It is not something to enable for every spell.

Use it only after the spell has been verified as one of the supported/proven channel cases.

## Provider availability

Prediction-dependent calls require the Prediction provider. If it is optional for your plugin:

```cpp
if (!Prediction::IsAvailable())
    return;
```

## Next

Continue with [TargetSelector and Orbwalker](06_TargetSelector_And_Orbwalker.md).

## RSDK 5.2.2: dynamic spell parameters and cast request throttle

`Spell` can update `SetRange()`, `SetWidth()`, `SetDelay()`, `SetSpeed()` without rebuilding the object.
`SetCastThrottle(seconds)` sets the default per-Spell throttle; an overload with a `float` parameter can override it per request. Throttle 0 disables the per-instance interval, so multi-stage recasts must be guarded by champion logic.

```cpp
q.SetRange(1000.0f).SetWidth(80.0f).SetDelay(0.25f).SetSpeed(1700.0f);
q.SetCastThrottle(0.25f);
(void)q.Cast(position);         // default throttle
(void)q.Cast(position, 0.05f);  // per-call override
```

Prediction's DashTracker now uses `NewPathEvent::IsBlink()` (bit 0 of `dashGravityFlags`) for blink classification.
