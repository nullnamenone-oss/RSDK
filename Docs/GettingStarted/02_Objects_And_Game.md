# 02 — Objects and Game

`Object` is the normal public wrapper for game objects. `ObjectManager` gives you the objects currently known to the game, while `Game` exposes global game state and coordinate helpers.

[Open full example: `02_ObjectsAndGame.cpp`](../Examples/GettingStarted/02_ObjectsAndGame.cpp)

## Local player

```cpp
const Object player = ObjectManager::LocalPlayer();
if (!player || player.IsDead())
    return;
```

The wrapper can be tested directly in a boolean context. Do this before reading gameplay properties when an object may be absent.

Common reads include:

```cpp
player.NetworkId();
player.Health();
player.MaxHealth();
player.ManaPercent();
player.MoveSpeed();
player.AttackRange();
player.BoundingRadius();
player.Position();
player.ServerPos();
```

## Teleport state and physical shields (5.2.2_fix)

```cpp
const Object player = ObjectManager::LocalPlayer();
if (player)
{
    const bool teleporting = player.IsTeleport();
    const float physicalShield = player.PhysicalShield();
    const char* const state = player.TeleportState();
    if (state != nullptr)
    {
        // Borrowed native text: copy immediately if you need to keep it.
        const std::string snapshot(state);
    }
}
```

`TeleportState()` returns `nullptr` when no state is available. Do not retain the borrowed pointer across updates.


## Object lists

```cpp
std::vector<Object> enemies = ObjectManager::AllHeroes(TeamFilter::Enemy);
std::vector<Object> minions = ObjectManager::AllMinions(TeamFilter::Enemy);
std::vector<Object> turrets = ObjectManager::AllTurrets(TeamFilter::Enemy);
std::vector<Object> attackables = ObjectManager::AttackableUnits(TeamFilter::Enemy);
```

`AttackableUnits()` is useful when you need the broader set of attackable game objects rather than only heroes/minions/turrets. Missiles are not part of that list.

Use `AllObjects()` only when you truly need the generic object list.

## `Position()` versus `ServerPos()`

`Position()` is the general object position. `ServerPos()` exists for gameplay logic where a source port used LeagueSharp `ServerPosition` semantics.

A safe pattern when server position can be invalid is:

```cpp
const Vec3 position = player.Position();
const Vec3 serverPosition = player.ServerPos().IsValid() ? player.ServerPos() : position;
```

Do not mechanically replace every visual position with `ServerPos()`. Rendering often wants the normal current position, while prediction/range logic may prefer server position.

## Valid targets

For normal combat checks, use the shared helper instead of repeating visibility/team/range tests:

```cpp
if (!Utility::IsValidTarget(enemy, 1200.0f))
    continue;
```

When a subsystem such as TargetSelector or Spell already validates candidates, do not duplicate every check again unless your champion has an additional rule.

## Object names — important safety rule

For arbitrary generic objects, particles, `.troy` objects, or other short-layout objects, prefer:

```cpp
object.ObjectName();
object.Name();
```

Only use richer identity fields such as `ChampionName()` or `BaseSkinName()` when the object type supports them. Do not call hero/minion-specific name fields on arbitrary particles just because the wrapper type is `Object`.

## Finding an object later

For long-lived state, storing a `NetworkId` is often safer than assuming the same wrapper remains useful forever:

```cpp
const std::uint32_t networkId = target.NetworkId();

// Later:
const Object currentTarget = ObjectManager::FindObjectByNetworkId(networkId);
```

Missiles have corresponding `FindMissileByNetworkId()` / `FindMissileById()` helpers.

## Game state and coordinates

```cpp
if (!Game::IsReady())
    return;

const float now = Game::TimeGame();     // seconds
const float ping = Game::Latency();     // milliseconds
const Vec3 cursor = Game::CursorPosition();
```

Coordinate conversion:

```cpp
const Vec2 screen = Game::WorldToScreen(player.Position());
const Vec2 minimap = Game::WorldToMinimap(player.Position());
```

Terrain helpers include `Game::CollisionFlag()`, `Game::IsWall()`, `Game::IsGrass()` and `Game::IsInFow()`.

## Paths

Object path APIs include:

```cpp
std::vector<Vec3> path = player.GetPath(destination);
std::vector<Vec3> current = player.CurrentPath();
```

Use `Utility::PathLength()` and `Utility::CutPath()` when porting path-based logic.

## Next

Continue with [Events](03_Events.md).

## RSDK 5.2.2: Object state and teleport

`Object::IsRecalling()` has been renamed to `Object::IsTeleport()` across Core and Public.
Rebuild the plugin against ABI 48 and use these Object methods:

```cpp
if (player)
{
    const bool teleport = player.IsTeleport();
    const bool zombie = player.IsZombie();
    const bool clone = player.IsClone();
    const float shield = player.PhysicalShield();
    // DoEmote(Emotes) is local-player only.
}
```
