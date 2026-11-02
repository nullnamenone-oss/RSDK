#define Plugin_Id              "Guide.ItemsBuffsUtility"
#define Plugin_Name            "Getting Started - Items, Buffs and Utility"
#define Author_Name            "EonCrest"
#define Plugin_Version         "1.0.0"
#define Plugin_Description     "Items, Buffs and Utility API example"
#define Plugin_Version_Number  RSDK_MAKE_VERSION(1, 0, 0)

#include <RSDK.h>
#include <Common/Items.h>
#include <Common/Utility.h>

#include <vector>

using namespace rsdk;

namespace guide
{
    float g_nextLogTime{};

    void TryUseItem(const std::uint32_t itemId, const Object& target)
    {
        if (!target)
            return;

        if (!Items::HasItem(itemId))
        {
            Log::Info("[Items] itemId=%u is not in the local inventory.", itemId);
            return;
        }

        if (!Items::CanUseItem(itemId))
        {
            Log::Info("[Items] itemId=%u is present but not ready.", itemId);
            return;
        }

        const bool used = Items::UseItem(itemId, target);
        Log::Info("[Items] UseItem id=%u target=%u result=%d", itemId, target.NetworkId(), used ? 1 : 0);
    }

    void OnGameUpdate()
    {
        const Object player = ObjectManager::LocalPlayer();
        if (!player)
            return;

        const float now = Game::TimeGame();
        if (now < g_nextLogTime)
            return;
        g_nextLogTime = now + 2.0f;

        const std::vector<Buff> buffs = player.GetAllBuffs();
        const std::vector<Item> items = player.GetAllItems();
        Log::Info("[Utility] player=%s buffs=%zu items=%zu stunned=%d",
            player.ChampionName().c_str(), buffs.size(), items.size(), player.HasBuff(BuffType::Stun) ? 1 : 0);

        for (const Item& item : items)
        {
            if (!item)
                continue;
            Log::Info("[Items] id=%u name=%s slot=%u stacks=%u charges=%u",
                item.Id(), item.Name().c_str(), item.Slot(), item.Stacks(), item.Charges());
        }

        const std::vector<Object> enemies = ObjectManager::AllHeroes(TeamFilter::Enemy);
        const Object target = enemies.empty() ? Object{} : enemies.front();
        if (Utility::IsValidTarget(target, 800.0f))
        {
            const float mitigated = Utility::CalculateDamage(player, target, DamageType::Magical, 100.0f);
            Log::Info("[Utility] 100 raw magical -> %.1f against %s", mitigated, target.ChampionName().c_str());
        }

        // Pass a real item id from your own logic when you want to use an item:
        // TryUseItem(itemId, target);
    }
}

class ItemsBuffsUtilityExample final : public Plugin
{
public:
    bool OnLoad() override
    {
        Events::OnGameUpdate += guide::OnGameUpdate;
        Log::Info("[Items] Example loaded; inventory/buff status is printed every 2 seconds.");
        return true;
    }
};

RSDK_REGISTER_PLUGIN(ItemsBuffsUtilityExample);
