#pragma once

#include "RSDKMenu.h"
#include "RSDKRuntime.hpp"
#include "Common/Vector.h"
#include "Common/Color.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rsdk
{
    enum class MenuCategory : std::uint8_t
    {
        Champion = RSDK_MENU_CATEGORY_CHAMPION,
        Gameplay = RSDK_MENU_CATEGORY_GAMEPLAY,
        Combat = Gameplay, // legacy alias
        Utility = RSDK_MENU_CATEGORY_UTILITY,
        Core = RSDK_MENU_CATEGORY_CORE,
        Debug = RSDK_MENU_CATEGORY_DEBUG,
        Misc = RSDK_MENU_CATEGORY_MISC
    };

    enum class KeyBindType : std::uint8_t
    {
        Press = RSDK_KEYBIND_PRESS,
        Toggle = RSDK_KEYBIND_TOGGLE
    };

    class MenuItem
    {
    public:
        constexpr MenuItem() noexcept = default;
        explicit constexpr MenuItem(const RSDK_MenuItemHandle handle) noexcept : handle_(handle) {}

        [[nodiscard]] constexpr RSDK_MenuItemHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_MENU_ITEM; }

        bool SetTooltip(const char* tooltip) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetTooltip != nullptr && api->SetTooltip(handle_, tooltip) != RSDK_FALSE;
        }

        bool SetSameLine(const bool enabled = true, const float spacing = -1.0f) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetSameLine != nullptr && api->SetSameLine(handle_, enabled ? RSDK_TRUE : RSDK_FALSE, spacing) != RSDK_FALSE;
        }

        bool SetPersistent(const bool persistent) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetPersistent != nullptr && api->SetPersistent(handle_, persistent ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
        }

    protected:
        [[nodiscard]] const RSDK_MenuApi* Api() const noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->menu : nullptr;
        }

        RSDK_MenuItemHandle handle_{};
    };

    class MenuBool final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] bool Value() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetBool != nullptr && api->GetBool(handle_) != RSDK_FALSE;
        }

        bool SetValue(const bool value) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetBool != nullptr && api->SetBool(handle_, value ? RSDK_TRUE : RSDK_FALSE) != RSDK_FALSE;
        }
    };

    class MenuInt final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] std::int32_t Value() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetInt != nullptr ? api->GetInt(handle_) : 0;
        }

        bool SetValue(const std::int32_t value) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetInt != nullptr && api->SetInt(handle_, value) != RSDK_FALSE;
        }
    };

    using MenuSliderInt = MenuInt;
    using MenuCombo = MenuInt;

    class MenuKeyBind final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] std::int32_t Key() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetInt != nullptr ? api->GetInt(handle_) : 0;
        }

        [[nodiscard]] std::int32_t Value() const noexcept { return Key(); }

        bool SetKey(const std::int32_t virtualKey) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetInt != nullptr && api->SetInt(handle_, virtualKey) != RSDK_FALSE;
        }

        bool SetValue(const std::int32_t virtualKey) const noexcept { return SetKey(virtualKey); }

        [[nodiscard]] KeyBindType Type() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetKeyBindType != nullptr
                ? static_cast<KeyBindType>(api->GetKeyBindType(handle_)) : KeyBindType::Press;
        }

        [[nodiscard]] bool Active() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->IsKeyBindActive != nullptr && api->IsKeyBindActive(handle_) != RSDK_FALSE;
        }
    };

    class MenuFloat final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] float Value() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->GetFloat != nullptr ? api->GetFloat(handle_) : 0.0f;
        }

        bool SetValue(const float value) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetFloat != nullptr && api->SetFloat(handle_, value) != RSDK_FALSE;
        }
    };

    using MenuSliderFloat = MenuFloat;

    class MenuColor final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] Color Value() const noexcept
        {
            RSDK_Color value{255, 255, 255, 255};
            const auto* const api = Api();
            if (api != nullptr && api->GetColor != nullptr)
                api->GetColor(handle_, &value);
            return Color(value);
        }

        bool SetValue(const Color value) const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->SetColor != nullptr && api->SetColor(handle_, value) != RSDK_FALSE;
        }
    };

    class MenuButton final : public MenuItem
    {
    public:
        using MenuItem::MenuItem;

        [[nodiscard]] bool ConsumePressed() const noexcept
        {
            const auto* const api = Api();
            return api != nullptr && api->ConsumePressed != nullptr && api->ConsumePressed(handle_) != RSDK_FALSE;
        }
    };

    template<typename T>
    class MenuLinkCollection final
    {
    public:
        void Add(std::string key, const T item) { items_.insert_or_assign(std::move(key), item); }
        void Remove(const std::string_view key) { items_.erase(std::string(key)); }
        void Clear() noexcept { items_.clear(); }
        [[nodiscard]] bool Contains(const std::string_view key) const { return items_.find(std::string(key)) != items_.end(); }

        [[nodiscard]] T operator[](const std::string_view key) const noexcept
        {
            try
            {
                const auto iterator = items_.find(std::string(key));
                return iterator != items_.end() ? iterator->second : T{};
            }
            catch (...)
            {
                return T{};
            }
        }

    private:
        std::unordered_map<std::string, T> items_;
    };

    using MenuBoolLinks = MenuLinkCollection<MenuBool>;
    using MenuComboLinks = MenuLinkCollection<MenuCombo>;
    using MenuSliderIntLinks = MenuLinkCollection<MenuSliderInt>;
    using MenuSliderFloatLinks = MenuLinkCollection<MenuSliderFloat>;
    using MenuKeyBindLinks = MenuLinkCollection<MenuKeyBind>;
    using MenuColorLinks = MenuLinkCollection<MenuColor>;

    struct MenuLinks final
    {
        MenuBoolLinks Bools;
        MenuComboLinks Combos;
        MenuSliderIntLinks SliderInts;
        MenuSliderFloatLinks SliderFloats;
        MenuKeyBindLinks KeyBinds;
        MenuColorLinks Colors;
    };


    class MenuGroup final
    {
    public:
        constexpr MenuGroup() noexcept = default;
        explicit constexpr MenuGroup(const RSDK_MenuGroupHandle handle) noexcept : handle_(handle) {}

        [[nodiscard]] constexpr RSDK_MenuGroupHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_MENU_GROUP; }

        bool SetTooltip(const char* tooltip) const noexcept
        {
            const auto* const api = MenuApi();
            return api != nullptr && api->SetGroupTooltip != nullptr && api->SetGroupTooltip(handle_, tooltip) != RSDK_FALSE;
        }

        [[nodiscard]] MenuGroup AddGroup(const char* id, const char* title, const bool defaultOpen = true) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuGroup(api != nullptr && api->AddSubGroup != nullptr ? api->AddSubGroup(handle_, id, title, defaultOpen ? RSDK_TRUE : RSDK_FALSE) : RSDK_INVALID_MENU_GROUP);
        }

        [[nodiscard]] MenuBool AddBool(const char* id, const char* label, const bool defaultValue = false) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuBool(api != nullptr && api->AddGroupBool != nullptr ? api->AddGroupBool(handle_, id, label, defaultValue ? RSDK_TRUE : RSDK_FALSE) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuSliderInt AddSliderInt(const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSliderInt(api != nullptr && api->AddGroupSliderInt != nullptr ? api->AddGroupSliderInt(handle_, id, label, defaultValue, minimum, maximum) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuSliderFloat AddSliderFloat(const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSliderFloat(api != nullptr && api->AddGroupSliderFloat != nullptr ? api->AddGroupSliderFloat(handle_, id, label, defaultValue, minimum, maximum) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::vector<const char*>& options, const std::int32_t defaultIndex = 0) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuCombo(api != nullptr && api->AddGroupCombo != nullptr ? api->AddGroupCombo(handle_, id, label, options.data(), static_cast<std::uint32_t>(options.size()), defaultIndex) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            const std::vector<const char*> values(options.begin(), options.end());
            return AddCombo(id, label, values, defaultIndex);
        }

        [[nodiscard]] MenuKeyBind AddKeyBind(const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuKeyBind(api != nullptr && api->AddGroupKeyBind != nullptr ? api->AddGroupKeyBind(handle_, id, label, defaultVirtualKey, static_cast<std::uint8_t>(type)) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuColor AddColor(const char* id, const char* label, const Color defaultValue = Color()) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuColor(api != nullptr && api->AddGroupColor != nullptr ? api->AddGroupColor(handle_, id, label, defaultValue) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuButton AddButton(const char* id, const char* label) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuButton(api != nullptr && api->AddGroupButton != nullptr ? api->AddGroupButton(handle_, id, label) : RSDK_INVALID_MENU_ITEM);
        }

        void AddText(const char* id, const char* text) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddGroupText != nullptr)
                api->AddGroupText(handle_, id, text);
        }

        void AddText(const char* id, const char* text, const Color color) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddGroupTextColor != nullptr)
                api->AddGroupTextColor(handle_, id, text, color);
        }

        void AddSeparator(const char* id) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddGroupSeparator != nullptr)
                api->AddGroupSeparator(handle_, id);
        }

        void AddSeparator() const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddGroupSeparator != nullptr)
                api->AddGroupSeparator(handle_, nullptr);
        }

        MenuBool AddLinkedBool(MenuBoolLinks& links, const char* id, const char* label, const bool defaultValue = false) const
        {
            const MenuBool item = AddBool(id, label, defaultValue);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuCombo AddLinkedCombo(MenuComboLinks& links, const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            const MenuCombo item = AddCombo(id, label, options, defaultIndex);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuSliderInt AddLinkedSliderInt(MenuSliderIntLinks& links, const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const
        {
            const MenuSliderInt item = AddSliderInt(id, label, defaultValue, minimum, maximum);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuSliderFloat AddLinkedSliderFloat(MenuSliderFloatLinks& links, const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const
        {
            const MenuSliderFloat item = AddSliderFloat(id, label, defaultValue, minimum, maximum);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuKeyBind AddLinkedKeyBind(MenuKeyBindLinks& links, const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const
        {
            const MenuKeyBind item = AddKeyBind(id, label, defaultVirtualKey, type);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuColor AddLinkedColor(MenuColorLinks& links, const char* id, const char* label, const Color defaultValue = Color()) const
        {
            const MenuColor item = AddColor(id, label, defaultValue);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

    private:
        [[nodiscard]] static const RSDK_MenuApi* MenuApi() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->menu : nullptr;
        }

        RSDK_MenuGroupHandle handle_{};
    };

    class MenuSection final
    {
    public:
        constexpr MenuSection() noexcept = default;
        explicit constexpr MenuSection(const RSDK_MenuSectionHandle handle) noexcept : handle_(handle) {}

        [[nodiscard]] constexpr RSDK_MenuSectionHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_MENU_SECTION; }

        [[nodiscard]] MenuGroup AddGroup(const char* id, const char* title, const bool defaultOpen = true) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuGroup(api != nullptr && api->AddGroup != nullptr ? api->AddGroup(handle_, id, title, defaultOpen ? RSDK_TRUE : RSDK_FALSE) : RSDK_INVALID_MENU_GROUP);
        }

        [[nodiscard]] MenuBool AddBool(const char* id, const char* label, const bool defaultValue = false) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuBool(api != nullptr && api->AddBool != nullptr ? api->AddBool(handle_, id, label, defaultValue ? RSDK_TRUE : RSDK_FALSE) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuSliderInt AddSliderInt(const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSliderInt(api != nullptr && api->AddSliderInt != nullptr ? api->AddSliderInt(handle_, id, label, defaultValue, minimum, maximum) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuSliderFloat AddSliderFloat(const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSliderFloat(api != nullptr && api->AddSliderFloat != nullptr ? api->AddSliderFloat(handle_, id, label, defaultValue, minimum, maximum) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::vector<const char*>& options, const std::int32_t defaultIndex = 0) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuCombo(api != nullptr && api->AddCombo != nullptr ? api->AddCombo(handle_, id, label, options.data(), static_cast<std::uint32_t>(options.size()), defaultIndex) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            const std::vector<const char*> values(options.begin(), options.end());
            return AddCombo(id, label, values, defaultIndex);
        }

        [[nodiscard]] MenuKeyBind AddKeyBind(const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuKeyBind(api != nullptr && api->AddKeyBind != nullptr
                ? api->AddKeyBind(handle_, id, label, defaultVirtualKey, static_cast<std::uint8_t>(type)) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuColor AddColor(const char* id, const char* label, const Color defaultValue = Color()) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuColor(api != nullptr && api->AddColor != nullptr ? api->AddColor(handle_, id, label, defaultValue) : RSDK_INVALID_MENU_ITEM);
        }

        [[nodiscard]] MenuButton AddButton(const char* id, const char* label) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuButton(api != nullptr && api->AddButton != nullptr ? api->AddButton(handle_, id, label) : RSDK_INVALID_MENU_ITEM);
        }

        void AddText(const char* id, const char* text) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddText != nullptr)
                api->AddText(handle_, id, text);
        }

        void AddText(const char* id, const char* text, const Color color) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddTextColor != nullptr)
                api->AddTextColor(handle_, id, text, color);
        }

        void AddSeparator(const char* id) const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddSeparator != nullptr)
                api->AddSeparator(handle_, id);
        }

        void AddSeparator() const noexcept
        {
            const auto* const api = MenuApi();
            if (api != nullptr && api->AddSeparator != nullptr)
                api->AddSeparator(handle_, nullptr);
        }

        MenuBool AddLinkedBool(MenuBoolLinks& links, const char* id, const char* label, const bool defaultValue = false) const
        {
            const MenuBool item = AddBool(id, label, defaultValue);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuCombo AddLinkedCombo(MenuComboLinks& links, const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            const MenuCombo item = AddCombo(id, label, options, defaultIndex);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuSliderInt AddLinkedSliderInt(MenuSliderIntLinks& links, const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const
        {
            const MenuSliderInt item = AddSliderInt(id, label, defaultValue, minimum, maximum);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuSliderFloat AddLinkedSliderFloat(MenuSliderFloatLinks& links, const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const
        {
            const MenuSliderFloat item = AddSliderFloat(id, label, defaultValue, minimum, maximum);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuKeyBind AddLinkedKeyBind(MenuKeyBindLinks& links, const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const
        {
            const MenuKeyBind item = AddKeyBind(id, label, defaultVirtualKey, type);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

        MenuColor AddLinkedColor(MenuColorLinks& links, const char* id, const char* label, const Color defaultValue = Color()) const
        {
            const MenuColor item = AddColor(id, label, defaultValue);
            links.Add(id != nullptr ? id : "", item);
            return item;
        }

    private:
        [[nodiscard]] static const RSDK_MenuApi* MenuApi() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->menu : nullptr;
        }

        RSDK_MenuSectionHandle handle_{};
    };

    class MenuTab final
    {
    public:
        constexpr MenuTab() noexcept = default;
        explicit constexpr MenuTab(const RSDK_MenuTabHandle handle) noexcept : handle_(handle) {}

        [[nodiscard]] constexpr RSDK_MenuTabHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_MENU_TAB; }

        [[nodiscard]] MenuSection AddSection(const char* id, const char* title) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSection(api != nullptr && api->AddSection != nullptr ? api->AddSection(handle_, id, title) : RSDK_INVALID_MENU_SECTION);
        }

        [[nodiscard]] MenuGroup AddGroup(const char* id, const char* title, const bool defaultOpen = true) const noexcept
        {
            return RootSection().AddGroup(id, title, defaultOpen);
        }

        [[nodiscard]] MenuBool AddBool(const char* id, const char* label, const bool defaultValue = false) const noexcept
        {
            return RootSection().AddBool(id, label, defaultValue);
        }

        [[nodiscard]] MenuSliderInt AddSliderInt(const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const noexcept
        {
            return RootSection().AddSliderInt(id, label, defaultValue, minimum, maximum);
        }

        [[nodiscard]] MenuSliderFloat AddSliderFloat(const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const noexcept
        {
            return RootSection().AddSliderFloat(id, label, defaultValue, minimum, maximum);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::vector<const char*>& options, const std::int32_t defaultIndex = 0) const noexcept
        {
            return RootSection().AddCombo(id, label, options, defaultIndex);
        }

        [[nodiscard]] MenuCombo AddCombo(const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            return RootSection().AddCombo(id, label, options, defaultIndex);
        }

        [[nodiscard]] MenuKeyBind AddKeyBind(const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const noexcept
        {
            return RootSection().AddKeyBind(id, label, defaultVirtualKey, type);
        }

        [[nodiscard]] MenuColor AddColor(const char* id, const char* label, const Color defaultValue = Color()) const noexcept
        {
            return RootSection().AddColor(id, label, defaultValue);
        }

        [[nodiscard]] MenuButton AddButton(const char* id, const char* label) const noexcept
        {
            return RootSection().AddButton(id, label);
        }

        void AddText(const char* id, const char* text) const noexcept
        {
            RootSection().AddText(id, text);
        }

        void AddText(const char* id, const char* text, const Color color) const noexcept
        {
            RootSection().AddText(id, text, color);
        }

        void AddSeparator(const char* id) const noexcept
        {
            RootSection().AddSeparator(id);
        }

        void AddSeparator() const noexcept
        {
            RootSection().AddSeparator();
        }

        MenuBool AddLinkedBool(MenuBoolLinks& links, const char* id, const char* label, const bool defaultValue = false) const
        {
            return RootSection().AddLinkedBool(links, id, label, defaultValue);
        }

        MenuCombo AddLinkedCombo(MenuComboLinks& links, const char* id, const char* label, const std::initializer_list<const char*> options, const std::int32_t defaultIndex = 0) const
        {
            return RootSection().AddLinkedCombo(links, id, label, options, defaultIndex);
        }

        MenuSliderInt AddLinkedSliderInt(MenuSliderIntLinks& links, const char* id, const char* label, const std::int32_t defaultValue, const std::int32_t minimum, const std::int32_t maximum) const
        {
            return RootSection().AddLinkedSliderInt(links, id, label, defaultValue, minimum, maximum);
        }

        MenuSliderFloat AddLinkedSliderFloat(MenuSliderFloatLinks& links, const char* id, const char* label, const float defaultValue, const float minimum, const float maximum) const
        {
            return RootSection().AddLinkedSliderFloat(links, id, label, defaultValue, minimum, maximum);
        }

        MenuKeyBind AddLinkedKeyBind(MenuKeyBindLinks& links, const char* id, const char* label, const std::int32_t defaultVirtualKey, const KeyBindType type = KeyBindType::Press) const
        {
            return RootSection().AddLinkedKeyBind(links, id, label, defaultVirtualKey, type);
        }

        MenuColor AddLinkedColor(MenuColorLinks& links, const char* id, const char* label, const Color defaultValue = Color()) const
        {
            return RootSection().AddLinkedColor(links, id, label, defaultValue);
        }

    private:
        [[nodiscard]] MenuSection RootSection() const noexcept
        {
            const auto* const api = MenuApi();
            return MenuSection(api != nullptr && api->AddSection != nullptr
                ? api->AddSection(handle_, "__rsdk_root", "")
                : RSDK_INVALID_MENU_SECTION);
        }

        [[nodiscard]] static const RSDK_MenuApi* MenuApi() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->menu : nullptr;
        }

        RSDK_MenuTabHandle handle_{};
    };

    class MenuPage final
    {
    public:
        constexpr MenuPage() noexcept = default;
        explicit constexpr MenuPage(const RSDK_MenuPageHandle handle) noexcept : handle_(handle) {}

        [[nodiscard]] constexpr RSDK_MenuPageHandle Handle() const noexcept { return handle_; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return handle_ != RSDK_INVALID_MENU_PAGE; }

        [[nodiscard]] MenuTab AddTab(const char* id, const char* title) const noexcept
        {
            const auto* const api = MenuApi();
            return MenuTab(api != nullptr && api->AddTab != nullptr ? api->AddTab(handle_, id, title) : RSDK_INVALID_MENU_TAB);
        }

        bool Remove() const noexcept
        {
            const auto* const api = MenuApi();
            return api != nullptr && api->RemovePage != nullptr && api->RemovePage(handle_) != RSDK_FALSE;
        }

    private:
        [[nodiscard]] static const RSDK_MenuApi* MenuApi() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr ? api->menu : nullptr;
        }

        RSDK_MenuPageHandle handle_{};
    };

    namespace Menu
    {
        [[nodiscard]] inline MenuPage CreatePage(const char* id, const char* title, const MenuCategory category) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api == nullptr || api->menu == nullptr || api->menu->CreatePage == nullptr)
                return {};
            return MenuPage(api->menu->CreatePage(id, title, static_cast<std::uint8_t>(category)));
        }

        [[nodiscard]] inline bool IsOpen() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            return api != nullptr && api->menu != nullptr && api->menu->IsOpen != nullptr && api->menu->IsOpen() != RSDK_FALSE;
        }

        inline void SetOpen(const bool open) noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->menu != nullptr && api->menu->SetOpen != nullptr)
                api->menu->SetOpen(open ? RSDK_TRUE : RSDK_FALSE);
        }

        inline void Toggle() noexcept
        {
            const auto* const api = detail::Runtime::Instance().Api();
            if (api != nullptr && api->menu != nullptr && api->menu->Toggle != nullptr)
                api->menu->Toggle();
        }
    }
}
