#include "DebugPrint.h"

#include <cstddef>
#include <cstdint>

namespace userbot::debug
{
    namespace
    {
        const char* BoolText(const bool value) noexcept
        {
            return value ? "true" : "false";
        }

        void PrintVec3(const char* label, const rsdk::Vec3& value)
        {
            rsdk::Log::Info("%s = (%.3f, %.3f, %.3f)", label, value.x, value.y, value.z);
        }

        void PrintVec2(const char* label, const rsdk::Vec2& value)
        {
            rsdk::Log::Info("%s = (%.3f, %.3f)", label, value.x, value.y);
        }

        void PrintHeroOrMinionRow(const char* listName, const std::size_t index, const rsdk::Object& object)
        {
            if (!object)
            {
                rsdk::Log::Info("[%s][%zu] unavailable", listName, index);
                return;
            }

            const rsdk::Vec3 position = object.Position();
            rsdk::Log::Info(
                "[%s][%zu] Address=0x%llX, Id=0x%X, NetworkId=0x%X, TeamId=0x%X, Name=%s, ChampionName=%s, HP=%.3f, MP=%.3f, Pos=(%.3f, %.3f, %.3f)",
                listName, index, static_cast<unsigned long long>(object.Address()), object.Id(), object.NetworkId(), object.Team(), object.Name().c_str(),
                object.ChampionName().c_str(), object.Health(), object.Mana(), position.x, position.y, position.z);
        }

        void PrintBasicObjectRow(const char* listName, const std::size_t index, const rsdk::Object& object)
        {
            if (!object)
            {
                rsdk::Log::Info("[%s][%zu] unavailable", listName, index);
                return;
            }

            const rsdk::Vec3 position = object.Position();
            rsdk::Log::Info(
                "[%s][%zu] Address=0x%llX, Id=0x%X, NetworkId=0x%X, TeamId=0x%X, Name=%s, Pos=(%.3f, %.3f, %.3f)",
                listName, index, static_cast<unsigned long long>(object.Address()), object.Id(), object.NetworkId(), object.Team(), object.Name().c_str(),
                position.x, position.y, position.z);
        }

        void PrintSpellSlot(const rsdk::Object& object, const SpellSlot slot, const char* slotName)
        {
            const std::string spellName = object.GetSpellName(slot);
            rsdk::Log::Info("SpellSlot.%s Name=%s, State=%d, CanUse=%s, Mana=%.3f", slotName, spellName.c_str(),
                static_cast<int>(object.GetSpellState(slot)), BoolText(object.CanUseSpell(slot)), object.ManaSpell(slot));
        }
    }

    void PrintPerks(const rsdk::Object& object)
    {
        if (!object)
            return;

        const auto perks = object.GetAllPerks();
        rsdk::Log::Info("========== Perks (%zu) ==========", perks.size());
        for (std::size_t i = 0; i < perks.size(); ++i)
        {
            const rsdk::Perk& perk = perks[i];
            rsdk::Log::Info("Perk[%zu] Address=0x%llX, Id=%u (0x%X), Name=%s", i, static_cast<unsigned long long>(perk.Address()), perk.Id(), perk.Id(), perk.Name().c_str());
        }
    }

    void PrintBuffs(const rsdk::Object& object)
    {
        if (!object)
            return;

        const auto buffs = object.GetAllBuffs();
        rsdk::Log::Info("========== Buffs (%zu) ==========", buffs.size());
        for (std::size_t i = 0; i < buffs.size(); ++i)
        {
            const rsdk::Buff& buff = buffs[i];
            const rsdk::Object caster = buff.Caster();
            rsdk::Log::Info(
                "Buff[%zu] Address=0x%llX, Type=%d, Name=%s, Begin=%.3f, End=%.3f, Duration=%.3f, Remaining=%.3f, Stacks=%u, Count=%u, Active=%s, Valid=%s, Caster=0x%llX, CasterId=0x%X, CasterNetworkId=0x%X",
                i, static_cast<unsigned long long>(buff.Address()), static_cast<int>(buff.Type()), buff.Name().c_str(), buff.BeginTime(), buff.EndTime(), buff.Duration(),
                buff.RemainingTime(), buff.Stacks(), buff.Count(), BoolText(buff.IsActive()), BoolText(buff.IsValid()),
                static_cast<unsigned long long>(caster.Address()), caster.Id(), caster.NetworkId());
        }
    }

    void PrintItems(const rsdk::Object& object)
    {
        if (!object)
            return;

        const auto items = object.GetAllItems();
        rsdk::Log::Info("========== Items (%zu) ==========", items.size());
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            const rsdk::Item& item = items[i];
            rsdk::Log::Info(
                "Item[%zu] Address=0x%llX, Slot=%u, HasSlot=%s, OwnerNetworkId=0x%X, ItemSpellSlot=%d, Id=%u (0x%X), Stacks=%u, Charges=%u, Name=%s",
                i, static_cast<unsigned long long>(item.Address()), static_cast<unsigned>(item.Slot()), BoolText(item.HasSlot()), item.OwnerNetworkId(),
                static_cast<int>(item.ItemSpellSlot()), item.Id(), item.Id(), item.Stacks(), item.Charges(), item.Name().c_str());
        }
    }

    void PrintSpellCast(const char* const label, const rsdk::SpellCast& spellCast)
    {
        const char* const safeLabel = label != nullptr ? label : "SpellCast";
        if (!spellCast)
        {
            rsdk::Log::Info("%s unavailable", safeLabel);
            return;
        }

        const rsdk::Object caster = spellCast.Caster();
        const rsdk::Object target = spellCast.Target();
        const std::uint32_t missileNetworkId = spellCast.MissileNetworkId();
        const rsdk::Missile missile = missileNetworkId != 0u ? rsdk::ObjectManager::FindMissileByNetworkId(missileNetworkId) : rsdk::Missile{};
        const rsdk::Vec3 startPos = spellCast.StartPos();
        const rsdk::Vec3 endPos = spellCast.EndPos();
        const rsdk::Vec3 castEndPos = spellCast.CastEndPos();

        rsdk::Log::Info(
            "%s Address=0x%llX, SpellName=%s, Caster=0x%llX, Target=0x%llX, Missile=0x%llX, AttackCastDelay=%.3f, AttackDelay=%.3f, Slot=%u, CastTime=%.3f, CastEndTime=%.3f, CooldownExpires=%.3f, IsAutoAttack=%s, ProjectileSpeed=%.3f, Range=%.3f, Radius=%.3f, Width=%.3f",
            safeLabel, static_cast<unsigned long long>(spellCast.Address()), spellCast.SpellName().c_str(), static_cast<unsigned long long>(caster.Address()),
            static_cast<unsigned long long>(target.Address()), static_cast<unsigned long long>(missile.Address()), spellCast.AttackCastDelay(), spellCast.AttackDelay(),
            static_cast<unsigned>(spellCast.Slot()), spellCast.CastTime(), spellCast.CastEndTime(), spellCast.CooldownExpires(), BoolText(spellCast.IsAutoAttack()),
            spellCast.ProjectileSpeed(), spellCast.Range(), spellCast.Radius(), spellCast.Width());
        rsdk::Log::Info("  StartPos=(%.3f, %.3f, %.3f), EndPos=(%.3f, %.3f, %.3f), CastEndPos=(%.3f, %.3f, %.3f)",
            startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z, castEndPos.x, castEndPos.y, castEndPos.z);        
    }

    void PrintSpells(const rsdk::Object& object)
    {
        if (!object)
            return;

        PrintSpellSlot(object, SpellSlot::_Q, "Q");
        PrintSpellSlot(object, SpellSlot::_W, "W");
        PrintSpellSlot(object, SpellSlot::_E, "E");
        PrintSpellSlot(object, SpellSlot::_R, "R");
        PrintSpellSlot(object, SpellSlot::_D, "D");
        PrintSpellSlot(object, SpellSlot::_F, "F");

        const auto spells = object.GetAllSpells();
        rsdk::Log::Info("========== Spells (%zu) ==========", spells.size());
        for (std::size_t i = 0; i < spells.size(); ++i)
        {
            const rsdk::SpellData& spell = spells[i];
            rsdk::Log::Info(
                "Spell[%zu] Address=0x%llX, Name=%s, Level=%u, CooldownExpires=%.3f, CooldownExpires2=%.3f, CooldownTime=%.3f, Ammo=%u, ToggleState=%u, ProjectileSpeed=%.3f, Range=%.3f, Radius=%.3f, Width=%.3f, BaseCooldownTime=%.3f",
                i, static_cast<unsigned long long>(spell.Address()), spell.SpellName().c_str(), static_cast<unsigned>(spell.Level()), spell.CooldownExpires(),
                spell.CooldownExpires2(), spell.CooldownTime(), static_cast<unsigned>(spell.Ammo()), static_cast<unsigned>(spell.ToggleState()), spell.ProjectileSpeed(),
                spell.Range(), spell.Radius(), spell.Width(), spell.BaseCooldownTime());
        }

        PrintSpellCast("BasicAttack", object.BasicAttack());
        PrintSpellCast("ActiveSpellCast", object.GetActiveSpellCast());

        std::uintptr_t spellCastActive = (std::uintptr_t)object.GetActiveSpellCast().Address();
        rsdk::Log::Info(" =============== spellCastActive: %llx", spellCastActive);
        if (spellCastActive)
        {
            std::uintptr_t ptr2 = *reinterpret_cast<const std::uintptr_t*>(spellCastActive + 0);
            if (ptr2)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    std::uintptr_t funcX = *reinterpret_cast<const std::uintptr_t*>(ptr2 + i * 8);
                    rsdk::Log::Info("i = %d -> 0x%x, funcX = %llx, ptr2: %llx", i, i * 8, funcX, ptr2);
                }
            }
        }
    }

    void PrintPlayer()
    {
        const rsdk::Object player = rsdk::ObjectManager::LocalPlayer();
        if (!player)
        {
            rsdk::Log::Warning("LocalPlayer is unavailable.");
            return;
        }

        rsdk::Vec3 boundsMin{};
        rsdk::Vec3 boundsMax{};
        const bool hasBounds = player.GetBounds(boundsMin, boundsMax);
        const rsdk::Vec3 position = player.Position();
        const rsdk::Vec3 direction = player.Direction();
        const rsdk::NavPath navPath = player.GetNavPath();
        const auto currentPath = player.CurrentPath();
        const rsdk::Vec2 healthBar = player.HealthBarScreenPosition();

        rsdk::Log::Info("================ LocalPlayer ================");
        rsdk::Log::Info("Address = 0x%llX", static_cast<unsigned long long>(player.Address()));
        rsdk::Log::Info("Id = 0x%X", player.Id());
        rsdk::Log::Info("NetworkId = 0x%X", player.NetworkId());
        rsdk::Log::Info("TeamId = 0x%X", player.Team());
        rsdk::Log::Info("ObjectName = %s", player.ObjectName().c_str());
        rsdk::Log::Info("Name = %s", player.Name().c_str());
        rsdk::Log::Info("ChampionName = %s", player.ChampionName().c_str());
        rsdk::Log::Info("BaseSkinName = %s", player.BaseSkinName().c_str());
        PrintVec3("Position", position);
        PrintVec3("Direction", direction);
        rsdk::Log::Info("BoundsAvailable = %s", BoolText(hasBounds));
        PrintVec3("BoundsMin", boundsMin);
        PrintVec3("BoundsMax", boundsMax);
        rsdk::Log::Info("BoundingRadius = %.3f", player.BoundingRadius());

        rsdk::Log::Info("Health = %.3f", player.Health());
        rsdk::Log::Info("MaxHealth = %.3f", player.MaxHealth());
        rsdk::Log::Info("HealthPercent = %.3f", player.HealthPercent());
        rsdk::Log::Info("HealthRegenRate = %.3f", player.HealthRegenRate());
        rsdk::Log::Info("Mana = %.3f", player.Mana());
        rsdk::Log::Info("MaxMana = %.3f", player.MaxMana());
        rsdk::Log::Info("ManaPercent = %.3f", player.ManaPercent());
        rsdk::Log::Info("SpecialPoint = %.3f", player.SpecialPoint());
        rsdk::Log::Info("ManaQ = %.3f, ManaW = %.3f, ManaE = %.3f, ManaR = %.3f", player.ManaSpell(SpellSlot::_Q), player.ManaSpell(SpellSlot::_W), player.ManaSpell(SpellSlot::_E), player.ManaSpell(SpellSlot::_R));

        rsdk::Log::Info("AttackRange = %.3f", player.AttackRange());
        rsdk::Log::Info("AttackSpeed = %.3f", player.AttackSpeed());
        rsdk::Log::Info("MovementSpeed = %.3f", player.MovementSpeed());
        rsdk::Log::Info("Crit = %.3f", player.Crit());
        rsdk::Log::Info("AttackDelay = %.3f", player.AttackDelay());
        rsdk::Log::Info("AttackCastDelay = %.3f", player.AttackCastDelay());
        rsdk::Log::Info("ArmorPenetrationPercent = %.3f", player.ArmorPenetrationPercent());
        rsdk::Log::Info("ArmorPenetrationFlat = %.3f", player.ArmorPenetrationFlat());
        rsdk::Log::Info("MagicPenetrationPercent = %.3f", player.MagicPenetrationPercent());
        rsdk::Log::Info("MagicPenetrationFlat = %.3f", player.MagicPenetrationFlat());
        rsdk::Log::Info("Armor = %.3f", player.Armor());
        rsdk::Log::Info("BonusArmor = %.3f", player.BonusArmor());
        rsdk::Log::Info("MagicResist = %.3f", player.MagicResist());
        rsdk::Log::Info("BonusMagicResist = %.3f", player.BonusMagicResist());
        rsdk::Log::Info("BaseAttackDamage = %.3f", player.BaseAttackDamage());
        rsdk::Log::Info("BonusPhysicalDamage = %.3f", player.BonusPhysicalDamage());
        rsdk::Log::Info("PercentPhysicalDamage = %.3f", player.PercentPhysicalDamage());
        rsdk::Log::Info("TotalPhysicalDamage = %.3f", player.TotalPhysicalDamage());
        rsdk::Log::Info("MagicDamage = %.3f", player.MagicDamage());
        rsdk::Log::Info("PercentMagicDamage = %.3f", player.PercentMagicDamage());
        rsdk::Log::Info("TotalMagicDamage = %.3f", player.TotalMagicDamage());
        rsdk::Log::Info("AllShield = %.3f", player.AllShield());
        rsdk::Log::Info("MagicalShield = %.3f", player.MagicalShield());
        rsdk::Log::Info("SpecialHealth = %.3f", player.SpecialHealth());
        rsdk::Log::Info("CooldownReductionPercent = %.3f", player.CooldownReductionPercent());

        rsdk::Log::Info("Level = %u", static_cast<unsigned>(player.Level()));
        rsdk::Log::Info("TargetId = 0x%X", player.TargetId());
        rsdk::Log::Info("TargetableTeamFlags = %u", static_cast<unsigned>(player.TargetableTeamFlags()));
        rsdk::Log::Info("ObjectType = 0x%08X", player.ObjectType());
        rsdk::Log::Info("IsEnemy(Self) = %s", BoolText(player.IsEnemy(player)));
        rsdk::Log::Info("IsInvulnerable = %s", BoolText(player.IsInvulnerable()));
        rsdk::Log::Info("IsTargetable = %s", BoolText(player.IsTargetable()));
        rsdk::Log::Info("IsMelee = %s", BoolText(player.IsMelee()));
        rsdk::Log::Info("IsRanged = %s", BoolText(player.IsRanged()));
        rsdk::Log::Info("IsTeleport = %s", BoolText(player.IsTeleport()));
        rsdk::Log::Info("IsDead = %s", BoolText(player.IsDead()));
        rsdk::Log::Info("IsDeleted = %s", BoolText(player.IsDeleted()));
        rsdk::Log::Info("IsMinion = %s", BoolText(player.IsMinion()));
        rsdk::Log::Info("IsCreep = %s", BoolText(player.IsCreep()));
        rsdk::Log::Info("IsHero = %s", BoolText(player.IsHero()));
        rsdk::Log::Info("IsTurret = %s", BoolText(player.IsTurret()));
        rsdk::Log::Info("IsNexus = %s", BoolText(player.IsNexus()));
        rsdk::Log::Info("IsInhibitor = %s", BoolText(player.IsInhibitor()));
        rsdk::Log::Info("IsMissile = %s", BoolText(player.IsMissile()));
        rsdk::Log::Info("IsVisible = %s", BoolText(player.IsVisible()));
        rsdk::Log::Info("IsOnScreen = %s", BoolText(player.IsOnScreen()));
        rsdk::Log::Info("IsMoving = %s", BoolText(player.IsMoving()));
        rsdk::Log::Info("IsMe = %s", BoolText(player.IsMe()));
        rsdk::Log::Info("IsDashing = %s", BoolText(player.IsDashing()));
        rsdk::Log::Info("IsObjectType(ObjectAI) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::ObjectAI)));
        rsdk::Log::Info("IsObjectType(Minion) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::Minion)));
        rsdk::Log::Info("IsObjectType(Hero) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::Hero)));
        rsdk::Log::Info("IsObjectType(Turret) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::Turret)));
        rsdk::Log::Info("IsObjectType(Missile) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::Missile)));
        rsdk::Log::Info("IsObjectType(Maker) = %s", BoolText(player.IsObjectType(ObjectTypeFlags::Maker)));

        rsdk::Log::Info("NavPath.Available = %s", BoolText(static_cast<bool>(navPath)));
        rsdk::Log::Info("NavPath.Address = 0x%llX", static_cast<unsigned long long>(navPath.Address()));
        PrintVec3("NavPath.StartPos", navPath.StartPos());
        PrintVec3("NavPath.EndPos", navPath.EndPos());
        rsdk::Log::Info("NavPath.IsDashing = %s", BoolText(navPath.IsDashing()));
        rsdk::Log::Info("NavPath.DashSpeed = %.3f", navPath.DashSpeed());
        rsdk::Log::Info("NavPath.DashGravityCount = %u", static_cast<unsigned>(navPath.DashGravityCount()));
        rsdk::Log::Info("NavPath.NavArray = 0x%llX", static_cast<unsigned long long>(navPath.NavArray()));
        rsdk::Log::Info("NavPath.NavArrayCount = %u", navPath.NavArrayCount());
        rsdk::Log::Info("CurrentPath.Count = %zu", currentPath.size());
        for (std::size_t i = 0; i < currentPath.size(); ++i)
            rsdk::Log::Info("CurrentPath[%zu] = (%.3f, %.3f, %.3f)", i, currentPath[i].x, currentPath[i].y, currentPath[i].z);

        rsdk::Log::Info("BaseHealthForLevel = %.3f", player.BaseHealthForLevel());
        rsdk::Log::Info("RespawnTimeRemaining = %.3f", player.RespawnTimeRemaining());
        PrintVec2("HealthBarScreenPosition", healthBar);

        PrintPerks(player);
        PrintBuffs(player);
        PrintItems(player);
        PrintSpells(player);
        rsdk::Log::Info("============== End LocalPlayer ==============");
    }

    void PrintHeroes()
    {
        const auto objects = rsdk::ObjectManager::AllHeroes();
        rsdk::Log::Info("========== Heroes (%zu) ==========", objects.size());
        for (std::size_t i = 0; i < objects.size(); ++i)
            PrintHeroOrMinionRow("Hero", i, objects[i]);
    }

    void PrintMinions()
    {
        const auto objects = rsdk::ObjectManager::AllMinions();
        rsdk::Log::Info("========== Minions (%zu) ==========", objects.size());
        for (std::size_t i = 0; i < objects.size(); ++i)
            PrintHeroOrMinionRow("Minion", i, objects[i]);
    }

    void PrintTurrets()
    {
        const auto objects = rsdk::ObjectManager::AllTurrets();
        rsdk::Log::Info("========== Turrets (%zu) ==========", objects.size());
        for (std::size_t i = 0; i < objects.size(); ++i)
            PrintBasicObjectRow("Turret", i, objects[i]);
    }

    void PrintBuildings()
    {
        const auto objects = rsdk::ObjectManager::AllBuildingObjects();
        rsdk::Log::Info("========== Buildings (%zu) ==========", objects.size());
        for (std::size_t i = 0; i < objects.size(); ++i)
            PrintBasicObjectRow("Building", i, objects[i]);
    }

    void PrintAllObjects()
    {
        const auto objects = rsdk::ObjectManager::AllObjects();
        rsdk::Log::Info("========== All Objects (%zu) ==========", objects.size());
        for (std::size_t i = 0; i < objects.size(); ++i)
            PrintBasicObjectRow("Object", i, objects[i]);
    }
}
