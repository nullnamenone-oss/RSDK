# 03 — Events

RSDK is designed around events. Subscribe only to the events a feature needs instead of polling everything on every frame.

[Open full example: `03_Events.cpp`](../Examples/GettingStarted/03_Events.cpp)

## Recommended registration style

For normal lifetime handlers, prefer **named functions**:

```cpp
void OnGameUpdate()
{
    // Champion decisions, trackers, timers.
}

void OnNewPath(const NewPathEvent& event)
{
    if (!event.sender)
        return;

    // Inspect event.path / event.isDash / event.dashSpeed.
}

void RegisterEvents()
{
    Events::OnGameUpdate += OnGameUpdate;
    Events::OnNewPath += OnNewPath;
}
```

This is the style used by the examples because the registration table stays short and each handler is easy to find, debug, and compare with its event signature.

Inline lambdas are also supported. They are useful for very small one-off handlers, but are usually less readable once the body grows:

```cpp
// Also valid:
// Events::OnNewPath += [](const NewPathEvent& event)
// {
//     // ...
// };
```

For a **class member function**, use the object/member overload where the event exposes it:

```cpp
class MyFeature
{
public:
    void Register()
    {
        Events::OnGameUpdate.Subscribe(this, &MyFeature::OnGameUpdate);
    }

private:
    void OnGameUpdate()
    {
        // ...
    }
};
```

That avoids writing a capture lambda only to forward into the member function.

## The common event split

Use `OnGameUpdate` for most gameplay logic:

```cpp
void OnGameUpdate()
{
    const Object player = ObjectManager::LocalPlayer();
    if (!player || player.IsDead())
        return;

    // Combo / Harass / trackers / timers.
}
```

Use `OnDraw` for rendering:

```cpp
void OnDraw()
{
    if (!Render::IsReady())
        return;

    // Draw only.
}
```

`OnTick`, `OnUpdate`, and `OnGameUpdate` are separate event sources. Do not assume they are interchangeable simply because all of them repeat.

## Lifetime subscriptions

For a handler that should remain registered for the entire plugin lifetime, `operator+=` is the simplest form:

```cpp
void OnProcessSpell(const ProcessSpellEvent& event)
{
    if (!event.sender || !event.spellCast)
        return;

    if (event.sender.IsMe())
    {
        Log::Info("Cast: %s", event.spellCast.SpellName().c_str());
        Log::Info("Slot=%u missileNet=%u", event.spellCast.Slot(), event.spellCast.MissileNetworkId());
    }
}

// OnLoad():
Events::OnProcessSpell += OnProcessSpell;
```

Framework-tracked subscriptions are released when the plugin unloads.

## RAII subscription with `EventConnection`

When an object or temporary feature should own its subscription and release it earlier than plugin unload, use `Connect()`:

```cpp
void OnGameUpdate()
{
    // temporary feature
}

EventConnection connection = Events::OnGameUpdate.Connect(OnGameUpdate);

// Optional early disconnect:
connection.Disconnect();
```

`EventConnection` is move-only and disconnects when its lifetime ends. It is useful for temporary trackers, charged-spell helpers, modes, or other object-scoped subscriptions.

Do not create an `EventConnection` as a short-lived local if you expect the subscription to survive after the local variable goes out of scope.

## Object and spell events

Common public events include:

```text
OnCreateObject / OnDeleteObject
OnNewPath
OnIssueOrder
OnProcessSpell / OnDoCast / OnStopCast
OnCastSpell / OnUpdateChargeableSpell
OnPlayAnimation
OnBuffAddRemove / OnUpdateBuff
OnSpellImpact
```

`OnProcessSpell` is useful when you need information from the processed cast. `OnDoCast` represents a later cast phase. `OnStopCast` is useful for explicit cancellation/interruption cases. Do not assume every spell lifecycle event fires at the same moment or for the same reason.

## `OnNewPath`

```cpp
void OnNewPath(const NewPathEvent& event)
{
    if (!event.sender || !event.sender.IsMe())
        return;

    Log::Info("points=%zu dash=%d speed=%.0f",
        event.path.size(),
        event.isDash ? 1 : 0,
        event.dashSpeed);

    if (!event.path.empty())
    {
        const Vec3& destination = event.path.back();
        Log::Info("destination=(%.1f, %.1f, %.1f)", destination.x, destination.y, destination.z);
    }
}

Events::OnNewPath += OnNewPath;
```

Prediction and dash tracking benefit from path events because the path change is observed directly rather than reconstructed by repeatedly polling navigation state.

## Cancellable events

Some public events expose a mutable `process` field. `OnIssueOrder`, `OnCastSpell`, and `OnUpdateChargeableSpell` are examples.

```cpp
void OnIssueOrder(IssueOrderEvent& event)
{
    if (!event.sender || !event.sender.IsMe())
        return;

    Log::Info("order=%d process=%d", static_cast<int>(event.order), event.process ? 1 : 0);

    // Only when your feature intentionally owns this action:
    // event.process = false;
}

Events::OnIssueOrder += OnIssueOrder;
```

Do not globally suppress movement, attacks, or user input from unrelated champion logic.

## Performance guidance

High-frequency handlers should do cheap checks first:

```cpp
if (!Game::IsReady())
    return;

const Object player = ObjectManager::LocalPlayer();
if (!player || player.IsDead())
    return;
```

Avoid printing a log line on every update or draw frame. The full example deliberately throttles its heartbeat log while still printing event-specific information when a spell, path, order, or stop-cast event occurs.

Avoid scanning every object from `OnDraw`; drawing should consume already-decided state whenever practical.

## Next

Continue with [Menu](04_Menu.md).

## RSDK 5.2.2: raw OnNewPath flags

`NewPathEvent::dashGravityFlags` is a 32-bit copy of native `a6_dashGravityBuffer[0..3]`.
`NewPathEvent::IsBlink()` evaluates `(dashGravityFlags & 1u) != 0`.

```cpp
void OnNewPath(const NewPathEvent& event)
{
    if (event.IsBlink())
    {
        // Detect a blink candidate (tested: Flash and Ezreal E).
    }
    const std::uint8_t lowByte = static_cast<std::uint8_t>(event.dashGravityFlags);
    (void)lowByte;
}
```

Do not assume undocumented high bits remain constant. This event layout requires ABI 48.
