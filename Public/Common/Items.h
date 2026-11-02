#pragma once

#include "../RSDK.h"

#include <cstdint>
#include <iterator>
#include <vector>

namespace rsdk::Items
{
    // Inventory slot 0 maps to SpellSlot::_Item1; following item slots are contiguous.
    [[nodiscard]] inline SpellSlot ToSpellSlot(const std::uint8_t inventorySlot) noexcept
    {
        return static_cast<SpellSlot>(static_cast<std::int32_t>(SpellSlot::_Item1) + static_cast<std::int32_t>(inventorySlot));
    }

    namespace detail
    {
        inline constexpr std::uint32_t WardItemIds[]
        {
            2045u, 2049u, 2050u, 2301u, 2302u, 2303u, 3340u, 3361u,
            3362u, 3711u, 1408u, 1409u, 1410u, 1411u, 2043u, 2055u
        };
    }

    // Inventory queries can target any Object; overloads without owner use LocalPlayer.
    [[nodiscard]] inline rsdk::Item FindItem(const std::uint32_t itemId, const Object& owner)
    {
        return owner ? owner.FindItem(itemId) : rsdk::Item{};
    }

    [[nodiscard]] inline rsdk::Item FindItem(const char* const itemName, const Object& owner)
    {
        return owner && itemName != nullptr && *itemName != '\0' ? owner.FindItem(itemName) : rsdk::Item{};
    }

    [[nodiscard]] inline rsdk::Item FindItem(const std::uint32_t itemId)
    {
        return FindItem(itemId, ObjectManager::LocalPlayer());
    }

    [[nodiscard]] inline rsdk::Item FindItem(const char* const itemName)
    {
        return FindItem(itemName, ObjectManager::LocalPlayer());
    }

    [[nodiscard]] inline bool HasItem(const std::uint32_t itemId, const Object& owner)
    {
        return owner && owner.HasItem(itemId);
    }

    [[nodiscard]] inline bool HasItem(const char* const itemName, const Object& owner)
    {
        return owner && itemName != nullptr && *itemName != '\0' && owner.HasItem(itemName);
    }

    [[nodiscard]] inline bool HasItem(const std::uint32_t itemId)
    {
        return HasItem(itemId, ObjectManager::LocalPlayer());
    }

    [[nodiscard]] inline bool HasItem(const char* const itemName)
    {
        return HasItem(itemName, ObjectManager::LocalPlayer());
    }

    [[nodiscard]] inline std::vector<SpellSlot> GetSlots(const std::uint32_t itemId, const Object& owner)
    {
        std::vector<SpellSlot> result{};
        if (!owner)
            return result;

        owner.ForEachItem([&](const rsdk::Item& item)
        {
            if (item.Id() == itemId && item.HasSlot())
                result.push_back(item.ItemSpellSlot());
            return true;
        });
        return result;
    }

    [[nodiscard]] inline std::vector<SpellSlot> GetSlots(const std::uint32_t itemId)
    {
        return GetSlots(itemId, ObjectManager::LocalPlayer());
    }

    // Returns the SpellSlot of the highest-priority ward inventory item regardless of current spell readiness.
    [[nodiscard]] inline SpellSlot GetWardSlot()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return SpellSlot::_SlotUnknown;

        SpellSlot best = SpellSlot::_SlotUnknown;
        std::size_t bestPriority = std::size(detail::WardItemIds);
        player.ForEachItem([&](const rsdk::Item& item)
        {
            if (!item.HasSlot())
                return true;

            const std::uint32_t itemId = item.Id();
            for (std::size_t priority = 0; priority < std::size(detail::WardItemIds); ++priority)
            {
                if (detail::WardItemIds[priority] != itemId || priority >= bestPriority)
                    continue;
                best = item.ItemSpellSlot();
                bestPriority = priority;
                break;
            }
            return bestPriority != 0u;
        });
        return best;
    }

    // Returns the highest-priority usable ward item with one inventory enumeration.
    [[nodiscard]] inline rsdk::Item GetWard()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return {};

        rsdk::Item best{};
        std::size_t bestPriority = std::size(detail::WardItemIds);
        player.ForEachItem([&](const rsdk::Item& item)
        {
            if (!item.HasSlot())
                return true;

            const std::uint32_t itemId = item.Id();
            for (std::size_t priority = 0; priority < std::size(detail::WardItemIds); ++priority)
            {
                if (detail::WardItemIds[priority] != itemId || priority >= bestPriority)
                    continue;
                if (!player.CanUseSpell(item.ItemSpellSlot()))
                    break;

                best = item;
                bestPriority = priority;
                break;
            }
            return bestPriority != 0u;
        });
        return best;
    }

    // Item actions always use LocalPlayer because the SDK does not control another object's spellbook.
    [[nodiscard]] inline bool CanUseItem(const std::uint32_t itemId)
    {
        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemId) : rsdk::Item{};
        return item && item.HasSlot() && player.CanUseSpell(item.ItemSpellSlot());
    }

    [[nodiscard]] inline bool CanUseItem(const char* const itemName)
    {
        if (itemName == nullptr || *itemName == '\0')
            return false;

        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemName) : rsdk::Item{};
        return item && item.HasSlot() && player.CanUseSpell(item.ItemSpellSlot());
    }

    [[nodiscard]] inline bool UseItem(const std::uint32_t itemId)
    {
        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemId) : rsdk::Item{};
        return item && item.HasSlot() && player.Cast(item.ItemSpellSlot());
    }

    [[nodiscard]] inline bool UseItem(const char* const itemName)
    {
        if (itemName == nullptr || *itemName == '\0')
            return false;

        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemName) : rsdk::Item{};
        return item && item.HasSlot() && player.Cast(item.ItemSpellSlot());
    }

    [[nodiscard]] inline bool UseItem(const std::uint32_t itemId, const Object& target)
    {
        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemId) : rsdk::Item{};
        return player && target && item && item.HasSlot() && player.Cast(item.ItemSpellSlot(), target);
    }

    [[nodiscard]] inline bool UseItem(const char* const itemName, const Object& target)
    {
        if (itemName == nullptr || *itemName == '\0')
            return false;

        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemName) : rsdk::Item{};
        return player && target && item && item.HasSlot() && player.Cast(item.ItemSpellSlot(), target);
    }

    [[nodiscard]] inline bool UseItem(const std::uint32_t itemId, const Vec3& position)
    {
        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemId) : rsdk::Item{};
        return player && position.IsValid() && item && item.HasSlot() && player.Cast(item.ItemSpellSlot(), position);
    }

    [[nodiscard]] inline bool UseItem(const char* const itemName, const Vec3& position)
    {
        if (itemName == nullptr || *itemName == '\0' || !position.IsValid())
            return false;

        const Object player = ObjectManager::LocalPlayer();
        const rsdk::Item item = player ? player.FindItem(itemName) : rsdk::Item{};
        return player && item && item.HasSlot() && player.Cast(item.ItemSpellSlot(), position);
    }

    // Small convenience object for repeatedly querying/casting one item id with an optional range.
    class Item final
    {
    public:
        explicit Item(const std::uint32_t id, const float range = 0.0f) noexcept : id_(id), range_(range) {}

        [[nodiscard]] std::uint32_t Id() const noexcept { return id_; }
        [[nodiscard]] float Range() const noexcept { return range_; }
        [[nodiscard]] float RangeSqr() const noexcept { return range_ * range_; }
        void SetRange(const float range) noexcept { range_ = range; }

        [[nodiscard]] std::vector<SpellSlot> Slots() const { return GetSlots(id_); }
        [[nodiscard]] bool IsOwned() const { return HasItem(id_); }
        [[nodiscard]] bool IsOwned(const Object& owner) const { return HasItem(id_, owner); }
        [[nodiscard]] bool IsReady() const { return CanUseItem(id_); }
        [[nodiscard]] bool Cast() const { return UseItem(id_); }
        [[nodiscard]] bool Cast(const Object& target) const { return UseItem(id_, target); }
        [[nodiscard]] bool Cast(const Vec3& position) const { return UseItem(id_, position); }

        [[nodiscard]] bool IsInRange(const Object& target) const noexcept
        {
            return target && IsInRange(target.Position());
        }

        [[nodiscard]] bool IsInRange(const Vec3& position) const noexcept
        {
            if (range_ <= 0.0f)
                return true;

            const Object player = ObjectManager::LocalPlayer();
            return player && player.Position().DistanceToSqr(position) <= RangeSqr();
        }

    private:
        std::uint32_t id_{};
        float range_{};
    };
}
