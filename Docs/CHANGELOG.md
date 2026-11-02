# EonCrest / RSDK 5.2.2 — Changelog

**Baseline:** `RSDK_Pro_v5.2.1__build_ok.zip` (user-supplied).  
**SDK ABI:** 48 (previously 47)  
**Plugin ABI:** unchanged  
**Orbwalker Provider / Facade API:** 6 (previously 5)

## v5.2.2_fix corrections
- Fix compile error in `Source/Api/ApiTable.cpp`: `ObjectGetPhysicalShield` delegates to `GameAccess::GetPhysicalShield` (not the non-existent `GameAccess::PhysicalShield`).
- Publish existing `RawGameObject::TeleportState()` as `Object::TeleportState()` through appended `RSDK_ObjectApi::GetTeleportState` (borrowed `const char*`, nullable). `structSize`-guarded extension keeps ABI 48.
- Standardize CHM source and output filenames (`EonCrest_RSDK.hhp/.hhc/.hhk/.chm`) and documentation Markdown/HTML names (no version suffix); document version remains 5.2.2. `BuildRelease.bat` validates CHM version marker and builds versionless CHM.

## Object API (Core and Public)
- Expose `Object::DoEmote(Emotes)`, `Object::IsZombie()`, `Object::IsClone()`, `Object::PhysicalShield()` through new **append-only** `RSDK_ObjectApi` function pointers.
- `DoEmote()` targets the local player; calling on a non-local object returns `false`.
- Rename every code-path occurrence of `IsRecalling` to `IsTeleport`: raw implementation, GameAccess, C ABI API field, C++ `Object` wrapper, and CppBot example.
- The renamed method keeps the pre-existing game-field semantics (`(BYTE)nTeleportStateLength != 0`).

## Orbwalker
- `RSDK_ORBWALKER_MODE_FLEE = 7` / `Orbwalker::Mode::Flee`: move to cursor, no auto attack/target selection. The Flee keybind is **Z (Press)** and outranks other modes.
- `Enable Spell Farming` is a **G (Toggle)** keybind on the Orbwalker menu. It is NOT a new Orbwalker mode; C/V still mean Mixed/LaneClear, normal AA farming is unchanged. Champion code is responsible for spell-specific farm and Flee spell decisions.
- `rsdk::Orbwalker::SpellFarmEnabled()` reads a Core-owned shared value; the setter on the facade is accepted only from the registered Orbwalker provider. Core resets it to `false` when provider unloads.

## Prediction / OnNewPath
- `RSDK_NewPathEvent` now has `uint32_t dashGravityFlags`, copied by the Core OnNewPath hook from 4 bytes at native `a6_dashGravityBuffer`, before invoking the original function.
- C++ `NewPathEvent::dashGravityFlags` and `IsBlink()` use bit 0. The observed low-byte values are `0x03` for Flash/Ezreal E/other blinks, `0x02` for ordinary dash and `0x04` for ordinary movement. Higher bits are raw/undocumented.
- `DashTracker` uses `event.IsBlink()` instead of gravity-count-change heuristics. Existing dash-path and dash-speed processing remain unchanged.

## Previously recorded features already present in baseline
- `Utility::CutPath(Vec2)` / `Utility::CutPath(Vec3)`.
- `Utility::IsFacing()` / `IsBothFacing()`.
- `Spell::SetRange`, `SetWidth`, `SetDelay`, `SetSpeed`, `SetCastThrottle` plus per-call Cast throttle overrides.
- Core-owned per-plugin Unload button, top gold EonCrest title, and Microsoft YaHei glyph atlas for en-US / vi-VN / zh-CN.

## Compatibility and build
- SDK ABI 48 is **required** due to public event/API struct extensions. Rebuild Core and all plugins against `Public/` from this archive; old SDK ABI 47 DLLs will be rejected by the plugin loader.
- Run `Build.bat` on Windows with VS2022/x64 C++ toolchain; it builds `RSDK.sln` in `Release | x64`.
- The CHM source project has been updated for 5.2.2 (`Docs/CHM_Project/`). Run `Docs/CHM_Project/Build_CHM.bat 5.2.2` on Windows with HTML Help Workshop/chmcmd to produce `Docs/EonCrest_RSDK.chm`. `BuildRelease.bat` automatically compiles and verifies the version-validated CHM before packaging the release.
- Linux Clang C++17 syntax checks passed for public API, Orbwalker.cpp, Prediction.cpp, DashTracker.cpp, DamagePassives.cpp, and OrbwalkerService.cpp. The Core DLL could not be compiled in this environment because Windows SDK / VS2022 tools are unavailable. **No claim of a successful MSBuild or in-game test is made.**
