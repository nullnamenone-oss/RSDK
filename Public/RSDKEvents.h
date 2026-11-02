#pragma once

#include "RSDKTypes.h"

typedef void(RSDK_CALL* RSDK_OnTickCallback)(void* userData);
typedef void(RSDK_CALL* RSDK_OnUpdateCallback)(void* userData);
typedef void(RSDK_CALL* RSDK_OnDrawCallback)(void* userData);

typedef struct RSDK_WndProcEvent
{
    uintptr_t hwnd;
    uint32_t message;
    uintptr_t wParam;
    intptr_t lParam;
    RSDK_Bool blockInput;
} RSDK_WndProcEvent;

typedef void(RSDK_CALL* RSDK_OnWndProcCallback)(
    RSDK_WndProcEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnCreateObjectCallback)(
    uintptr_t address,
    uint32_t networkId,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnDeleteObjectCallback)(
    uintptr_t address,
    void* userData);


typedef void(RSDK_CALL* RSDK_OnIssueOrderCallback)(
    RSDK_IssueOrderEvent* eventData,
    void* userData);


typedef void(RSDK_CALL* RSDK_OnNewPathCallback)(
    const RSDK_NewPathEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnProcessSpellCallback)(
    const RSDK_ProcessSpellEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnDoCastCallback)(
    const RSDK_DoCastSpellEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnPlayAnimationCallback)(
    const RSDK_PlayAnimationEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnStopCastCallback)(
    const RSDK_StopCastEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnCastSpellCallback)(
    RSDK_CastSpellEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnUpdateChargeableSpellCallback)(
    RSDK_UpdateChargeableSpellEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnBuffAddRemoveCallback)(
    const RSDK_BuffAddRemoveEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnUpdateBuffCallback)(
    const RSDK_UpdateBuffEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnSpellImpactCallback)(
    const RSDK_SpellImpactEvent* eventData,
    void* userData);

typedef void(RSDK_CALL* RSDK_OnGameUpdateCallback)(void* userData);

