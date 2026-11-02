#pragma once

#include "RSDKApi.h"

// Plugin system ABI version.
// Legacy dependency fields are kept for source compatibility; Core does not auto-load plugin dependencies.
#define RSDK_PLUGIN_ABI_VERSION 3u
#define RSDK_PLUGIN_ABI_MIN_SUPPORTED 1u

// Packs a Major.Minor.Patch version into a uint32_t.
// Each component is limited to 0..255.
#define RSDK_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major) & 0xFFu) << 16u) | \
     (((uint32_t)(minor) & 0xFFu) << 8u) | \
     ((uint32_t)(patch) & 0xFFu))

#define RSDK_VERSION_MAJOR(version) (((uint32_t)(version) >> 16u) & 0xFFu)
#define RSDK_VERSION_MINOR(version) (((uint32_t)(version) >> 8u) & 0xFFu)
#define RSDK_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFu)

// Legacy dependency metadata. Core ignores this list; plugins/users load supporting DLLs themselves.
typedef struct RSDK_PluginDependency
{
    // Must match the provider plugin's RSDK_PluginInfo::name.
    const char* name;

    // Minimum version, created with RSDK_MAKE_VERSION(...).
    uint32_t minimumVersion;
} RSDK_PluginDependency;

typedef struct RSDK_PluginInfo
{
    uint32_t structSize;
    uint32_t pluginAbiVersion;
    uint32_t requiredSdkAbiVersion;

    const char* name;
    const char* author;
    const char* version;
    const char* description;

    // The fields below were added in Plugin ABI 2.
    // versionNumber lets the SDK compare dependency versions precisely.
    uint32_t versionNumber;
    const RSDK_PluginDependency* dependencies;
    uint32_t dependencyCount;

    // Stable identity for config/localization. Plugin ABI 3+. Display name is not identity.
    // If null/empty, Core falls back to the DLL filename stem.
    const char* persistentId;
} RSDK_PluginInfo;

typedef const RSDK_PluginInfo*(RSDK_CALL* RSDK_PluginQueryFn)(void);
typedef RSDK_Bool(RSDK_CALL* RSDK_PluginLoadFn)(
    const RSDK_Api* api,
    RSDK_PluginState* outputState);
typedef void(RSDK_CALL* RSDK_PluginUnloadFn)(RSDK_PluginState state);

RSDK_PLUGIN_EXPORT const RSDK_PluginInfo* RSDK_CALL RSDK_PluginQuery(void);
RSDK_PLUGIN_EXPORT RSDK_Bool RSDK_CALL RSDK_PluginLoad(
    const RSDK_Api* api,
    RSDK_PluginState* outputState);
RSDK_PLUGIN_EXPORT void RSDK_CALL RSDK_PluginUnload(RSDK_PluginState state);
