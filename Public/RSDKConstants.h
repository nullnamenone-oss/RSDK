#pragma once

#include <stdint.h>

/*
 * Shared public RSDK enums are centralized in this file.
 * Types that are part of the C ABI remain C-compatible.
 */
typedef enum RSDK_LogLevel
{
    RSDK_LOG_TRACE   = 0,
    RSDK_LOG_INFO    = 1,
    RSDK_LOG_WARNING = 2,
    RSDK_LOG_ERROR   = 3
} RSDK_LogLevel;

// Generic damage type shared by TargetSelector, damage helpers, Orbwalker and champion logic.
typedef enum RSDK_DamageType
{
    RSDK_DAMAGE_MAGICAL = 0,
    RSDK_DAMAGE_PHYSICAL = 1,
    RSDK_DAMAGE_TRUE = 2
} RSDK_DamageType;

// Compatibility aliases for the v4.0-v4.2 TargetSelector-specific names.
#define RSDK_TARGET_DAMAGE_MAGICAL RSDK_DAMAGE_MAGICAL
#define RSDK_TARGET_DAMAGE_PHYSICAL RSDK_DAMAGE_PHYSICAL
#define RSDK_TARGET_DAMAGE_TRUE RSDK_DAMAGE_TRUE


#if defined(__cplusplus)

#include <cstdint>

namespace rsdk
{
    enum class DamageType : std::uint8_t
    {
        Magical = RSDK_DAMAGE_MAGICAL,
        Physical = RSDK_DAMAGE_PHYSICAL,
        True = RSDK_DAMAGE_TRUE
    };
}

// Filters objects by team relationship to LocalPlayer.
enum class TeamFilter : std::uint8_t
{
    All             = 0,
    Ally            = 1,
    Enemy           = 2
};

enum class GameObjectTeam : std::int32_t
{
	Unknown,
	Order	= 100,
	Chaos	= 200,
	Neutral = 300
};

// Public spell slots shared by Core and C++ plugins.
enum class SpellSlot : std::int8_t
{
    _Q					= 0,
    _W					= 1,
    _E					= 2,
    _R					= 3,
    _D					= 4,                 // SummonerSlot1
    _F					= 5,                 // SummonerSlot2
    _Item1				= 6,
    _Item2				= 7,
    _Item3				= 8,
    _Item4				= 9,
    _Item5				= 10,
    _Item6				= 11,
    _SlotTrinket		= 12,
    _SlotRecall			= 13,
    _SlotUnknown		= -1
};

// ObjectType is a bitmask; one object may contain multiple flags at the same time.
enum class ObjectTypeFlags : std::uint32_t
{
    Unknown				= 0,
    Unknown1			= 1,
    ObjectAI			= 0x400,
    Minion				= 0x800,
    Hero				= 0x1000,
    Turret				= 0x2000,
    Missile				= 0x8000,
    Maker				= 0x10000
};

enum class BuffType : std::int8_t
{
	Internal			= 0,
	Aura				= 1,
	CombatEnchancer		= 2,
	CombatDehancer		= 3,			
	SpellShield			= 4,
	Stun				= 5,
	Invisibility		= 6,
	Silence				= 7,
	Taunt				= 8,
	Berserk				= 9,			
	Polymorph			= 10,
	Slow				= 11,
	Snare				= 12,
	Damage				= 13,
	Heal				= 14,
	Haste				= 15,
	SpellImmunity		= 16,
	PhysicalImmunity	= 17,
	Invulnerability		= 18,
	AttackSpeedSlow		= 19,			
	NearSight			= 20,
	Frenzy				= 21,
	Fear				= 22,
	Charm				= 23,
	Poison				= 24,
	Suppression			= 25,			
	Blind				= 26,
	Counter				= 27,
	Shred				= 28,
	Flee				= 29,
	Knockup				= 30,
	Knockback			= 31,
	Disarm				= 32,			
	Grounded            = 33,			
	Drowsy              = 34,
	Sleep               = 35,
	Obscured			= 36,
	ClickProofToEnemies	= 37,
	Unkillable			= 38
};

enum class GameObjectOrder : std::uint8_t
{
	HoldPosition		= 1,
	MoveTo				= 2,
	AttackUnit			= 3,
	AutoAttackPet		= 4,
	AutoAttack			= 5,
	MovePet				= 6,
	AttackTo			= 7,
	Stop				= 10
};

enum class SpellState : std::uint8_t
{
	Ready				= 0,
	NotAvailable		= 4,
	Surpressed			= 8,
	NotLearned			= 12,
	Cooldown			= 32,
	NoMana				= 64,
	Unknown
};

enum class Emotes : std::uint8_t
{
	Dance				= 0,
	Taunt				= 1,
	Laugh				= 2,
	Joke				= 3,
	Toggle				= 4,
	Unknown				= 5,
};





#endif
