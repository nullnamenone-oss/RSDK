#pragma once

#include <RSDK.h>

namespace userbot::debug
{
    void PrintPerks(const rsdk::Object& object);
    void PrintBuffs(const rsdk::Object& object);
    void PrintItems(const rsdk::Object& object);
    void PrintSpells(const rsdk::Object& object);
    void PrintSpellCast(const char* label, const rsdk::SpellCast& spellCast);

    void PrintPlayer();
    void PrintHeroes();
    void PrintMinions();
    void PrintTurrets();
    void PrintBuildings();
    void PrintAllObjects();
}
