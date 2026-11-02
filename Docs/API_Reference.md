# EonCrest / RSDK 5.2.2 — API Reference

This is the cumulative public API reference for RSDK 5.2.2. It describes the current API as one SDK; it does not require reading older version documents first.

For a tutorial-style introduction, read `Getting_Started.md`. For changes introduced specifically in 5.2.2, read `CHANGELOG.md`.

> **Documentation rule:** signatures and behavior are documented here; inline implementation details are intentionally omitted. The shipped `Public/` headers remain the authoritative source when an overload is not shown in full.

## Version matrix

| Component | Version |
|---|---:|
| Public ABI | 48 |
| Plugin ABI | 3 |
| Prediction provider | 2 |
| TargetSelector provider | 1 |
| Orbwalker provider | 6 |
| Damage provider contract | 1 |

## Runtime modules

| API area | Provider |
|---|---|
| Object, Game, Events, Menu, Render, Items, Utility, Geometry | Core |
| Prediction | `Prediction.dll` |
| TargetSelector | `TargetSelector.dll` |
| Orbwalker | `Orbwalker.dll` |
| HealthPrediction | `Orbwalker.dll` |
| Damage | `Damage.dll` (shipped), provider v1 |

---

# 1. Core conventions

- Namespace: `rsdk` unless otherwise stated.
- Gameplay timestamps use `float` seconds.
- `Game::Latency()` returns milliseconds.
- `Object`, `Missile`, `Buff`, `SpellData`, etc. are lightweight wrappers around handles/native addresses. Test wrapper truthiness before using it.
- Long-lived game objects should normally be re-resolved by `NetworkId` when necessary.
- Generic short-layout objects such as missiles and `.troy` particles should use `ObjectName()` / `Name()`. Do not assume `ChampionName()` or `BaseSkinName()` is valid on every object type.
- `Events::OnXxx += handler` creates a plugin-lifetime subscription. `Events::OnXxx.Connect(...)` returns an `EventConnection` for RAII/manual disconnect.
- Provider-backed APIs fail safely when the provider is unavailable; use `IsAvailable()` when the module is optional.

---

# 2. Fundamental enums and types

Header: `RSDKConstants.h`, `RSDKTypes.h`, `Common/Vector.h`, `Common/Color.h`

## `DamageType`

```cpp
enum class DamageType : std::uint8_t
{
    Magical,
    Physical,
    True
};
```

## `TeamFilter`

```cpp
enum class TeamFilter : std::uint8_t
{
    All,
    Ally,
    Enemy
};
```

## `SpellSlot`

```cpp
enum class SpellSlot : std::int8_t
{
    _Q, _W, _E, _R,
    _D, _F,
    _Item1, _Item2, _Item3, _Item4, _Item5, _Item6,
    _SlotTrinket,
    _SlotRecall,
    _SlotUnknown
};
```

## Other common enums

- `GameObjectTeam`
- `ObjectTypeFlags`
- `BuffType`
- `GameObjectOrder`
- `SpellState`

## `Vec2`, `Vec3`, `Vec4`

Vectors provide standard arithmetic plus helpers such as:

```cpp
Length()
LengthSqr()
Normalized()
DotProduct(...)
CrossProduct(...)
DistanceTo(...)
DistanceToSqr(...)
Extend(...)
Rotated(...)
IsValid()
```

`Vec3::To2D()` maps world X/Z to `Vec2`.

## `Color`

```cpp
struct Color
{
    std::uint8_t r, g, b, a;

    Color WithAlpha(std::uint8_t alpha) const noexcept;

    static Color FromHSV(
        float hue,
        float saturation = 1.0f,
        float value = 1.0f,
        std::uint8_t alpha = 255) noexcept;

    static Color Random(std::uint8_t alpha = 255) noexcept;
    static Color Rainbow(float speed = 0.25f, std::uint8_t alpha = 255, float hueOffset = 0.0f) noexcept;
};
```

---

# 3. `NavPath`

Header: `RSDKCpp.hpp`

```cpp
class NavPath final
{
public:
    explicit operator bool() const noexcept;
    std::uintptr_t Address() const noexcept;
    Vec3 StartPos() const noexcept;
    Vec3 EndPos() const noexcept;
    bool IsDashing() const noexcept;
    float DashSpeed() const noexcept;
    std::uint8_t DashGravityCount() const noexcept;
    std::uintptr_t NavArray() const noexcept;
    std::uint32_t NavArrayCount() const noexcept;
};
```

---

# 4. Lightweight game wrappers

Header: `RSDKCpp.hpp`

## `Perk`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
std::uint32_t Id() const noexcept;
std::string Name() const;
```

## `Buff`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
BuffType Type() const noexcept;
float BeginTime() const noexcept;
float EndTime() const noexcept;
float Duration() const noexcept;
float RemainingTime() const noexcept;
std::uint32_t Stacks() const noexcept;
std::uint32_t Count() const noexcept;
bool IsActive() const noexcept;
bool IsValid() const noexcept;
std::string Name() const;
Object Caster() const noexcept;
```

## `Item`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
std::uint8_t Slot() const noexcept;
bool HasSlot() const noexcept;
std::uint32_t OwnerNetworkId() const noexcept;
SpellSlot ItemSpellSlot() const noexcept;
std::uint32_t Id() const noexcept;
std::uint32_t Stacks() const noexcept;
std::uint32_t Charges() const noexcept;
std::string Name() const;
```

## `Missile`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
std::uint32_t Id() const noexcept;
std::uint32_t NetworkId() const noexcept;
std::int32_t Team() const noexcept;
float SpawnTime() const noexcept;
Object Caster() const noexcept;
Object Target() const noexcept;
bool IsDeleted() const noexcept;
bool IsMissile() const noexcept;
std::string Name() const;
Vec3 Position() const noexcept;
Vec3 StartPosition() const noexcept;
Vec3 EndPosition() const noexcept;
Vec3 CastEndPos() const noexcept;
```

---

# 5. `Object`

Header: `RSDKCpp.hpp`

`Object` is the main game-object wrapper.

## Identity and type

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
std::uint32_t Id() const noexcept;
std::uint32_t NetworkId() const noexcept;
std::int32_t Team() const noexcept;
std::uint32_t ObjectType() const noexcept;

bool IsObjectType(std::uint32_t type) const noexcept;
bool IsObjectType(ObjectTypeFlags type) const noexcept;
bool IsEnemy(const Object& other) const noexcept;
```

Type/state helpers include:

```cpp
IsMinion()
IsCreep()
IsHero()
IsTurret()
IsNexus()
IsInhibitor()
IsMissile()
IsVisible()
IsOnScreen()
IsMoving()
IsMe()
IsDead()
IsDeleted()
IsTargetable()
IsInvulnerable()
IsMelee()
IsRanged()
IsTeleport()
```

## Position and path

```cpp
Vec3 Position() const noexcept;
Vec3 ServerPos() const noexcept;
Vec3 Direction() const noexcept;
bool GetBounds(Vec3& outMin, Vec3& outMax) const noexcept;
Vec2 HealthBarScreenPosition() const noexcept;

NavPath GetNavPath() const noexcept;
bool IsDashing() const noexcept;

std::vector<Vec3> GetPath(const Vec3& to, bool smoothPath = true) const;
std::vector<Vec3> GetPath(const Vec3& from, const Vec3& to, bool smoothPath = true) const;
std::vector<Vec3> CurrentPath() const;
```

`ServerPos()` is public in 5.2.1 and is the preferred mapping for LeagueSharp logic that used `ServerPosition`.

## Names

```cpp
std::string ObjectName() const;
std::string Name() const;
std::string ChampionName() const;
std::string BaseSkinName() const;
```

**Safety:** use `ObjectName()` for generic objects such as missiles/particles. `ChampionName()` and `BaseSkinName()` are for full hero/minion/turret-style layouts.

## Health, mana and movement

```cpp
float BoundingRadius() const noexcept;
float Health() const noexcept;
float MaxHealth() const noexcept;
float HealthPercent() const noexcept;
float HealthRegenRate() const noexcept;
float Mana() const noexcept;
float MaxMana() const noexcept;
float ManaPercent() const noexcept;
float MovementSpeed() const noexcept;
float AttackRange() const noexcept;
float AttackSpeed() const noexcept;
std::uint8_t Level() const noexcept;
```

## Offensive stats

```cpp
float Crit() const noexcept;
float CritDamageBonusMultiplier() const noexcept;
float CritDamageMultiplier() const noexcept;

float BaseAttackDamage() const noexcept;
float BonusPhysicalDamage() const noexcept;
float PercentPhysicalDamage() const noexcept;
float TotalPhysicalDamage() const noexcept;
float MagicDamage() const noexcept;
float PercentMagicDamage() const noexcept;
float TotalMagicDamage() const noexcept;

float ArmorPenetrationPercent() const noexcept;
float ArmorPenetrationFlat() const noexcept;
float MagicPenetrationPercent() const noexcept;
float MagicPenetrationFlat() const noexcept;
```

Crit semantics in 5.2.1:

```cpp
CritDamageMultiplier() == 1.0f + CritDamageBonusMultiplier();
```

Typical current values are `2.0f` total without an extra crit-damage modifier and `2.3f` when the raw bonus multiplier is `1.3f`.

## Defensive stats

```cpp
float Armor() const noexcept;
float BonusArmor() const noexcept;
float MagicResist() const noexcept;
float BonusMagicResist() const noexcept;
float AllShield() const noexcept;
float MagicalShield() const noexcept;
float SpecialHealth() const noexcept;
```

## Spellbook and casting

```cpp
float ManaSpell(SpellSlot slot) const noexcept;
float AttackDelay() const noexcept;
float AttackCastDelay() const noexcept;
SpellState GetSpellState(SpellSlot slot) const noexcept;
bool CanUseSpell(SpellSlot slot) const noexcept;

bool Cast(SpellSlot slot) const noexcept;
bool Cast(SpellSlot slot, SpellEventMode eventMode) const noexcept;
bool Cast(SpellSlot slot, const Vec3& position) const noexcept;
bool Cast(SpellSlot slot, const Vec3& position, SpellEventMode eventMode) const noexcept;
bool Cast(SpellSlot slot, const Vec3& from, const Vec3& to) const noexcept;
bool Cast(SpellSlot slot, const Vec3& from, const Vec3& to, SpellEventMode eventMode) const noexcept;
bool Cast(SpellSlot slot, const Object& target) const noexcept;
bool Cast(SpellSlot slot, const Object& target, SpellEventMode eventMode) const noexcept;

bool ReleaseCharge(SpellSlot slot, const Vec3& position, bool releaseCast = true) const noexcept;
bool ReleaseCharge(SpellSlot slot, const Vec3& position, bool releaseCast, SpellEventMode eventMode) const noexcept;
```

`SpellEventMode::Bypass` is used by framework internals when a cast/release must not re-trigger the public cast event path.

## Spell and combat information

```cpp
std::vector<SpellData> GetAllSpells() const;
SpellData GetSpellData(SpellSlot slot) const noexcept;
SpellCast BasicAttack() const noexcept;
SpellCast GetActiveSpellCast() const noexcept;
std::string GetSpellName(SpellSlot slot) const;

float BaseHealthForLevel() const noexcept;
float RespawnTimeRemaining() const noexcept;
```

## Perks, buffs and inventory

```cpp
std::vector<Perk> GetAllPerks() const;
bool HasPerk(std::uint32_t id) const noexcept;
bool HasPerk(const char* name) const noexcept;

std::vector<Buff> GetAllBuffs() const;
std::vector<Buff> GetAllBuffs(const char* name) const;
std::vector<Buff> GetAllBuffs(BuffType type) const;
bool HasBuff(BuffType type) const noexcept;
bool HasBuff(const char* name) const noexcept;
Buff GetFirstBuff(const char* name) const noexcept;
Buff GetFirstBuff(BuffType type) const noexcept;

template <typename Visitor>
void ForEachBuff(Visitor&& visitor) const;

bool HasItem(std::uint32_t itemId) const noexcept;
bool HasItem(const char* itemName) const noexcept;
Item FindItem(std::uint32_t itemId) const noexcept;
Item FindItem(const char* itemName) const noexcept;

template <typename Visitor>
void ForEachItem(Visitor&& visitor) const;

std::vector<Item> GetAllItems() const;
```

## Orders

```cpp
bool IssueOrder(GameObjectOrder order, const Vec3& position, bool triggerEvent = true) const noexcept;
bool IssueOrder(GameObjectOrder order, const Object& target, bool triggerEvent = true) const noexcept;
```

---

# 6. `SpellData`

Header: `RSDKCpp.hpp`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
std::uint8_t Level() const noexcept;
float CooldownExpires() const noexcept;
float CooldownExpires2() const noexcept;
float CooldownTime() const noexcept;
std::uint8_t Ammo() const noexcept;
std::uint8_t ToggleState() const noexcept;
float ProjectileSpeed() const noexcept;
float Range() const noexcept;
float Radius() const noexcept;
float Width() const noexcept;
float BaseCooldownTime() const noexcept;
std::string SpellName() const;
```

---

# 7. `SpellCast`

Header: `RSDKCpp.hpp`

```cpp
explicit operator bool() const noexcept;
std::uintptr_t Address() const noexcept;
Object Caster() const noexcept;
Object Target() const noexcept;
std::uint32_t MissileNetworkId() const noexcept;
float AttackCastDelay() const noexcept;
float AttackDelay() const noexcept;
std::uint8_t Slot() const noexcept;
float CastTime() const noexcept;
float CastEndTime() const noexcept;
float CooldownExpires() const noexcept;
Vec3 StartPos() const noexcept;
Vec3 EndPos() const noexcept;
Vec3 CastEndPos() const noexcept;
std::string SpellName() const;
bool IsAutoAttack() const noexcept;
float ProjectileSpeed() const noexcept;
float Range() const noexcept;
float Radius() const noexcept;
float Width() const noexcept;
```

`MissileNetworkId()` is the cast-time identity. The live missile object may not exist yet.

---

# 8. `ObjectManager`

Header: `RSDKCpp.hpp`

```cpp
Object LocalPlayer() noexcept;
Object FindObjectByNetworkId(std::uint32_t networkId) noexcept;
Object FindObjectById(std::uint32_t id) noexcept;
Missile FindMissileByNetworkId(std::uint32_t networkId) noexcept;
Missile FindMissileById(std::uint32_t id) noexcept;

std::vector<Object> AllHeroes(TeamFilter filter = TeamFilter::All);
std::vector<Object> AttackableUnits(TeamFilter filter = TeamFilter::All);
std::vector<Object> AllMinions(TeamFilter filter = TeamFilter::All);
std::vector<Object> AllTurrets(TeamFilter filter = TeamFilter::All);
std::vector<Object> AllBuildingObjects(TeamFilter filter = TeamFilter::All);
std::vector<Object> AllInhibitors(TeamFilter filter = TeamFilter::All);
std::vector<Object> AllObjects();
std::vector<Missile> AllMissiles(TeamFilter filter = TeamFilter::All);

Object EnemyNexus() noexcept;
Object TeamNexus() noexcept;
```

`AttackableUnits()` returns attackable game objects, including heroes, minions, turrets, and other attackable objects. Missiles are excluded.

---

# 9. `Game`

Header: `RSDKCpp.hpp`

```cpp
float TimeGame() noexcept;
float Time() noexcept; // compatibility alias
std::int32_t Latency() noexcept;
void PrintChat(const char* text) noexcept;
Vec3 CursorPosition() noexcept;
std::int32_t State() noexcept;
bool IsReady() noexcept;
bool IsFocused() noexcept;
bool IsChatOpen() noexcept;
std::uint16_t CollisionFlag(const Vec3& position) noexcept;
bool IsWall(const Vec3& position) noexcept;
bool IsGrass(const Vec3& position) noexcept;
bool IsInFow(const Vec3& position) noexcept;
Vec2 WorldToScreen(const Vec3& position) noexcept;
Vec2 WorldToMinimap(const Vec3& position) noexcept;
```

RSDK intentionally does not add a separate LeagueSharp-style `NavMesh` facade for collision flags; use `Game::CollisionFlag`, `Game::IsWall`, and related Core helpers.

---

# 10. Localization

Header: `RSDKCpp.hpp`

Lazy menu-localized text:

```cpp
LocalizedText Tr(const char* key, const char* fallback = nullptr);
```

Immediate localization:

```cpp
namespace Localization
{
    std::string Tr(const char* key, const char* fallback = nullptr);
    std::string Locale();
    const char* Text(const char* key, const char* fallback = nullptr);
    bool SetLocale(const char* locale) noexcept;
    bool Reload() noexcept;
}
```

Language files use stable flat domains such as:

```text
Languages/Core/<locale>.json
Languages/Orbwalker/<locale>.json
Languages/TargetSelector/<locale>.json
```

---

# 11. Logging and config

Headers: `RSDKFramework.hpp`, `RSDKCpp.hpp`

## `Log`

```cpp
Log::Trace(format, ...);
Log::Info(format, ...);
Log::Warning(format, ...);
Log::Error(format, ...);
bool Log::Clear() noexcept;
```

## `Config`

```cpp
void Config::SetUseRawPointer(bool enabled) noexcept;
bool Config::UseRawPointer() noexcept;
```

---

# 12. Events

Header: `RSDKFramework.hpp`

## Event sources

```cpp
Events::OnTick
Events::OnUpdate
Events::OnDraw
Events::OnWndProc
Events::OnCreateObject
Events::OnDeleteObject
Events::OnIssueOrder
Events::OnNewPath
Events::OnProcessSpell
Events::OnDoCast
Events::OnPlayAnimation
Events::OnStopCast
Events::OnCastSpell
Events::OnUpdateChargeableSpell
Events::OnBuffAddRemove
Events::OnUpdateBuff
Events::OnSpellImpact
Events::OnGameUpdate
```

Subscription forms:

```cpp
Events::OnTick += handler;

EventConnection connection = Events::OnGameUpdate.Connect(handler);
connection.Disconnect();
```

## Event payloads

### `IssueOrderEvent`

```cpp
Object sender;
GameObjectOrder order;
Vec3 position;
Object target;
bool isAttackMove;
bool process;
```

Set `process = false` to cancel the order.

### `NewPathEvent`

```cpp
Object sender;
std::vector<Vec3> path;
bool isDash;
float dashSpeed;
std::uint8_t dashGravityCount;
```

### `ProcessSpellEvent`

```cpp
Object sender;
SpellCast spellCast;
```

### `DoCastSpellEvent`

```cpp
SpellCast spellCast;
Object sender;
```

### `PlayAnimationEvent`

```cpp
Object sender;
std::string animationName;
```

### `StopCastEvent`

```cpp
Object sender;
SpellCast spellCast;
bool stopAnimation;
bool destroyMissile;
std::uint32_t missileNetworkId;
```

### `CastSpellEvent`

```cpp
Object sender;
Object target;
SpellData spellData;
Vec3 sourcePosition;
Vec3 destinationPosition;
Vec3 castEndPosition;
Vec3 unknownPosition;
std::uint32_t sourceId;
std::uint32_t targetId;
SpellSlot slot;
bool process;
```

Set `process = false` to cancel the cast.

### `UpdateChargeableSpellEvent`

```cpp
Object sender;
SpellData spellData;
SpellSlot slot;
Vec3 position;
bool flag;
bool process;
```

### `BuffAddRemoveEvent`

```cpp
Object sender;
Buff buff;
bool added;
```

### `UpdateBuffEvent`

```cpp
Object sender;
Buff buff;
std::int32_t stacks;
```

### `SpellImpactEvent`

```cpp
Object sender;
Object target;
SpellCast spellCast;
std::int32_t nativeArgument;
```

---

# 13. `Delay`

Header: `RSDKFramework.hpp`

```cpp
using TaskId = std::uint64_t;
inline constexpr TaskId InvalidTask = 0u;

template <typename Callable>
TaskId After(std::uint32_t delayMs, Callable&& callable);

template <typename Callable>
TaskId Every(std::uint32_t intervalMs, Callable&& callable);

bool Cancel(TaskId id);
void Clear() noexcept;
```

Delay timing uses `std::chrono::steady_clock` and updates through `Events::OnGameUpdate`.

---

# 14. Rendering

Header: `RSDKRender.hpp`

## `Texture`

```cpp
static Texture LoadFromFile(const char* filePath) noexcept;
static Texture LoadFromMemory(const void* data, std::size_t size) noexcept;
void Reset() noexcept;
explicit operator bool() const noexcept;
```

## `ImageStyle`

```cpp
ImageShape shape;
Color tint;
Color borderColor;
float borderThickness;
float rounding;
```

## `Render`

```cpp
bool IsReady() noexcept;
void Text(const Vec2& position, const char* text, Color color, float fontSize = 0.0f) noexcept;
void Line2D(const Vec2& from, const Vec2& to, Color color, float thickness = 1.0f) noexcept;
void Circle2D(const Vec2& center, float radius, Color color, float thickness = 1.0f, std::uint32_t segments = 64u) noexcept;
void Rect(const Vec2& position, const Vec2& size, Color color, float thickness = 1.0f, float rounding = 0.0f) noexcept;
void FilledRect(const Vec2& position, const Vec2& size, Color color, float rounding = 0.0f) noexcept;
void Line3D(const Vec3& from, const Vec3& to, Color color, float thickness = 1.0f) noexcept;
void Circle3D(const Vec3& center, float radius, Color color, float thickness = 1.0f,
              std::uint32_t segments = 64u, bool clipTerrain = false) noexcept;
void CircleGlow3D(const Vec3& center, float radius, Color color, float thickness = 2.0f,
                  float glowSize = 10.0f, std::uint32_t glowLayers = 4u,
                  std::uint32_t segments = 64u, bool clipTerrain = false) noexcept;
void RainbowCircle3D(const Vec3& center, float radius, float thickness = 2.0f,
                     std::uint32_t segments = 96u, float speed = 0.20f,
                     bool clockwise = true, std::uint8_t alpha = 255u,
                     bool clipTerrain = false) noexcept;
void Image(const Texture& texture, const Vec2& position, const Vec2& size,
           const ImageStyle& style = {}) noexcept;
```

---

# 15. Menu

Header: `RSDKMenu.hpp`

## Categories and keybind types

```cpp
enum class MenuCategory
{
    Champion,
    Gameplay,
    Combat, // legacy alias of Gameplay
    Utility,
    Core,
    Debug
};

enum class KeyBindType
{
    Press,
    Toggle
};
```

## Menu item wrappers

Common base:

```cpp
MenuItem::SetTooltip(...)
MenuItem::SetSameLine(...)
MenuItem::SetPersistent(...)
```

Value wrappers:

```cpp
MenuBool::Value() / SetValue(bool)
MenuInt::Value() / SetValue(std::int32_t)
MenuFloat::Value() / SetValue(float)
MenuKeyBind::Key() / SetKey(...) / Active()
MenuColor::Value() / SetValue(Color)
MenuButton::ConsumePressed()
```

## Hierarchy

```text
MenuCategory
  -> MenuPage
      -> MenuTab
          -> optional MenuSection
              -> MenuGroup
                  -> MenuItem
```

`MenuSection` is optional. Items and groups may be created directly on a `MenuTab`.

### `Menu::CreatePage`

```cpp
MenuPage Menu::CreatePage(const char* id, const char* title, MenuCategory category) noexcept;
bool Menu::IsOpen() noexcept;
void Menu::SetOpen(bool open) noexcept;
void Menu::Toggle() noexcept;
```

### `MenuPage`

```cpp
MenuTab AddTab(const char* id, const char* title) const noexcept;
bool Remove() const noexcept;
```

### `MenuTab`

```cpp
MenuSection AddSection(...)
MenuGroup AddGroup(...)
MenuBool AddBool(...)
MenuSliderInt AddSliderInt(...)
MenuSliderFloat AddSliderFloat(...)
MenuCombo AddCombo(...)
MenuKeyBind AddKeyBind(...)
MenuColor AddColor(...)
MenuButton AddButton(...)
void AddText(...)
void AddSeparator(...)
```

It also exposes linked item-creation variants for convenient ID-based storage in `MenuLinks`.

### `MenuSection` / `MenuGroup`

Both expose the normal item-creation functions. `MenuGroup` also supports nested groups and tooltips.

### Linked items

`MenuLinks` provides collections for storing items by string key:

```cpp
MenuLinks links;

auto useQ = group.AddLinkedBool(links.Bools, "use_q", "Use Q", true);
auto mode = group.AddLinkedCombo(links.Combos, "mode", "Mode", {"A", "B"}, 0);
```

---

# 16. `HeroManager`

Header: `Common/HeroManager.h`

```cpp
bool IsInitialized() noexcept;
Object Player();
const std::vector<Object>& AllHeroes();
const std::vector<Object>& Allies();
const std::vector<Object>& Enemies();
void Refresh();
```

---

# 17. `Items`

Header: `Common/Items.h`

```cpp
SpellSlot ToSpellSlot(std::uint8_t inventorySlot) noexcept;

Item FindItem(std::uint32_t itemId, const Object& owner);
Item FindItem(const char* itemName, const Object& owner);
Item FindItem(std::uint32_t itemId);
Item FindItem(const char* itemName);

bool HasItem(std::uint32_t itemId, const Object& owner);
bool HasItem(const char* itemName, const Object& owner);
bool HasItem(std::uint32_t itemId);
bool HasItem(const char* itemName);

std::vector<SpellSlot> GetSlots(std::uint32_t itemId, const Object& owner);
std::vector<SpellSlot> GetSlots(std::uint32_t itemId);

SpellSlot GetWardSlot();
Item GetWard();

bool CanUseItem(std::uint32_t itemId);
bool CanUseItem(const char* itemName);

bool UseItem(std::uint32_t itemId);
bool UseItem(const char* itemName);
bool UseItem(std::uint32_t itemId, const Object& target);
bool UseItem(const char* itemName, const Object& target);
bool UseItem(std::uint32_t itemId, const Vec3& position);
bool UseItem(const char* itemName, const Vec3& position);
```

`Common/Items.h` also includes the range-aware convenience `Items::Item` helper.

---

# 18. `Utility`

Header: `Common/Utility.h`

```cpp
bool IsValidTarget(
    const Object& unit,
    float range = std::numeric_limits<float>::max(),
    bool checkTeam = true,
    const Vec3* from = nullptr);

std::vector<Vec3> GetWaypoints(const Object& unit);
float PathLength(const std::vector<Vec3>& path) noexcept;

std::vector<Vec2> CutPath(const std::vector<Vec2>& path, float distance);
std::vector<Vec3> CutPath(const std::vector<Vec3>& path, float distance);

float PhysicalDamageMultiplier(const Object& source, const Object& target) noexcept;
float MagicalDamageMultiplier(const Object& source, const Object& target) noexcept;
float CalculateDamage(const Object& source, const Object& target, DamageType type, float rawDamage) noexcept;

void DrawCircle(...);
```

`Utility::CalculateDamage()` is a lightweight raw-damage mitigation helper. The separate full Damage provider is implemented in `Damage.dll`.

---

# 19. Geometry

Header: `Geometry.h`

Namespace: `rsdk::geometry`

Common helpers:

```cpp
DistanceSquared(...)
Distance(...)
Dot(...)
Cross(...)
ClosestPointOnSegment(...)
DistanceSquaredToSegment(...)
DistanceToSegment(...)
SegmentIntersection(...)
IsInsideCircle(...)
```

Circle / MEC:

```cpp
struct Circle2D
{
    Vec2 Center;
    float Radius;
    bool Valid;
};

CircleFromTwoPoints(...)
CircleFromThreePoints(...)
Encloses(...)
CircleCircleIntersections(...)
ConvexHull(...)
MinimumEnclosingCircle(...)
```

Movement geometry:

```cpp
struct MovementCollisionResult
{
    bool Valid;
    float Time;
    Vec2 Position;
};

VectorMovementCollision(...)
MovingCircleIntersectsInterval(...)
```

---

# 20. `MinionManager`

Header: `Common/MinionManager.h`

```cpp
struct FarmLocation
{
    Vec3 Position;
    int MinionsHit;
};

bool IsWard(const Object& minion);
bool IsMinion(const Object& minion, bool includeWards = false);

std::vector<Vec3> GetMinionsPredictedPositions(...);

FarmLocation GetBestCircularFarmLocation(...);
FarmLocation GetBestLineFarmLocation(...);
```

Both farm-location helpers provide overloads for object lists and already predicted positions.

---

# 21. Prediction

Header: `Common/Prediction.h`

Requires: `Prediction.dll`

## Enums

```cpp
enum class HitChance
{
    Collision,
    OutOfRange,
    Impossible,
    Low,
    Medium,
    High,
    VeryHigh,
    Dashing,
    Immobile
};

enum class SkillshotType
{
    Line,
    Circle,
    Cone
};

enum class CollisionableObjects : std::uint32_t
{
    None,
    Minions,
    Heroes,
    YasuoWall,
    Walls,
    Allies
};
```

`CollisionableObjects` supports bitwise `|` and `&`.

## `PredictionInput`

```cpp
struct PredictionInput
{
    Object Unit;
    bool Aoe;
    bool Collision;
    CollisionableObjects CollisionObjects;
    float Delay;
    float Radius;
    float Range;
    float Speed;
    SkillshotType Type;
    bool UseBoundingRadius;
    Vec3 From;
    Vec3 RangeCheckFrom;

    float RealRadius() const noexcept;
};
```

## `PredictionOutput`

```cpp
struct PredictionOutput
{
    std::vector<Object> AoeTargetsHit;
    std::vector<Object> CollisionObjects;
    HitChance Hitchance;
    Vec3 CastPosition;
    Vec3 UnitPosition;
    std::uint32_t AoeTargetsHitCount;
};
```

## Methods

```cpp
bool Prediction::IsAvailable() noexcept;

PredictionOutput Prediction::GetPrediction(const Object& unit, float delay);
PredictionOutput Prediction::GetPrediction(const Object& unit, float delay, float radius);
PredictionOutput Prediction::GetPrediction(const Object& unit, float delay, float radius, float speed);
PredictionOutput Prediction::GetPrediction(const PredictionInput& input);

std::vector<Object> Prediction::GetCollision(
    const std::vector<Vec3>& positions,
    const PredictionInput& input);

std::vector<Object> Prediction::GetCollision(
    const Vec3& position,
    const PredictionInput& input);
```

Generic collision accepts arbitrary endpoint lists. This is the shared collision primitive used by Prediction and `Spell::GetCollision()`.

---

# 22. TargetSelector

Header: `Common/TargetSelector.h`

Requires: `TargetSelector.dll`

## Targeting modes

```cpp
AutoPriority
LowHP
MostAD
MostAP
Closest
NearMouse
LessAttack
LessCast
MostStack
```

## Filter

```cpp
using TargetFilter = bool(*)(const Object& target);
```

## Methods

```cpp
bool IsAvailable() noexcept;
bool Initialize() noexcept;
void Shutdown() noexcept;

bool GetCustomTS() noexcept;
void SetCustomTS(bool value) noexcept;
void AddToMenu(const MenuGroup& group) noexcept;

Object GetSelectedTarget() noexcept;
void SetTarget(const Object& hero) noexcept;
void ClearTarget() noexcept;

float GetPriority(const Object& hero) noexcept;
int GetPriorityLevel(const Object& hero) noexcept;
void SetPriority(const Object& hero, int priority) noexcept;

TargetingMode GetMode() noexcept;
void SetMode(TargetingMode mode) noexcept;

bool IsInvulnerable(const Object& target, DamageType damageType, bool ignoreShields = true) noexcept;

Object GetTargetNoCollision(
    const Spell& spell,
    bool ignoreShield = true,
    const std::vector<Object>* ignoredChamps = nullptr,
    const Vec3* rangeCheckFrom = nullptr);
```

`GetTarget(...)` overloads support:

- local player or an explicit source champion;
- range;
- `DamageType`;
- shield handling;
- ignored champion lists;
- custom range-check origin;
- optional `TargetFilter`.

Compatibility helper: `LockedTargetSelector` provides sticky/locked-target behavior.

---

# 23. Orbwalker

Header: `Common/Orbwalker.h`

Requires: `Orbwalker.dll`

## Modes

```cpp
Orbwalker::Mode::LastHit
Orbwalker::Mode::Mixed
Orbwalker::Mode::LaneClear
Orbwalker::Mode::Combo
Orbwalker::Mode::Freeze
Orbwalker::Mode::CustomMode
Orbwalker::Mode::None
```

## Main methods

```cpp
bool IsAvailable() noexcept;
Mode ActiveMode() noexcept;
void SetMode(Mode mode) noexcept;

Object GetTarget() noexcept;
void ForceTarget(const Object& target) noexcept;
void ClearForcedTarget() noexcept;

bool AttackEnabled() noexcept;
void SetAttack(bool enabled) noexcept;
bool MovementEnabled() noexcept;
void SetMovement(bool enabled) noexcept;

bool CanAttack() noexcept;
bool CanMove(float extraWindup = 0.0f, bool disableMissileCheck = false) noexcept;

float GetAttackRange(const Object& target) noexcept;
float GetRealAutoAttackRange(const Object& target = {}) noexcept;
bool InAutoAttackRange(const Object& target) noexcept;
float GetMyProjectileSpeed() noexcept;

bool IsAutoAttack(const char* name) noexcept;
bool IsAutoAttackReset(const char* name) noexcept;
void ResetAutoAttackTimer() noexcept;

void SetMinimumOrbwalkDistance(float distance) noexcept;
void SetMovementDelay(float delay) noexcept;
void SetOrbwalkingPoint(const Vec3& position) noexcept;
void ClearOrbwalkingPoint() noexcept;

void Orbwalk(
    const Object& target,
    const Vec3& position,
    float extraWindup = 0.090f,
    float holdAreaRadius = 0.0f,
    bool useFixedDistance = true,
    bool randomizeMinDistance = true) noexcept;

void MoveTo(
    const Vec3& position,
    float holdAreaRadius = 0.0f,
    bool overrideTimer = false,
    bool useFixedDistance = true,
    bool randomizeMinDistance = true) noexcept;

bool ShouldWait() noexcept;

Object LastTarget() noexcept;
std::array<Object, 3> LastTargets() noexcept;
float LastAATick() noexcept;
float LastAttackCommandTime() noexcept;
float LastMoveCommandTime() noexcept;
Vec3 LastMoveCommandPosition() noexcept;
```

Orbwalker gameplay timestamps are seconds.

## Attack events

```cpp
Orbwalker::BeforeAttack
Orbwalker::OnAttack
Orbwalker::AfterAttack
Orbwalker::OnNonKillableMinion
Orbwalker::OnTargetChange
```

Payloads:

```cpp
struct BeforeAttackEvent
{
    Object Unit;
    Object Target;
    bool Process;
};

struct OrbwalkerAttackEvent
{
    Object Unit;
    Object Target;
};

struct OrbwalkerTargetChangeEvent
{
    Object OldTarget;
    Object NewTarget;
};
```

Set `BeforeAttackEvent::Process = false` to cancel the attack.

---

# 24. HealthPrediction

Header: `Common/HealthPrediction.h`

Implementation provider: `Orbwalker.dll`

```cpp
bool HealthPrediction::IsAvailable() noexcept;

float HealthPrediction::GetHealthPrediction(
    const Object& unit,
    float time,
    float delay = 0.070f) noexcept;

float HealthPrediction::LaneClearHealthPrediction(
    const Object& unit,
    float time,
    float delay = 0.070f) noexcept;

bool HealthPrediction::HasMinionAggro(const Object& minion) noexcept;
bool HealthPrediction::HasTurretAggro(const Object& minion) noexcept;
float HealthPrediction::TurretAggroStartTime(const Object& minion) noexcept;
```

Lane classification uses a 1000-unit nearest-lane threshold and does not cache `Unknown`.

---

# 25. `Spell`

Header: `Common/Spell.h`

`Spell` is a header-only convenience facade. It uses Core for basic casts and delegates to Prediction / TargetSelector / HealthPrediction providers when those features are requested.

## Enums

```cpp
enum class SpellCastState
{
    SuccessfullyCasted,
    NotReady,
    NotCasted,
    InvalidTarget,
    OutOfRange,
    Collision,
    NotEnoughTargets,
    LowHitChance,
    PredictionUnavailable
};

enum class ChargedReleaseMode
{
    UpdateOnly,
    UpdateThenCast
};
```

## Construction and state

```cpp
Spell();
explicit Spell(
    SpellSlot slot,
    float range = std::numeric_limits<float>::max(),
    DamageType damageType = DamageType::Physical) noexcept;

SpellSlot Slot() const noexcept;
void SetSlot(SpellSlot value) noexcept;
DamageType GetDamageType() const noexcept;
void SetDamageType(DamageType value) noexcept;

bool IsSkillshot() const noexcept;
bool IsChargedSpell() const noexcept;
ChargedReleaseMode GetChargedReleaseMode() const noexcept;
bool IsChannelProtected() const noexcept;
bool IsChanneling() const noexcept;
bool Collision() const noexcept;
float Delay() const noexcept;
float Width() const noexcept;
float Speed() const noexcept;
SkillshotType Type() const noexcept;
HitChance MinHitChance() const noexcept;
CollisionableObjects CollisionObjects() const noexcept;
float LastCastAttemptTime() const noexcept;
```

## Configuration

```cpp
Spell& SetMinHitChance(HitChance value) noexcept;
Spell& SetCollisionObjects(CollisionableObjects value) noexcept;
Spell& SetAddSelfHitboxToRange(bool value) noexcept;
Spell& SetAddEnemyHitboxToRange(bool value) noexcept;
Spell& SetChargedReleaseMode(ChargedReleaseMode value) noexcept;

Spell& UpdateSourcePosition(Vec3 from = {}, Vec3 rangeCheckFrom = {}) noexcept;

Spell& SetSkillshot(
    float delay,
    float width,
    float speed,
    bool collision,
    SkillshotType type,
    Vec3 from = {},
    Vec3 rangeCheckFrom = {}) noexcept;

Spell& SetTargeted(
    float delay,
    float speed,
    Vec3 from = {},
    Vec3 rangeCheckFrom = {}) noexcept;
```

## Range/readiness

```cpp
Vec3 From() const noexcept;
Vec3 RangeCheckFrom() const noexcept;
float Range() const noexcept;
float RangeSqr() const noexcept;
float WidthSqr() const noexcept;
bool IsReady() const noexcept;
float GetRange(const Object& target) const noexcept;
float GetRangeSqr(const Object& target) const noexcept;
bool CanCast(const Object& unit) const noexcept;
std::uint8_t Level() const noexcept;
float ManaCost() const noexcept;
float Cooldown() const noexcept;
```

## Charged spells

```cpp
bool SetCharged(
    std::string spellName,
    std::string buffName,
    float minRange,
    float maxRange,
    float deltaTime);

bool IsCharging() const noexcept;
bool StartCharging() noexcept;
bool StartCharging(const Vec3& position) noexcept;
bool ReleaseCharge(const Vec3& position, bool releaseCast = true) noexcept;
```

`UpdateOnly` is the default release mode. `UpdateThenCast` is opt-in for spells verified to require the second cast call.

## Protected channel spells

```cpp
bool SetChannelProtection(
    bool allowUserCancel,
    bool usePositionCast,
    bool allowOtherSpellsToCancel = false);

void DisableChannelProtection() noexcept;
```

While a supported protected channel is active, the helper can block move/attack orders and optionally other spell casts. Natural completion is tracked from the known channel effect object lifetime; interruption/cancellation also uses the stop-cast path.

## Target selection

```cpp
Object GetTarget(
    float extraRange = 0.0f,
    TargetSelector::TargetFilter filter = nullptr) const;

Object GetTarget(
    float extraRange,
    const std::vector<Object>& ignoredChamps,
    TargetSelector::TargetFilter filter = nullptr) const;
```

## Prediction and collision

```cpp
PredictionOutput GetPrediction(
    const Object& unit,
    bool aoe = false,
    float overrideRange = -1.0f) const;

PredictionOutput GetPrediction(
    const Object& unit,
    bool aoe,
    float overrideRange,
    CollisionableObjects collisionObjects) const;

std::vector<Object> GetCollision(
    const Vec3& from,
    const std::vector<Vec3>& positions,
    float delayOverride = -1.0f) const;

std::vector<Object> GetCollision(
    const Vec3& from,
    const Vec3& position,
    float delayOverride = -1.0f) const;
```

## Casting

```cpp
SpellCastState Cast(const Object& unit, bool aoe = false);
SpellCastState Cast(const Object& unit, HitChance minimumHitChance, bool aoe = false);

bool CastIfHitchanceEquals(const Object& unit, HitChance hitChance);
bool CastIfHitchanceMinimum(const Object& unit, HitChance hitChance);
bool CastIfWillHit(const Object& unit, int minTargets = 5);
SpellCastState CastOnBestTarget(float extraRange = 0.0f, bool aoe = false);

bool CastOnUnit(const Object& unit);
bool Cast();
bool Cast(const Vec3& position);
bool Cast(const Vec3& fromPosition, const Vec3& toPosition);
```

## Hit/farm helpers

```cpp
bool WillHit(const Object& unit, const Vec3& castPosition,
             float extraWidth = 0.0f,
             HitChance minHitChance = HitChance::High) const;

bool WillHit(const Vec3& point, const Vec3& castPosition,
             float extraWidth = 0.0f) const noexcept;

int CountHits(const std::vector<Object>& units, const Vec3& castPosition) const;
int CountHits(const std::vector<Object>& units, const Vec3& castPosition,
              float extraWidth, HitChance minHitChance) const;
int CountHits(const std::vector<Vec3>& points, const Vec3& castPosition,
              float extraWidth = 0.0f) const noexcept;

float GetHealthPrediction(const Object& unit) const noexcept;
int GetHitCount(HitChance hitChance = HitChance::High) const;

MinionManager::FarmLocation GetCircularFarmLocation(... ) const;
MinionManager::FarmLocation GetLineFarmLocation(... ) const;
```

See the current Public headers for Damage and spell-damage helper availability; check the Damage provider at runtime.

---

# 26. Damage

Header: `Common/Damage.h`

Provider contract exists in 5.2.1. `Damage.dll` is part of the 5.2.2 runtime.

## `DamageStage`

```cpp
enum class DamageStage
{
    Default,
    WayBack,
    Detonation,
    DamagePerSecond,
    SecondForm,
    SecondCast,
    Buff,
    Empowered
};
```

## Methods

```cpp
bool Damage::IsAvailable() noexcept;

float Damage::CalculateDamage(
    const Object& source,
    const Object& target,
    DamageType damageType,
    float rawDamage) noexcept;

float Damage::GetSpellDamage(
    const Object& source,
    const Object& target,
    SpellSlot slot,
    DamageStage stage = DamageStage::Default,
    std::uint32_t flags = 0u) noexcept;

float Damage::GetAutoAttackDamage(
    const Object& source,
    const Object& target,
    bool includePassives = true) noexcept;
```

Until a provider registers, these calculations are unavailable and `Damage::IsAvailable()` is false.

---

# 27. Plugin API

Headers: `RSDKPlugin.h`, `RSDKFramework.hpp`

## Plugin metadata macros

Typical definitions:

```cpp
#define Plugin_Id              "Stable.Plugin.Id"
#define Plugin_Name            "Display Name"
#define Author_Name            "Author"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Description"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)
```

## `Plugin`

```cpp
class Plugin
{
public:
    virtual ~Plugin() = default;
    virtual bool OnLoad();
    virtual void OnUnload();
};
```

Register once at the end of the plugin source:

```cpp
RSDK_REGISTER_PLUGIN(MyPlugin);
```

The macro generates the three required plugin exports:

```cpp
RSDK_PluginQuery
RSDK_PluginLoad
RSDK_PluginUnload
```

Plugin ABI 3 includes `persistentId`; when `Plugin_Id` is not provided, Core falls back to the DLL filename stem.

---

# 28. C ABI / provider summary

Most C++ champion plugins should use the C++ wrappers above. This section exists for direct C ABI consumers and provider implementers.

## Top-level API

`RSDK_Api` exposes versioned sub-tables for Core services such as:

```text
log
events
objects
objectManager
game
render
menu
localization
targetSelector
orbwalker
prediction
damage
...
```

Direct C ABI consumers should check `structSize`, API version fields and individual function pointers before use.

## Provider versions

```cpp
RSDK_TARGET_SELECTOR_API_VERSION = 1
RSDK_PREDICTION_API_VERSION      = 2
RSDK_ORBWALKER_API_VERSION       = 6
RSDK_DAMAGE_API_VERSION          = 1
```

### Prediction provider v2

Main primitives:

```cpp
GetPrediction(...)
GetCollision(...)
```

Collision uses `RSDK_CollisionRequest` / `RSDK_CollisionResult` and accepts a synchronous `positions + positionCount` array.

### Orbwalker provider v6

Includes the normal Orbwalker state/attack functions plus:

```cpp
MoveTo(...)
ShouldWait()
```

### Damage provider v1

Provider primitives:

```cpp
CalculateDamage(...)
GetSpellDamage(...)
GetAutoAttackDamage(...)
```

The provider contract is implemented by the shipped `Damage.dll`; consumers should check `Damage::IsAvailable()` when provider loading is optional.

---

# 29. Compatibility notes

- Menu branding is **EonCrest**, but public ABI/API identifiers remain `RSDK_*` for compatibility.
- `Game::Time()` remains a compatibility alias for `Game::TimeGame()`.
- `TargetSelector::DamageType` remains an alias to the shared `rsdk::DamageType`.
- `MenuCategory::Combat` remains an alias of `MenuCategory::Gameplay`.
- The public object wrapper uses `Address()` for the raw native object address.
- `SpellCast::Missile()` is no longer the cast-time public contract; use `MissileNetworkId()` and resolve the missile later if required.

---

# 30. Where to start

For normal champion code, the most commonly used surfaces are:

```text
Object / ObjectManager
Game
Events
Spell
Prediction
TargetSelector
Orbwalker
HealthPrediction
Menu
Render
```

Read `Getting_Started.md` for practical examples.

## Menu category Misc (5.2.1)

Use `Menu::CreatePage("my_misc_plugin", "My Misc Plugin", MenuCategory::Misc)` to place a plugin page under `Menu → Misc`. This C++ value wraps `RSDK_MENU_CATEGORY_MISC = 5`. Existing category numeric values remain unchanged. Plugins under `Plugins\Misc\` are discovered by Plugin Manager regardless of their page category; choose `Misc` explicitly for correct menu placement.

## Damage 5.2.1 database

`Damage.dll` loads `Plugins/Gameplay/Data/Damage/16.20.json` once at initialization. The file initially contains LeagueSharp SDK 6.7 values; the versioned filename is NOT proof of current-game accuracy. The `RSDK_SPELL_DAMAGE_CURRENT_DATA_ONLY` flag is deprecated and accepted as a no-op with the single database. Champion/passive/item special cases use C++ handlers. Removed masteries and rune effects are excluded.


## Additions in RSDK 5.2.2

### Object wrappers
```cpp
[[nodiscard]] bool Object::DoEmote(Emotes emoteId) const noexcept;
[[nodiscard]] bool Object::IsZombie() const noexcept;
[[nodiscard]] bool Object::IsClone() const noexcept;
[[nodiscard]] float Object::PhysicalShield() const noexcept;
[[nodiscard]] bool Object::IsTeleport() const noexcept;
[[nodiscard]] const char* Object::TeleportState() const noexcept;
```
`IsTeleport` replaces `IsRecalling`. `TeleportState()` is a borrowed `const char*` from the game, or `nullptr` when no teleport/recall state is present. The pointer is not owned by the plugin; copy the text immediately if needed later. The appended `RSDK_ObjectApi::GetTeleportState` field is guarded by `structSize`, keeping SDK ABI 48 for this compatible extension. `DoEmote` is valid on the local player only. The source of physical shielding is the existing native `RawGameObject::PhysicalShield()` mapping.

### Orbwalker
`Orbwalker::Mode::Flee` is the public C++ mapping of `RSDK_ORBWALKER_MODE_FLEE = 7`. The built-in Flee mode does **not** cast champion spells. Use `if (Orbwalker::ActiveMode() == Orbwalker::Mode::Flee)` in champion code.

`Orbwalker::SpellFarmEnabled()` reports the **global, Core-owned** spell-farming toggle set by the Orbwalker keybind. It does not change auto attacks or imply the currently active C/V mode.

### OnNewPath
```cpp
uint32_t NewPathEvent::dashGravityFlags;      // raw native a6[0..3]
bool NewPathEvent::IsBlink() const noexcept;  // (dashGravityFlags & 1u) != 0u
```
Native flag semantics beyond bit 0 are still under investigation. Prediction's DashTracker consumes the same public event flag.

### Compatibility
`RSDK_ABI_VERSION` = **48**; `RSDK_ORBWALKER_API_VERSION` = **6**. Rebuild all DLLs using this SDK; the plugin loader requires the current SDK ABI.
