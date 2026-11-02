#pragma once

#include "RSDKTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t RSDK_MenuPageHandle;
typedef uint64_t RSDK_MenuTabHandle;
typedef uint64_t RSDK_MenuSectionHandle;
typedef uint64_t RSDK_MenuGroupHandle;
typedef uint64_t RSDK_MenuItemHandle;

#define RSDK_INVALID_MENU_PAGE ((RSDK_MenuPageHandle)0u)
#define RSDK_INVALID_MENU_TAB ((RSDK_MenuTabHandle)0u)
#define RSDK_INVALID_MENU_SECTION ((RSDK_MenuSectionHandle)0u)
#define RSDK_INVALID_MENU_GROUP ((RSDK_MenuGroupHandle)0u)
#define RSDK_INVALID_MENU_ITEM ((RSDK_MenuItemHandle)0u)

typedef enum RSDK_MenuCategory
{
    RSDK_MENU_CATEGORY_CHAMPION = 0,
    RSDK_MENU_CATEGORY_GAMEPLAY = 1,
    RSDK_MENU_CATEGORY_COMBAT = RSDK_MENU_CATEGORY_GAMEPLAY, /* legacy alias */
    RSDK_MENU_CATEGORY_UTILITY = 2,
    RSDK_MENU_CATEGORY_CORE = 3,
    RSDK_MENU_CATEGORY_DEBUG = 4,
    RSDK_MENU_CATEGORY_MISC = 5
} RSDK_MenuCategory;

typedef enum RSDK_KeyBindType
{
    RSDK_KEYBIND_PRESS = 0,
    RSDK_KEYBIND_TOGGLE = 1
} RSDK_KeyBindType;

typedef struct RSDK_MenuApi
{
    uint32_t structSize;

    RSDK_MenuPageHandle(RSDK_CALL* CreatePage)(const char* id, const char* title, uint8_t category);
    RSDK_Bool(RSDK_CALL* RemovePage)(RSDK_MenuPageHandle page);
    RSDK_MenuTabHandle(RSDK_CALL* AddTab)(RSDK_MenuPageHandle page, const char* id, const char* title);
    RSDK_MenuSectionHandle(RSDK_CALL* AddSection)(RSDK_MenuTabHandle tab, const char* id, const char* title);
    RSDK_MenuGroupHandle(RSDK_CALL* AddGroup)(RSDK_MenuSectionHandle section, const char* id, const char* title, RSDK_Bool defaultOpen);
    RSDK_MenuGroupHandle(RSDK_CALL* AddSubGroup)(RSDK_MenuGroupHandle parent, const char* id, const char* title, RSDK_Bool defaultOpen);

    RSDK_MenuItemHandle(RSDK_CALL* AddBool)(RSDK_MenuSectionHandle section, const char* id, const char* label, RSDK_Bool defaultValue);
    RSDK_MenuItemHandle(RSDK_CALL* AddSliderInt)(RSDK_MenuSectionHandle section, const char* id, const char* label, int32_t defaultValue, int32_t minimum, int32_t maximum);
    RSDK_MenuItemHandle(RSDK_CALL* AddSliderFloat)(RSDK_MenuSectionHandle section, const char* id, const char* label, float defaultValue, float minimum, float maximum);
    RSDK_MenuItemHandle(RSDK_CALL* AddCombo)(RSDK_MenuSectionHandle section, const char* id, const char* label, const char* const* options, uint32_t optionCount, int32_t defaultIndex);
    RSDK_MenuItemHandle(RSDK_CALL* AddKeyBind)(RSDK_MenuSectionHandle section, const char* id, const char* label, int32_t defaultVirtualKey, uint8_t defaultType);
    RSDK_MenuItemHandle(RSDK_CALL* AddColor)(RSDK_MenuSectionHandle section, const char* id, const char* label, RSDK_Color defaultValue);
    RSDK_MenuItemHandle(RSDK_CALL* AddButton)(RSDK_MenuSectionHandle section, const char* id, const char* label);
    RSDK_MenuItemHandle(RSDK_CALL* AddText)(RSDK_MenuSectionHandle section, const char* id, const char* text);
    RSDK_MenuItemHandle(RSDK_CALL* AddTextColor)(RSDK_MenuSectionHandle section, const char* id, const char* text, RSDK_Color color);
    RSDK_MenuItemHandle(RSDK_CALL* AddSeparator)(RSDK_MenuSectionHandle section, const char* id);

    RSDK_MenuItemHandle(RSDK_CALL* AddGroupBool)(RSDK_MenuGroupHandle group, const char* id, const char* label, RSDK_Bool defaultValue);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupSliderInt)(RSDK_MenuGroupHandle group, const char* id, const char* label, int32_t defaultValue, int32_t minimum, int32_t maximum);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupSliderFloat)(RSDK_MenuGroupHandle group, const char* id, const char* label, float defaultValue, float minimum, float maximum);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupCombo)(RSDK_MenuGroupHandle group, const char* id, const char* label, const char* const* options, uint32_t optionCount, int32_t defaultIndex);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupKeyBind)(RSDK_MenuGroupHandle group, const char* id, const char* label, int32_t defaultVirtualKey, uint8_t defaultType);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupColor)(RSDK_MenuGroupHandle group, const char* id, const char* label, RSDK_Color defaultValue);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupButton)(RSDK_MenuGroupHandle group, const char* id, const char* label);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupText)(RSDK_MenuGroupHandle group, const char* id, const char* text);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupTextColor)(RSDK_MenuGroupHandle group, const char* id, const char* text, RSDK_Color color);
    RSDK_MenuItemHandle(RSDK_CALL* AddGroupSeparator)(RSDK_MenuGroupHandle group, const char* id);

    RSDK_Bool(RSDK_CALL* GetBool)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* SetBool)(RSDK_MenuItemHandle item, RSDK_Bool value);
    int32_t(RSDK_CALL* GetInt)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* SetInt)(RSDK_MenuItemHandle item, int32_t value);
    float(RSDK_CALL* GetFloat)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* SetFloat)(RSDK_MenuItemHandle item, float value);
    RSDK_Bool(RSDK_CALL* GetColor)(RSDK_MenuItemHandle item, RSDK_Color* output);
    RSDK_Bool(RSDK_CALL* SetColor)(RSDK_MenuItemHandle item, RSDK_Color value);
    uint8_t(RSDK_CALL* GetKeyBindType)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* IsKeyBindActive)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* ConsumePressed)(RSDK_MenuItemHandle item);
    RSDK_Bool(RSDK_CALL* SetTooltip)(RSDK_MenuItemHandle item, const char* tooltip);
    RSDK_Bool(RSDK_CALL* SetSameLine)(RSDK_MenuItemHandle item, RSDK_Bool enabled, float spacing);
    RSDK_Bool(RSDK_CALL* SetGroupTooltip)(RSDK_MenuGroupHandle group, const char* tooltip);
    RSDK_Bool(RSDK_CALL* SetPersistent)(RSDK_MenuItemHandle item, RSDK_Bool persistent);

    RSDK_Bool(RSDK_CALL* IsOpen)(void);
    void(RSDK_CALL* SetOpen)(RSDK_Bool open);
    void(RSDK_CALL* Toggle)(void);
} RSDK_MenuApi;

#ifdef __cplusplus
}
#endif
