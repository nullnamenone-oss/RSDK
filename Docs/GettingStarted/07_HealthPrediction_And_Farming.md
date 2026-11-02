# 07 — HealthPrediction and Farming

HealthPrediction estimates future minion health from tracked attacks. MinionManager/Spell farming helpers find cast positions that hit useful groups of minions.

[Open full example: `07_HealthPredictionFarming.cpp`](../Examples/GettingStarted/07_HealthPredictionFarming.cpp)

## Provider availability

HealthPrediction is exposed publicly but backed by the Orbwalker runtime provider:

```cpp
if (!HealthPrediction::IsAvailable())
    return;
```

## Predict health at a future time

```cpp
const float hp = HealthPrediction::GetHealthPrediction(
    minion,
    0.35f);
```

The `time` argument is seconds into the future. The optional delay parameter also uses seconds:

```cpp
const float hp = HealthPrediction::GetHealthPrediction(
    minion,
    0.35f,
    0.070f);
```

Do not pass milliseconds such as `350` for a 350 ms horizon.

## Lane-clear prediction

```cpp
const float hp = HealthPrediction::LaneClearHealthPrediction(
    minion,
    1.0f);
```

Use the lane-clear form when you need the farming model that can project repeated lane attack cycles over the requested horizon. Use normal `GetHealthPrediction()` for a direct future-health query.

Aggro helpers include:

```cpp
HealthPrediction::HasMinionAggro(minion);
HealthPrediction::HasTurretAggro(minion);
HealthPrediction::TurretAggroStartTime(minion);
```

## Circular farming

With a configured circular spell:

```cpp
std::vector<Object> minions;
for (const Object& minion : ObjectManager::AllMinions(TeamFilter::Enemy))
{
    if (Utility::IsValidTarget(minion, w.Range()))
        minions.push_back(minion);
}

const MinionManager::FarmLocation farm = w.GetCircularFarmLocation(minions);

if (farm.MinionsHit >= 3)
    (void)w.Cast(farm.Position);
```

`FarmLocation` contains:

```cpp
farm.Position;
farm.MinionsHit;
```

For line spells, use `Spell::GetLineFarmLocation()`.

## Direct MinionManager helpers

When you already own the predicted positions or do not need a Spell wrapper, MinionManager exposes helpers such as:

```cpp
MinionManager::GetBestCircularFarmLocation(...);
MinionManager::GetBestLineFarmLocation(...);
MinionManager::GetMinionsPredictedPositions(...);
```

Classification helpers include `MinionManager::IsWard()` and `MinionManager::IsMinion()`.

## Damage provider in RSDK 5.2.2

`Damage.dll` is shipped as a gameplay module in 5.2.2. The damage dataset shipped with this source is versioned separately, and game updates can change its correctness.
Check `Damage::IsAvailable()` before using provider-backed damage APIs.

`Utility::CalculateDamage()` is only a basic mitigation helper, not a full replacement for champion passives, on-hit effects, spell stages or the Damage provider.

## Next

Continue with [Rendering](08_Rendering.md).
