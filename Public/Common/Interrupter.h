#pragma once
// LeagueSharp.Common/Interrupter.cs + LastCastedSpell.cs port (legacy database).
// Compile Interrupter.cpp directly into the consuming DLL, not RSDK Core.
// NO automatic event subscriptions: the consuming DLL owns subscriptions/unsubscriptions.
#include "../RSDK.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace rsdk
{
    enum class InterruptableDangerLevel { Low, Medium, High };

    struct InterruptableSpell
    {
        std::string BuffName;
        std::string ChampionName;
        InterruptableDangerLevel DangerLevel{InterruptableDangerLevel::Low};
        int ExtraDuration{}; // L# milliseconds
        SpellSlot Slot{SpellSlot::_SlotUnknown};
        std::string SpellName;
    };

    struct LastCastedSpellEntry
    {
        std::string Name;
        Object Target;
        float Tick{}; // RSDK Game::TimeGame(), seconds; L# TickCount is milliseconds
    };

    struct LastCastPacketSentEntry
    {
        SpellSlot Slot{SpellSlot::_SlotUnknown};
        std::uint32_t TargetNetworkId{};
        float Tick{}; // game seconds
    };

    class LastCastedSpell final
    {
    public:
        [[nodiscard]] static std::optional<LastCastedSpellEntry> LastCastedspell(const Object& unit);
        [[nodiscard]] static std::string LastCastedSpellName(const Object& unit);
        [[nodiscard]] static float LastCastedSpellT(const Object& unit);
        [[nodiscard]] static Object LastCastedSpellTarget(const Object& unit);
        [[nodiscard]] static std::optional<LastCastPacketSentEntry> LastCastPacketSent();

        // Consumers explicitly subscribe these; this class DOES NOT register itself.
        static void Obj_AI_Hero_OnProcessSpellCast(const ProcessSpellEvent& args);
        static void SpellbookOnCastSpell(CastSpellEvent& args);
        static void Reset() noexcept;
    };

    using OnPossibleToInterruptH = std::function<void(const Object&, const InterruptableSpell&)>;

    class Interrupter final
    {
    public:
        // L# Interrupter.Spells, including original entry order and duplicates.
        [[nodiscard]] static const std::vector<InterruptableSpell>& Spells();
        // No-op by design: Unlike L# static constructor, NEVER registers events.
        static void Initialize() noexcept;
        [[nodiscard]] static bool IsChannelingImportantSpell(const Object& unit);
        static void Shutdown() noexcept;
        // L# OnPossibleToInterrupt is multicast. Handlers are scoped to the
        // consuming DLL; these methods do NOT register any RSDK game event.
        static std::uint64_t AddOnPossibleToInterruptHandler(OnPossibleToInterruptH handler);
        static bool RemoveOnPossibleToInterruptHandler(std::uint64_t token);
        static void SetOnPossibleToInterrupt(OnPossibleToInterruptH handler); // replace all handlers
        static void Game_OnGameUpdate();

    private:
        static void FireOnInterruptable(const Object& unit, const InterruptableSpell& spell);
    };
}
