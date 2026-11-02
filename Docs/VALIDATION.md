# RSDK v5.2.2 – Validation report

## Baseline
- Source archive: `RSDK_Pro_v5.2.1__build_ok.zip`
- Previous SDK ABI: 47
- Current SDK ABI: 48 (required due to extended object and OnNewPath C ABI structs)
- Current Orbwalker API version: 6
- Existing Plugin ABI preserved

## Passed (Linux Clang C++17 syntax checks)
- `Modules/Gameplay/Orbwalker/Orbwalker.cpp` — PASS
- `Modules/Gameplay/Orbwalker/HealthPrediction.cpp` — PASS
- `Modules/Gameplay/Prediction/DashTracker.cpp` — PASS
- `Modules/Gameplay/Prediction/Prediction.cpp` — PASS
- `Modules/Gameplay/Damage/DamagePassives.cpp` — PASS
- `Modules/Gameplay/Damage/Damage.cpp` — PASS
- `Modules/Gameplay/Evade/Evade.cpp` — PASS
- `Modules/Gameplay/TargetSelector/TargetSelector.cpp` — PASS
- `Source/Core/OrbwalkerService.cpp` — PASS
- `Tests/api_smoke_5.2.2.cpp` — PASS

## Passed (source consistency checks)
- `Tests/validate_522.py`: 23/23 checks
- Added fields, conversions, provider state, event flags, old-name removal and localized key names checked.
- No edit to Core/Renderer menu rendering; existing V5.2.1 menu, fonts and terrain clip source preserved.

## Not validated on this system
- **VS2022 MSBuild Release x64** — unavailable: Linux environment does not include Windows SDK headers (`Windows.h`) or MSBuild.
- **Native gameplay/reverse offsets** — require in-game testing.
- CHM binary has not been compiled here: HTML Help Compiler / MSBuild are unavailable. CHM project sources and the versionless project filenames have been updated for 5.2.2.

## Windows build
1. Open Visual Studio 2022 Developer Command Prompt on Windows.
2. Unzip complete archive.
3. Run `Build.bat`.
4. Keep all plugin builds and Core on SDK ABI 48 / Orbwalker API v6; do not mix SDK ABI 47 binary plugins.

## v5.2.2_fix additions
- Fixed `ApiTable.cpp` to call the existing `GameAccess::GetPhysicalShield` method.
- Added the appended `RSDK_ObjectApi::GetTeleportState` pointer, public `Object::TeleportState()` with `structSize` guard.
- CHM project filenames are versionless: `EonCrest_RSDK.hhp/.hhc/.hhk`, and generated output is `EonCrest_RSDK.chm`.
- Updated `BuildRelease.bat` to build/verify this CHM and reject docs whose `CHM_VERSION.txt` differs from requested release.

## New expected API examples
```cpp
Object player = ObjectManager::LocalPlayer();
if (player) {
    (void)player.DoEmote(Emotes::Dance);
    const bool zombie = player.IsZombie();
    const bool clone = player.IsClone();
    const float physicalShield = player.PhysicalShield();
    const bool teleporting = player.IsTeleport();
    const char* const teleportState = player.TeleportState(); // Borrowed; copy if retaining.
    (void)teleportState;
}
if (Orbwalker::ActiveMode() == Orbwalker::Mode::Flee) {
    // Champion DLL chooses Q/W/E/R defensive spells.
}
if (Orbwalker::SpellFarmEnabled()) {
    // Champion-specific skill farm in Mixed / LaneClear.
}
void OnNewPath(const NewPathEvent& event) {
    if (event.IsBlink()) { /* bit 0 native flags */ }
}
```
