# EonCrest RSDK

**A C++ Software Development Kit for building EonCrest plugins.**

EonCrest RSDK provides public C++ headers, documentation, examples, and reference gameplay module sources for developing plugins on Windows x64. Prebuilt runtime components and version-specific SDK packages are distributed through GitHub Releases.

[**Download SDK**](https://github.com/nullnamenone-oss/RSDK/releases) · [Getting Started](Docs/Getting_Started.md) · [API Reference](Docs/API_Reference.md) · [Examples](Docs/Examples/GettingStarted) · [Report an Issue](https://github.com/nullnamenone-oss/RSDK/issues)

## Current release

| Component | Version |
| --- | --- |
| RSDK | **5.2.2** |
| Supported game build | **League of Legends CN 16.21.832.5350** |
| Platform | **Windows x64** |
| Development environment | **Visual Studio 2022 (MSVC v143)** |
| C++ standard | **C++17 or newer; C++20 recommended** |

**Release:** [EonCrest RSDK v5.2.2 — LoL CN 16.21.832.5350](https://github.com/nullnamenone-oss/RSDK/releases/tag/v5.2.2-lolcn.16.21.832.5350)

> **Compatibility matters:** RSDK's version and the game's version are independent. A future release may support a different game build while remaining RSDK 5.2.2. Always download the package matching your game build; compatibility with other builds or regions is not guaranteed.

## Getting started

1. Download the appropriate **SDK ZIP from a GitHub Release** and extract it. The automatically generated GitHub *Source code (zip)* archive is not a replacement for the SDK runtime package.
2. Install **Visual Studio 2022** with C++ desktop development tools and select the **x64** configuration.
3. Read [Getting Started](Docs/Getting_Started.md) and [Creating Your First Plugin](Docs/GettingStarted/01_First_Plugin.md).
4. Create a C++ DLL project. Add the extracted SDK's `Public/` directory to your **Additional Include Directories**, and define `RSDK_PLUGIN_BUILD` as shown in the sample project.
5. Build your plugin against the headers from the **same RSDK release** as the runtime you intend to use.
6. Place the resulting DLL in the appropriate `Plugins/` folder of the matching EonCrest runtime and manage it through the Plugin Manager.

See [`Examples/CppBot/`](Examples/CppBot) for a Visual Studio plugin project and [`Docs/Examples/GettingStarted/`](Docs/Examples/GettingStarted) for focused API examples.

## Minimal C++ plugin

```cpp
#define Plugin_Id              "Example.HelloRSDK"
#define Plugin_Name            "Hello RSDK"
#define Author_Name            "YourName"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Minimal RSDK plugin example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>

using namespace rsdk;

void OnGameUpdate()
{
    const Object player = ObjectManager::LocalPlayer();
    if (!player || player.IsDead())
        return;

    // Your plugin logic goes here.
}

class HelloPlugin final : public Plugin
{
public:
    bool OnLoad() override
    {
        Events::OnGameUpdate += OnGameUpdate;
        return true;
    }
};

RSDK_REGISTER_PLUGIN(HelloPlugin);
```

`Plugin_Id` is your plugin's stable identifier. Keep it unchanged across releases to preserve its configuration and localization identity. Prefer framework lifecycle methods such as `OnLoad()` and `OnUnload()` over substantial work in `DllMain`.

## SDK capabilities

| Area | Description |
| --- | --- |
| **Object & Game** | Game objects, heroes, minions, missiles, game state, and positions |
| **Events** | Game updates, rendering, spells, paths, and object lifecycle callbacks |
| **Menu & Localization** | Configurable plugin UI, keybinds, settings, and translated labels |
| **Rendering & Geometry** | Drawing helpers, vectors, shapes, and geometry utilities |
| **Spell & Prediction** | Spell helpers, movement prediction, and collision queries |
| **TargetSelector & Orbwalker** | Target selection, attack/movement control, and health prediction |
| **Damage** | Public damage calculation helpers through the provider module |

Some APIs depend on optional runtime provider DLLs. Check their availability when your plugin needs to work without a particular provider. The [API Reference](Docs/API_Reference.md) documents the current public interfaces and version matrix.

## Repository layout

```text
RSDK/
├── Public/                      Public C++ SDK headers
├── Docs/                        Tutorials, API reference, changelog, CHM
│   ├── Getting_Started.md
│   ├── API_Reference.md
│   ├── CHANGELOG.md
│   └── GettingStarted/          Step-by-step guides
├── Examples/
│   └── CppBot/                  Sample Visual Studio plugin
├── Src_Modules/
│   └── Gameplay/                Reference gameplay module sources
└── README.md
```

`Src_Modules/Gameplay/` contains reference source for modules such as Orbwalker, Prediction, Damage, TargetSelector, Evade, and EzEvade. These source files are **not standalone build projects**. For plugin development, use the `Public/` API rather than private Core implementation headers.

## Documentation

- [Getting Started](Docs/Getting_Started.md) — recommended starting point for new plugin developers.
- [API Reference](Docs/API_Reference.md) — public API contracts, signatures, and provider versions.
- [Changelog](Docs/CHANGELOG.md) — changes introduced across SDK releases.
- [Step-by-step tutorials](Docs/GettingStarted) — events, objects, menus, prediction, rendering, and more.
- [C++ tutorial examples](Docs/Examples/GettingStarted) — standalone source examples for individual features.
- [CppBot sample](Examples/CppBot) — Visual Studio C++ example project.
- `Docs/EonCrest_RSDK.chm` — offline help file distributed with the SDK package.

## Releases and compatibility

Release tags include both the **RSDK version** and **target game build**. For example:

```text
v5.2.2-lolcn.16.21.832.5350
```

This lets game compatibility updates be published without unnecessarily changing the public SDK version. Use the release's **Assets** for the complete SDK ZIP. The repository's `main` branch can be updated independently of an older release tag.

[**Browse all releases**](https://github.com/nullnamenone-oss/RSDK/releases)

## Issues and feedback

Use [GitHub Issues](https://github.com/nullnamenone-oss/RSDK/issues) for API bugs, documentation problems, and feature requests. When reporting a problem, include the RSDK version, exact game build, plugin details, steps to reproduce, and relevant error output. Remove personal or sensitive information from logs before posting.

---

**Disclaimer:** EonCrest RSDK is an independent, third-party development project and is not affiliated with or endorsed by Riot Games. League of Legends and related trademarks belong to their respective owners.
