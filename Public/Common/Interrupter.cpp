#include "Interrupter.h"
#include "Utility.h"
#include <algorithm>
#include <cctype>
#include <limits>
#include <unordered_map>
#include <utility>

namespace rsdk
{
    namespace
    {
        std::unordered_map<std::uint32_t, LastCastedSpellEntry> CastedSpells{};
        std::optional<LastCastPacketSentEntry> lastCastPacketSent{};
        std::vector<std::pair<std::uint64_t, OnPossibleToInterruptH>> onPossibleToInterruptHandlers{};
        std::uint64_t nextInterrupterHandlerId{1};

        // SOURCE L#: StringComparison.CurrentCultureIgnoreCase.
        // RSDK-CHANGE: game spell identifiers use ASCII case folding; no locale-dependent matching.
        [[nodiscard]] bool EqualsIgnoreCase(const std::string& lhs, const std::string& rhs)
        {
            return lhs.size() == rhs.size() &&
                std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](unsigned char a, unsigned char b)
                {
                    return std::tolower(a) == std::tolower(b);
                });
        }
    }

    std::optional<LastCastedSpellEntry> LastCastedSpell::LastCastedspell(const Object& unit)
    {
        if (!unit)
            return std::nullopt;
        const auto entry = CastedSpells.find(unit.NetworkId());
        return entry != CastedSpells.end() ? std::optional<LastCastedSpellEntry>{entry->second} : std::nullopt;
    }

    std::string LastCastedSpell::LastCastedSpellName(const Object& unit)
    {
        const auto entry = LastCastedspell(unit);
        return entry ? entry->Name : std::string{};
    }

    float LastCastedSpell::LastCastedSpellT(const Object& unit)
    {
        const auto entry = LastCastedspell(unit);
        return entry ? entry->Tick : (Game::TimeGame() > 0.0f ? 0.0f : -std::numeric_limits<float>::max());
    }

    Object LastCastedSpell::LastCastedSpellTarget(const Object& unit)
    {
        const auto entry = LastCastedspell(unit);
        return entry ? entry->Target : Object{};
    }

    std::optional<LastCastPacketSentEntry> LastCastedSpell::LastCastPacketSent()
    {
        return lastCastPacketSent;
    }

    void LastCastedSpell::Obj_AI_Hero_OnProcessSpellCast(const ProcessSpellEvent& args)
    {
        // SOURCE L#: only Obj_AI_Hero; cache indexed by sender.NetworkId.
        if (args.sender && args.sender.IsHero() && args.spellCast)
        {
            // SOURCE L#: Target = ObjectManager.Player, not args.Target (preserve origin).
            LastCastedSpellEntry entry{args.spellCast.SpellName(), ObjectManager::LocalPlayer(), Game::TimeGame()};
            CastedSpells[args.sender.NetworkId()] = std::move(entry);
        }
    }

    void LastCastedSpell::SpellbookOnCastSpell(CastSpellEvent& args)
    {
        if (args.sender && args.sender.IsMe())
        {
            lastCastPacketSent = LastCastPacketSentEntry{args.slot, args.target ? args.target.NetworkId() : 0u,
                Game::TimeGame()};
        }
    }

    void LastCastedSpell::Reset() noexcept
    {
        CastedSpells.clear();
        lastCastPacketSent.reset();
    }

    const std::vector<InterruptableSpell>& Interrupter::Spells()
    {
        // SOURCE L#: static Interrupter() adds these entries in this order.
        // Historical spell/buff names: verify against current LoL patch before relying on them.
        static const std::vector<InterruptableSpell> spells = {
        {"VarusQ", "Varus", InterruptableDangerLevel::Low, 0, SpellSlot::_Q, "VarusQ"},
        {"UrgotSwap2", "Urgot", InterruptableDangerLevel::High, 0, SpellSlot::_R, "UrgotSwap2"},
        {"CaitlynAceintheHole", "Caitlyn", InterruptableDangerLevel::High, 600, SpellSlot::_R, "CaitlynAceintheHole"},
        {"infiniteduresssound", "Warwick", InterruptableDangerLevel::High, 0, SpellSlot::_R, "InfiniteDuress"},
        {"shenstandunitedlock", "Shen", InterruptableDangerLevel::Low, 0, SpellSlot::_R, "ShenStandUnited"},
        {"alzaharnethergraspsound", "Malzahar", InterruptableDangerLevel::High, 2000, SpellSlot::_R, "AlZaharNetherGrasp"},
        {"AbsoluteZero", "Nunu", InterruptableDangerLevel::High, 0, SpellSlot::_R, "AbsoluteZero"},
        {"PantheonRJump", "Pantheon", InterruptableDangerLevel::High, 0, SpellSlot::_R, "PantheonRJump"},
        {"karthusfallenonecastsound", "Karthus", InterruptableDangerLevel::High, 0, SpellSlot::_R, "KarthusFallenOne"},
        {"VelkozR", "Velkoz", InterruptableDangerLevel::High, 0, SpellSlot::_R, "VelkozR"},
        {"GalioIdolOfDurand", "Galio", InterruptableDangerLevel::High, 200, SpellSlot::_R, "GalioIdolOfDurand"},
        {"missfortunebulletsound", "MissFortune", InterruptableDangerLevel::High, 0, SpellSlot::_R, "MissFortuneBulletTime"},
        {"Drain", "FiddleSticks", InterruptableDangerLevel::Medium, 0, SpellSlot::_W, "Drain"},
        {"fearmonger_marker", "FiddleSticks", InterruptableDangerLevel::Medium, 0, SpellSlot::_W, "Drain"},
        {"katarinarsound", "Katarina", InterruptableDangerLevel::High, 0, SpellSlot::_R, "KatarinaR"},
        {"Meditate", "MasterYi", InterruptableDangerLevel::Low, 0, SpellSlot::_W, "Meditate"},
        {"XerathLocusOfPower2", "Xerath", InterruptableDangerLevel::Low, 0, SpellSlot::_R, "XerathLocusOfPower2"},
        {"ReapTheWhirlwind", "Janna", InterruptableDangerLevel::Low, 0, SpellSlot::_R, "ReapTheWhirlwind"},
        {"LucianR", "Lucian", InterruptableDangerLevel::High, 0, SpellSlot::_R, "LucianR"},
        {"Destiny", "TwistedFate", InterruptableDangerLevel::Medium, 0, SpellSlot::_R, "Destiny"},
        };
        return spells;
    }

    void Interrupter::Initialize() noexcept
    {
        // RSDK-CHANGE: consumers subscribe their own Events; do not auto register.
    }

    bool Interrupter::IsChannelingImportantSpell(const Object& unit)
    {
        if (!unit)
            return false;

        const auto lastCastedspell = LastCastedSpell::LastCastedspell(unit);
        const auto lastCastPacketSent = LastCastedSpell::LastCastPacketSent();
        const float now = Game::TimeGame();
        const std::string championName = unit.ChampionName();

        for (const InterruptableSpell& spell : Spells())
        {
            if (spell.ChampionName == championName &&
                ((lastCastedspell && EqualsIgnoreCase(lastCastedspell->Name, spell.SpellName) &&
                  now - lastCastedspell->Tick < (350.0f + spell.ExtraDuration) * 0.001f) ||
                 (!spell.BuffName.empty() && unit.HasBuff(spell.BuffName.c_str())) ||
                 (unit.IsMe() && lastCastPacketSent && lastCastPacketSent->Slot == spell.Slot &&
                  now - lastCastPacketSent->Tick < (150.0f + static_cast<float>(Game::Latency())) * 0.001f)))
            {
                return true;
            }
        }
        return false;
    }

    void Interrupter::Shutdown() noexcept
    {
        onPossibleToInterruptHandlers.clear();
    }

    std::uint64_t Interrupter::AddOnPossibleToInterruptHandler(OnPossibleToInterruptH handler)
    {
        if (!handler)
            return 0;
        const std::uint64_t token = nextInterrupterHandlerId++;
        onPossibleToInterruptHandlers.emplace_back(token, std::move(handler));
        return token;
    }

    bool Interrupter::RemoveOnPossibleToInterruptHandler(std::uint64_t token)
    {
        const auto it = std::remove_if(onPossibleToInterruptHandlers.begin(),
            onPossibleToInterruptHandlers.end(), [token](const auto& item) { return item.first == token; });
        const bool removed = it != onPossibleToInterruptHandlers.end();
        onPossibleToInterruptHandlers.erase(it, onPossibleToInterruptHandlers.end());
        return removed;
    }

    void Interrupter::SetOnPossibleToInterrupt(OnPossibleToInterruptH handler)
    {
        onPossibleToInterruptHandlers.clear();
        if (handler)
            (void)AddOnPossibleToInterruptHandler(std::move(handler));
    }

    void Interrupter::FireOnInterruptable(const Object& unit, const InterruptableSpell& spell)
    {
        // Snapshot: user callbacks may add/remove handlers while dispatching.
        const auto handlers = onPossibleToInterruptHandlers;
        for (const auto& entry : handlers)
        {
            if (entry.second)
                entry.second(unit, spell);
        }
    }

    void Interrupter::Game_OnGameUpdate()
    {
        if (onPossibleToInterruptHandlers.empty())
            return;

        const auto now = Game::TimeGame();
        const Object myHero = ObjectManager::LocalPlayer();
        if (!myHero)
            return;

        for (const Object& enemy : ObjectManager::AllHeroes())
        {
            // SOURCE L#: e.IsValidTarget() (RSDK uses shared Utility with 2D range semantics).
            if (!Utility::IsValidTarget(enemy))
                continue;

            const auto lastCastedspell = LastCastedSpell::LastCastedspell(enemy);
            for (const InterruptableSpell& spell : Spells())
            {
                // SOURCE L#: unlike IsChannelingImportantSpell, Game_OnGameUpdate
                // deliberately does NOT filter the spell list by ChampionName.
                if ((lastCastedspell && EqualsIgnoreCase(lastCastedspell->Name, spell.SpellName) &&
                    now - lastCastedspell->Tick < (350.0f + spell.ExtraDuration) * 0.001f) ||
                    (!spell.BuffName.empty() && enemy.HasBuff(spell.BuffName.c_str())))
                {
                    FireOnInterruptable(enemy, spell);
                }
            }
        }
    }
}
