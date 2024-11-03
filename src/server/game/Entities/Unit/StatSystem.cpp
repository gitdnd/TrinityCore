/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// @tswow-begin
#include "TSCreature.h"
// @tswow-end

#include "Unit.h"
#include "Creature.h"
#include "Item.h"
#include "Pet.h"
#include "Player.h"
#include "SharedDefines.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "World.h"
#include <numeric>

// @tswow-begin move dodge_cap to top of file and remove const
float dodge_cap[MAX_CLASSES] =
{
    88.129021f,     // Warrior
    88.129021f,     // Paladin
    145.560408f,    // Hunter
    145.560408f,    // Rogue
    150.375940f,    // Priest
    88.129021f,     // DK
    145.560408f,    // Shaman
    150.375940f,    // Mage
    150.375940f,    // Warlock
    0.0f,           // ??
    116.890707f,     // Druid

    // default values for custom classes
    100,100,100,100,100,100,100,
    100,100,100,100,100,100,100,
    100,100,100,100,100,100,100,
};
// @tswow-end

// @tswow-begin move miss_cap to top of file and remove const
float miss_cap[MAX_CLASSES] =
{
    16.00f,     // Warrior //correct
    16.00f,     // Paladin //correct
    16.00f,     // Hunter  //?
    16.00f,     // Rogue   //?
    16.00f,     // Priest  //?
    16.00f,     // DK      //correct
    16.00f,     // Shaman  //?
    16.00f,     // Mage    //?
    16.00f,     // Warlock //?
    0.0f,       // ??
    16.00f,      // Druid   //?

    // default values for custom classes
    16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,
};
// @tswow-end

// @tswow-begin move m_diminishing_k to top of file and remove const
float m_diminishing_k[MAX_CLASSES] =
{
    0.9560f,  // Warrior
    0.9560f,  // Paladin
    0.9880f,  // Hunter
    0.9880f,  // Rogue
    0.9830f,  // Priest
    0.9560f,  // DK
    0.9880f,  // Shaman
    0.9830f,  // Mage
    0.9830f,  // Warlock
    0.0f,     // ??
    0.9720f,   // Druid
    // default values for custom classes
    0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,
    0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,
    0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,0.98f,
};
// @tswow-end

// @tswow-begin move parry_cap to top of file and remove const
float parry_cap[MAX_CLASSES] =
{
    47.003525f,     // Warrior
    47.003525f,     // Paladin
    145.560408f,    // Hunter
    145.560408f,    // Rogue
    0.0f,           // Priest
    47.003525f,     // DK
    145.560408f,    // Shaman
    0.0f,           // Mage
    0.0f,           // Warlock
    0.0f,           // ??
    0.0f,           // Druid

    // default values for custom classes
    0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,
};
// @tswow-end

inline bool _ModifyUInt32(bool apply, uint32& baseValue, int32& amount)
{
    // If amount is negative, change sign and value of apply.
    if (amount < 0)
    {
        apply = !apply;
        amount = -amount;
    }
    if (apply)
        baseValue += amount;
    else
    {
        // Make sure we do not get uint32 overflow.
        if (amount > int32(baseValue))
            amount = baseValue;
        baseValue -= amount;
    }
    return apply;
}

/*#######################################
########                         ########
########    UNIT STAT SYSTEM     ########
########                         ########
#######################################*/

void Unit::UpdateAllResistances()
{
    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);
}

void Unit::UpdateDamagePhysical(WeaponAttackType attType)
{
    float totalMin = 0.f;
    float totalMax = 0.f;

    float tmpMin, tmpMax;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        CalculateMinMaxDamage(attType, false, true, tmpMin, tmpMax, i);
        totalMin += tmpMin;
        totalMax += tmpMax;
    }

    // @tswow-begin
    if (Creature* c = this->ToCreature())
    {
        // @tswow-begin
        FIRE_ID(
              c->GetCreatureTemplate()->events.id
            , Creature,OnUpdateDamagePhysical
            , TSCreature(c)
            , TSMutableNumber<float>(&totalMin)
            , TSMutableNumber<float>(&totalMax)
            , false
            , uint8(attType)
        );
        // @tswow-end
    }
    // @tswow-end

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            SetStatFloatValue(UNIT_FIELD_MINDAMAGE, totalMin);
            SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, totalMax);
            break;
        case OFF_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, totalMin);
            SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, totalMax);
            break;
        case RANGED_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, totalMin);
            SetStatFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, totalMax);
            break;
    }
}

/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

bool Unit::UpdateStats(Stats stat)
{
    if (stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

    if (stat == STAT_STAMINA || stat == STAT_INTELLECT || stat == STAT_STRENGTH)
    {
        Pet* pet = GetPet();
        if (pet)
            pet->UpdateStats(stat);
    }

    switch (stat)
    {
        case STAT_STRENGTH:
            UpdateShieldBlockValue();
            break;
        case STAT_AGILITY:
            UpdateArmor();
            UpdateAllCritPercentages();
            UpdateDodgePercentage();
            break;
        case STAT_STAMINA:
            UpdateMaxHealth();
            break;
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            UpdateAllSpellCritChances();
            UpdateArmor();                                  //SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT, only armor currently
            break;
        case STAT_SPIRIT:
            break;
        default:
            break;
    }

    if (stat == STAT_STRENGTH)
    {
        UpdateAttackPowerAndDamage(false);
        if (HasAuraTypeWithMiscvalue(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT, stat))
            UpdateAttackPowerAndDamage(true);
    }
    else if (stat == STAT_AGILITY)
    {
        UpdateAttackPowerAndDamage(false);
        UpdateAttackPowerAndDamage(true);
    }
    else
    {
        // Need update (exist AP from stat auras)
        if (HasAuraTypeWithMiscvalue(SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT, stat))
            UpdateAttackPowerAndDamage(false);
        if (HasAuraTypeWithMiscvalue(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT, stat))
            UpdateAttackPowerAndDamage(true);
    }

    UpdateSpellDamageAndHealingBonus();
    UpdatePowerRegen(POWER_MANA);

    // Update ratings in exist SPELL_AURA_MOD_RATING_FROM_STAT and only depends from stat
    uint32 mask = 0;
    AuraEffectList const& modRatingFromStat = GetAuraEffectsByType(SPELL_AURA_MOD_RATING_FROM_STAT);
    for (AuraEffectList::const_iterator i = modRatingFromStat.begin(); i != modRatingFromStat.end(); ++i)
        if (Stats((*i)->GetMiscValueB()) == stat)
            mask |= (*i)->GetMiscValue();
    if (mask)
    {
        for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
            if (mask & (1 << rating))
                ApplyRatingMod(CombatRating(rating), 0, true, true);
    }
    return true;
}
void AddSpellPowerBonus(std::vector<uint16>& vec, SpellSchoolMask schoolMask, uint32 amount)
{
    if (schoolMask & SPELL_SCHOOL_MASK_NORMAL)
    {
        vec[SPELL_SCHOOL_NORMAL] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_HOLY)
    {
        vec[SPELL_SCHOOL_HOLY] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_FIRE)
    {
        vec[SPELL_SCHOOL_FIRE] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_NATURE)
    {
        vec[SPELL_SCHOOL_NATURE] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_FROST)
    {
        vec[SPELL_SCHOOL_FROST] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_SHADOW)
    {
        vec[SPELL_SCHOOL_SHADOW] += amount;
    }
    if (schoolMask & SPELL_SCHOOL_MASK_ARCANE)
    {
        vec[SPELL_SCHOOL_ARCANE] += amount;
    }

void Unit::ApplySpellPowerBonus(int32 amount, bool apply)
{
    apply = _ModifyUInt32(apply, m_baseSpellPower, amount);

    // For speed just update for client

    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, amount, apply);
}

void Unit::UpdateSpellDamageAndHealingBonus()
{

    for (int i = 0; i < MAX_SPELL_SCHOOL; i++)
    {
        m_baseSpellPowerSchool[i]    = 0;
        m_derivedSpellPowerSchool[i] = 0;
    }
    AuraEffectList const& mDamageDone = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (AuraEffectList::const_iterator i = mDamageDone.begin(); i != mDamageDone.end(); ++i)
        if ((*i)->GetSpellInfo()->EquippedItemClass == -1 &&
            // -1 == any item class (not wand then)
            (*i)->GetSpellInfo()->EquippedItemInventoryTypeMask == 0)
                // 0 == any inventory type (not wand then)
                AddSpellPowerBonus(m_baseSpellPowerSchool, (SpellSchoolMask)(*i)->GetMiscValue(), (*i)->GetAmount());


    // Damage bonus from stats
    AuraEffectList const& mDamageDoneOfStatPercent = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);
    for (AuraEffectList::const_iterator i = mDamageDoneOfStatPercent.begin(); i != mDamageDoneOfStatPercent.end(); ++i)
    {
        // stat used stored in miscValueB for this aura
        Stats usedStat = Stats((*i)->GetMiscValueB());
        AddSpellPowerBonus(m_derivedSpellPowerSchool, (SpellSchoolMask)(*i)->GetMiscValue(),
                           int32(CalculatePct(GetStat(usedStat), (*i)->GetAmount())));
        // if this is derived, derived doesn't help any stat buffs, both +ATK auras and +SPELL POWER TO ATK auras
    }
    // ... and attack power
    AuraEffectList const& mDamageDonebyAP = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER);
    for (AuraEffectList::const_iterator i = mDamageDonebyAP.begin(); i != mDamageDonebyAP.end(); ++i)
        AddSpellPowerBonus(m_derivedSpellPowerSchool, (SpellSchoolMask)(*i)->GetMiscValue(),
                           int32(CalculatePct(GetTotalAttackPowerValue(BASE_ATTACK), (*i)->GetAmount())));

    SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, SpellBasePowerBonusDone(SpellSchoolMask(1 << i)));
}

bool Unit::UpdateAllStats()
{
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), int32(value));
    }

    UpdateArmor();
    UpdateAttackPowerAndDamage(false);
    UpdateAttackPowerAndDamage(true);
    UpdateMaxHealth();

    for (uint8 i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    UpdateAllRatings();
    UpdateAllCritPercentages();
    UpdateAllSpellCritChances();
    UpdateDefenseBonusesMod();
    UpdateShieldBlockValue();
    UpdateSpellDamageAndHealingBonus();
    UpdatePowerRegen(POWER_MANA);
    UpdatePowerRegen(POWER_RAGE);
    UpdatePowerRegen(POWER_ENERGY);
    UpdatePowerRegen(POWER_RUNIC_POWER);
    UpdateExpertise(BASE_ATTACK);
    UpdateExpertise(OFF_ATTACK);
    RecalculateRating(CR_ARMOR_PENETRATION);
    UpdateAllResistances();

    return true;
}

void Unit::ApplySpellPenetrationBonus(int32 amount, bool apply)
{
    ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, -amount, apply);
    m_spellPenetrationItemMod += apply ? amount : -amount;
}

void Unit::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
        // @tswow-begin
        FIRE(
              Player,OnUpdateResistance
            , TSPlayer(this)
            , TSMutableNumber<float>(&value)
            , school
        );
        // @tswow-end

        Pet* pet = GetPet();
        if (pet)
            pet->UpdateResistances(school);
    }
    else
        UpdateArmor();
}

void Unit::UpdateArmor(bool derived)
{
    UnitMods unitMod = UNIT_MOD_ARMOR;
    float value;
    if (derived)
    {
        value                       = GetArmor() - m_derivedModifiers[unitMod];
        m_derivedModifiers[unitMod] = 0;
        // add dynamic flat mods
        AuraEffectList const& mResbyIntellect = GetAuraEffectsByType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT);
        for (AuraEffectList::const_iterator i = mResbyIntellect.begin(); i != mResbyIntellect.end(); ++i)
        {
            if ((*i)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
                m_derivedModifiers[unitMod] += CalculatePct(GetStat(Stats((*i)->GetMiscValueB())), (*i)->GetAmount());
        }
    }
    else
    {
        value = GetModifierValue(unitMod, BASE_VALUE); // base armor (from items)
        value *= GetModifierValue(unitMod, BASE_PCT);  // armor percent from items
        value += GetModifierValue(unitMod, TOTAL_VALUE);


        value *= GetModifierValue(unitMod, TOTAL_PCT);
    }

    // @tswow-begin
    FIRE(Player, OnUpdateArmor, TSPlayer(this), TSMutableNumber<float>(&value));

    SetArmor(int32(value + m_derivedModifiers[unitMod]));

    UpdateMaxHealth();
    UpdateAttackPowerAndDamage(); // armor dependent auras update for SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR
}

float Unit::GetHealthBonusFromStamina(int32 health)
{
    float retHp = float(health) * (1.f + (float)GetStat(STAT_STAMINA) / 100.f);
    FIRE(Player,OnCalcStaminaHealthBonus
        , TSPlayer(this)
        , TSMutableNumber<float>(&retHp)
        , baseStam
        , moreStam
    );
    return retHp;
    // @tswow-end
}

float Unit::GetManaBonusFromIntellect(int32 mana)
{

    float retMana = float(mana) * (1.f + (float)GetStat(STAT_INTELLECT) / 100.f);

    FIRE(Player,OnCalcIntellectManaBonus
        ,TSPlayer(this)
        ,TSMutableNumber<float>(&retMana)
        ,baseInt
        ,moreInt
    );
    // @tswow-end
    return retMana;
}

void Unit::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;

    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE);
    value += GetHealthBonusFromStamina(value);
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);
    // @tswow-begin
    FIRE(Player,OnUpdateMaxHealth
        ,TSPlayer(this)
        ,TSMutableNumber<float>(&value)
    );
    // @tswow-end
    SetMaxHealth((uint32)value);
}

void Unit::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + AsUnderlyingType(power));

    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreatePowerValue(power);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE);
    value += GetManaBonusFromIntellect(value);
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);
    // @tswow-begin
    FIRE(Player,OnUpdateMaxPower
        , TSPlayer(this)
        , TSMutableNumber<float>(&value)
        , static_cast<int8>(power)
        , bonusPower
    );
    // @tswow-end
    SetMaxPower(power, uint32(std::lroundf(value)));
}



void Unit::UpdateAttackPowerAndDamage(bool ranged)
{
    float val2 = 0.0f;

    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    if (ranged)
    {
        val2 = GetStat(STAT_AGILITY);

        FIRE(Player,OnUpdateRangedAttackPower
            , TSPlayer(this)
            , TSMutableNumber<float>(&val2)
        );
    }
    else
    {
        val2 = GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY);

        FIRE(Player,OnUpdateAttackPower
            , TSPlayer(this)
            , TSMutableNumber<float>(&val2)
        );
    }

    SetStatFlatModifier(unitMod, BASE_VALUE, val2);

    float base_attPower  = GetFlatModifierValue(unitMod, BASE_VALUE) * GetPctModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetFlatModifierValue(unitMod, TOTAL_VALUE);

    //add dynamic flat mods
    if (ranged)
    {
        if ((GetClassMask() & CLASSMASK_WAND_USERS) == 0)
        {
            AuraEffectList const& mRAPbyStat = GetAuraEffectsByType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT);
            for (AuraEffect const* aurEff : mRAPbyStat)
                attPowerMod += CalculatePct(GetStat(Stats(aurEff->GetMiscValue())), aurEff->GetAmount());
        }
    }
    else
    {
        AuraEffectList const& mAPbyStat = GetAuraEffectsByType(SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT);
        for (AuraEffect const* aurEff : mAPbyStat)
            attPowerMod += CalculatePct(GetStat(Stats(aurEff->GetMiscValue())), aurEff->GetAmount());
    }

    // applies to both, amount updated in PeriodicTick each 30 seconds
    attPowerMod += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR);

    float attPowerMultiplier = GetPctModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    if (ranged)
    {
        SetRangedAttackPower(int32(base_attPower));
        if (attPowerMod >= 0)
            SetRangedAttackPowerModPos(int32(attPowerMod));
        if (attPowerMod <= 0)
            SetRangedAttackPowerModNeg(int32(attPowerMod));
        SetRangedAttackPowerMultiplier(attPowerMultiplier);
    }
    else
    {
        SetAttackPower(int32(base_attPower));
        if (attPowerMod >= 0)
            SetAttackPowerModPos(int32(attPowerMod));
        if (attPowerMod <= 0)
            SetAttackPowerModNeg(int32(attPowerMod));
        SetAttackPowerMultiplier(attPowerMultiplier);
    }

    Pet* pet = GetPet();                                //update pet's AP
    Guardian* guardian = GetGuardianPet();
    //automatically update weapon damage after attack power modification
    if (ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);
        if (pet && pet->IsHunterPet()) // At ranged attack change for hunter pet
            pet->UpdateAttackPowerAndDamage();
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);

        UpdateSpellDamageAndHealingBonus();
        if (guardian)
            guardian->UpdateAttackPowerAndDamage();
    }
}

void Unit::UpdateShieldBlockValue()
{
    // @tswow-begin move block and fire event
    uint32 block = GetShieldBlockValue();
    FIRE(
          Player,OnUpdateShieldBlock
        , TSPlayer(this)
        , TSMutableNumber<uint32>(&block)
    );
    SetUInt32Value(PLAYER_SHIELD_BLOCK, block);
    // @tswow-end
}

void Unit::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, uint8 damageIndex) const
{
    // Only proto damage, not affected by any mods
    if (damageIndex != 0)
    {
        minDamage = 0.0f;
        maxDamage = 0.0f;

        if (CanUseAttackType(attType))
        {
            minDamage = GetWeaponDamageRange(attType, MINDAMAGE, damageIndex);
            maxDamage = GetWeaponDamageRange(attType, MAXDAMAGE, damageIndex);
        }
        return;
    }

    UnitMods unitMod;

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    float const attackPowerMod = std::max(GetAPMultiplier(attType, normalized), 0.25f);

    float baseValue  = GetFlatModifierValue(unitMod, BASE_VALUE);
    baseValue += GetTotalAttackPowerValue(attType) / 14.0f * attackPowerMod;

    float basePct    = GetPctModifierValue(unitMod, BASE_PCT);
    float totalValue = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float totalPct   = addTotalPct ? GetPctModifierValue(unitMod, TOTAL_PCT) : 1.0f;

    float weaponMinDamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weaponMaxDamage = GetWeaponDamageRange(attType, MAXDAMAGE);



    if (!CanUseAttackType(attType)) // check if player not in form but still can't use (disarm case)
    {
        // cannot use ranged/off attack, set values to 0
        if (attType != BASE_ATTACK)
        {
            minDamage = 0.f;
            maxDamage = 0.f;
            return;
        }

        weaponMinDamage = BASE_MINDAMAGE;
        weaponMaxDamage = BASE_MAXDAMAGE;
    }
    else if (attType == RANGED_ATTACK) // add ammo DPS to ranged primary damage
    {
        if (IsPlayer())
        {
            Player* player = dynamic_cast<Player*>(this);
            weaponMinDamage += player->GetAmmoDPS() * attackSpeedMod;
            weaponMaxDamage += player->GetAmmoDPS() * attackSpeedMod;
        }
    }

    minDamage = ((weaponMinDamage + baseValue) * basePct + totalValue) * totalPct;
    maxDamage = ((weaponMaxDamage + baseValue) * basePct + totalValue) * totalPct;
}

void Unit::UpdateDefenseBonusesMod()
{
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
}

void Player::UpdateBlockPercentage()
{
    // No block
    float value = 0.0f;
    // Modify value from defense skill
    value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    // Increase from SPELL_AURA_MOD_BLOCK_PERCENT aura
    value += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
    // Increase from rating
    value += GetRatingBonusValue(CR_BLOCK);

    value = value < 0.0f ? 0.0f : value;

    // @tswow-begin
    FIRE(
          Player,OnUpdateBlockPercentage
        , TSPlayer(this)
        , TSMutableNumber<float>(&value)
    );
    // @tswow-end
    SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, value);
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup;
    uint16 index;
    CombatRating cr;

    switch (attType)
    {
        case OFF_ATTACK:
            modGroup = OFFHAND_CRIT_PERCENTAGE;
            index = PLAYER_OFFHAND_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
        case RANGED_ATTACK:
            modGroup = RANGED_CRIT_PERCENTAGE;
            index = PLAYER_RANGED_CRIT_PERCENTAGE;
            cr = CR_CRIT_RANGED;
            break;
        case BASE_ATTACK:
        default:
            modGroup = CRIT_PERCENTAGE;
            index = PLAYER_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
    }

    // flat = bonus from crit auras, pct = bonus from agility, combat rating = mods from items
    float value = GetBaseModValue(modGroup, FLAT_MOD) + GetBaseModValue(modGroup, PCT_MOD) + GetRatingBonusValue(cr);

    // Modify crit from weapon skill and maximized defense skill of same level victim difference
    value += (int32(GetWeaponSkillValue(attType)) - int32(GetMaxSkillValueForLevel())) * 0.04f;

    if (sWorld->getBoolConfig(CONFIG_STATS_LIMITS_ENABLE))
         value = value > sWorld->getFloatConfig(CONFIG_STATS_LIMITS_CRIT) ? sWorld->getFloatConfig(CONFIG_STATS_LIMITS_CRIT) : value;

    value = std::max(0.0f, value);
    // @tswow-begin
    FIRE(
        Player,OnUpdateCrit
        , TSPlayer(this)
        , TSMutableNumber<float>(&value)
        , uint32(attType)
    );
    // @tswow-end
    SetStatFloatValue(index, value);
}

void Unit::UpdateAllCritPercentages()
{
    float value = GetMeleeCritFromAgility();

    SetBaseModPctValue(CRIT_PERCENTAGE, value);
    SetBaseModPctValue(OFFHAND_CRIT_PERCENTAGE, value);
    SetBaseModPctValue(RANGED_CRIT_PERCENTAGE, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

// @tswow-begin move m_diminishing_k to top of file
// @tswow-end

// helper function
float CalculateDiminishingReturns(float const (&capArray)[MAX_CLASSES], uint8 playerClass, float nonDiminishValue, float diminishValue)
{
    //  1     1     k              cx
    // --- = --- + --- <=> x' = --------
    //  x'    c     x            x + ck

    // where:
    // k  is m_diminishing_k for that class
    // c  is capArray for that class
    // x  is chance before DR (diminishValue)
    // x' is chance after DR (our result)

    uint32 const classIdx = playerClass - 1;

    float const k = m_diminishing_k[classIdx];
    float const c = capArray[classIdx];

    float result = c * diminishValue / (diminishValue + c * k);
    result += nonDiminishValue;
    return result;
}

// @tswow-begin move miss_cap to top of file
// @tswow-end

float Unit::GetMissPercentageFromDefense() const
{
    float diminishing = 0.0f, nondiminishing = 0.0f;
    // Modify value from defense skill (only bonus from defense rating diminishes)
    nondiminishing += (int32(GetSkillValue(SKILL_DEFENSE)) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    diminishing += (GetRatingBonusValue(CR_DEFENSE_SKILL) * 0.04f);

    // apply diminishing formula to diminishing miss chance
    return CalculateDiminishingReturns(miss_cap, 1, nondiminishing, diminishing);
}

// @tswow-begin move parry-cap to top of file
// @tswow-end

void Unit::UpdateParryPercentage()
{
    // No parry
    float value          = 0.0f;
    float nondiminishing = 5.0f;
    // Parry from rating
    float diminishing = GetRatingBonusValue(CR_PARRY);
    // Parry from SPELL_AURA_MOD_PARRY_PERCENT aura
    nondiminishing += GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
    // apply diminishing formula to diminishing parry chance
    m_realParry = nondiminishing + diminishing;
    m_realParry = m_realParry < 0.0f ? 0.0f : m_realParry;

    value = std::max(diminishing + nondiminishing, 0.0f);


    // @tswow-begin
    FIRE(
          Player,OnUpdateParryPercentage
        , TSPlayer(this)
        , TSMutableNumber<float>(&value)
    );
    // @tswow-end
    SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, value);
}

// @tswow-begin move dodge_cap to top of file
// @tswow-end

void Unit::UpdateDodgePercentage()
{
    float diminishing = 0.0f, nondiminishing = 0.0f;
    GetDodgeFromAgility(diminishing, nondiminishing);
    // Modify value from defense skill (only bonus from defense rating diminishes)
    nondiminishing += (int32(GetSkillValue(SKILL_DEFENSE));
    diminishing += (GetRatingBonusValue(CR_DEFENSE_SKILL) * 0.04f);
    // Dodge from SPELL_AURA_MOD_DODGE_PERCENT aura
    nondiminishing += GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
    // Dodge from rating
    diminishing += GetRatingBonusValue(CR_DODGE);

    value *= 0.1f;

    value = value < 0.0f ? 0.0f : value;
    // @tswow-begin
    FIRE(
          Player,OnUpdateDodgePercentage
        , TSPlayer(this)
        , TSMutableNumber<float>(&value)
    );
    // @tswow-end
    SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, value);
}

void Unit::UpdateSpellCritChance(uint32 school)
{
    // For normal school set zero crit chance
    if (school == SPELL_SCHOOL_NORMAL)
    {
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1, 0.0f);
        return;
    }
    // For others recalculate it from:
    float crit = 0.0f;
    // Crit from Intellect
    crit += GetSpellCritFromIntellect();
    // Increase crit from SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
    // Increase crit from SPELL_AURA_MOD_CRIT_PCT
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
    // Increase crit by school from SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    crit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, 1<<school);
    // Increase crit from spell crit ratings
    crit += GetRatingBonusValue(CR_CRIT_SPELL);

    // @tswow-begin
    FIRE(
          Player,OnUpdateSpellCrit
        , TSPlayer(this)
        , TSMutableNumber<float>(&crit)
        , school
    );
    // @tswow-end
    // Store crit value
    SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + school, crit);
}

void Unit::UpdateArmorPenetration(int32 amount)
{
    // Store Rating Value
    // @tswow-begin
    FIRE(
          Player,OnUpdateArmorPenetration
        , TSPlayer(this)
        , TSMutableNumber<int32>(&amount)
    );
    // @tswow-end
    SetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + AsUnderlyingType(CR_ARMOR_PENETRATION), amount);
}

void Unit::UpdateMeleeHitChances()
{
    m_modMeleeHitChance = GetRatingBonusValue(CR_HIT_MELEE);
    // @tswow-begin
    FIRE(
          Player,OnUpdateMeleeHitChances
        , TSPlayer(this)
        , TSMutableNumber<float>(&m_modMeleeHitChance)
    );
    // @tswow-end
}

void Unit::UpdateRangedHitChances()
{
    m_modRangedHitChance = GetRatingBonusValue(CR_HIT_RANGED);
    // @tswow-begin
    FIRE(
        Player,OnUpdateRangedHitChances
        , TSPlayer(this)
        , TSMutableNumber<float>(&m_modRangedHitChance)
    );
    // @tswow-end
}

void Unit::UpdateSpellHitChances()
{
    m_modSpellHitChance = (float)GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
    m_modSpellHitChance += GetRatingBonusValue(CR_HIT_SPELL);
    // @tswow-begin
    FIRE(
        Player,OnUpdateSpellHitChances
        , TSPlayer(this)
        , TSMutableNumber<float>(&m_modSpellHitChance)
    );
    // @tswow-end
}

void Unit::UpdateAllSpellCritChances()
{
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateSpellCritChance(i);
}

void Unit::UpdateExpertise(WeaponAttackType attack)
{
    if (attack == RANGED_ATTACK)
        return;

    return(int32(GetRatingBonusValue(CR_EXPERTISE)));
}

void Unit::ApplyManaRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseManaRegen, amount);
    UpdatePowerRegen(POWER_MANA);
}

void Unit::ApplyHealthRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseHealthRegen, amount);
}

static std::pair<float, Optional<Rates>> const powerRegenInfo[MAX_POWERS] =
{
    { 0.f,      RATE_POWER_MANA             }, // POWER_MANA
    { -12.5f,   RATE_POWER_RAGE_LOSS        }, // POWER_RAGE,           -1.25 rage per second
    { 0.f,      std::nullopt                }, // POWER_FOCUS
    { 10.f,     RATE_POWER_ENERGY           }, // POWER_ENERGY,         +10 energy per second
    { 0.f,      std::nullopt                }, // POWER_HAPPINESS
    { 0.f,      std::nullopt                }, // POWER_RUNE
    { -12.5f,   RATE_POWER_RUNICPOWER_LOSS  }  // POWER_RUNIC_POWER,    -1.25 runic power per second
};

void Unit::UpdatePowerRegen(Powers power)
{
    if (power == POWER_HEALTH || power >= MAX_POWERS)
        return;

    float result_regen              = 0.f; // Out-of-combat / without last mana use effect
    float result_regen_interrupted  = 0.f; // In combat / with last mana use effect
    float modifier                  = 1.f; // Config rate or any other modifiers

    // Set regen rate in cast state apply only on spirit based regen
    int32 modManaRegenInterrupt = GetTotalAuraModifier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    if (modManaRegenInterrupt > 100)
        modManaRegenInterrupt = 100;

    /// @todo possible use of miscvalueb instead of amount
    if (HasAuraTypeWithValue(SPELL_AURA_PREVENT_REGENERATE_POWER, power))
    {
        SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + AsUnderlyingType(power), power == POWER_ENERGY ? -10.f : 0.f);
        SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER + AsUnderlyingType(power), power == POWER_ENERGY ? -10.f : 0.f);
        return;
    }

    switch (power)
    {
        case POWER_MANA:
        {
            float Intellect = GetStat(STAT_INTELLECT);
            // Mana regen from spirit and intellect
            float power_regen = std::sqrt(Intellect) * OCTRegenMPPerSpirit();
            // Apply PCT bonus from SPELL_AURA_MOD_POWER_REGEN_PERCENT aura on spirit base regen
            power_regen *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_MANA);

            // Mana regen from SPELL_AURA_MOD_POWER_REGEN aura
            float power_regen_mp5 = (GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_MANA) + m_baseManaRegen) / 5.0f;

            // Get bonus from SPELL_AURA_MOD_MANA_REGEN_FROM_STAT aura
            AuraEffectList const& regenAura = GetAuraEffectsByType(SPELL_AURA_MOD_MANA_REGEN_FROM_STAT);
            for (AuraEffectList::const_iterator i = regenAura.begin(); i != regenAura.end(); ++i)
                power_regen_mp5 += GetStat(Stats((*i)->GetMiscValue())) * (*i)->GetAmount() / 500.0f;

            // Set regen rate in cast state apply only on spirit based regen
            int32 modManaRegenInterrupt = GetTotalAuraModifier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
            if (modManaRegenInterrupt > 100)
                modManaRegenInterrupt = 100;

            result_regen                = power_regen_mp5 + power_regen;
            result_regen_interrupted    = power_regen_mp5 + CalculatePct(power_regen, modManaRegenInterrupt);


            break;

                // @tswow-begin
    FIRE(Player,OnUpdateManaRegen
        , TSPlayer(this)
        , TSMutableNumber<float>(&power_regen)
        , TSMutableNumber<float>(&power_regen_mp5)
        , TSMutableNumber<int32>(&modManaRegenInterrupt)
    );
    // @tswow-end
    SetStatFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, power_regen_mp5 + CalculatePct(power_regen, modManaRegenInterrupt));
        }
        case POWER_RAGE:
        case POWER_ENERGY:
        case POWER_RUNIC_POWER:
        {
            result_regen                = powerRegenInfo[AsUnderlyingType(power)].first;
            result_regen_interrupted    = 0.f;

            result_regen *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, AsUnderlyingType(power));
            result_regen_interrupted += static_cast<float>(GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, AsUnderlyingType(power))) / 5.f;

            if (power != POWER_RUNIC_POWER) // Butchery requires combat
                result_regen += result_regen_interrupted;
            break;
        }
        default:
            break;
    }

    if (powerRegenInfo[AsUnderlyingType(power)].second.has_value())
        modifier *= sWorld->getRate(powerRegenInfo[AsUnderlyingType(power)].second.value()); // Config rate

    result_regen                *= modifier;
    result_regen_interrupted    *= modifier;

    // Unit fields contain an offset relative to the base power regeneration.
    if (power != POWER_MANA)
        result_regen -= powerRegenInfo[AsUnderlyingType(power)].first;

    if (power == POWER_ENERGY)
        result_regen_interrupted = result_regen;

    SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + AsUnderlyingType(power), result_regen);
    SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER + AsUnderlyingType(power), result_regen_interrupted);
}

float Unit::GetPowerRegen(Powers power) const
{
    if (power == POWER_HEALTH || power >= MAX_POWERS)
        return 0.f;

    bool interrupted =  HasAuraType(SPELL_AURA_INTERRUPT_REGEN) ||
                        (power == POWER_MANA && IsUnderLastManaUseEffect()) ||
                        (power != POWER_MANA && IsInCombat());

    float regen = GetFloatValue((interrupted ? UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER : UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER) + AsUnderlyingType(power));
    if (power != POWER_MANA)
        regen += (power == POWER_ENERGY || !interrupted) ? powerRegenInfo[AsUnderlyingType(power)].first : 0.f;

    return regen;
}

void Unit::UpdateRuneRegen(RuneType rune)
{
    if (rune >= NUM_RUNE_TYPES)
        return;

    uint32 cooldown = 0;

    for (uint32 i = 0; i < MAX_RUNES; ++i)
        if (GetBaseRune(i) == rune)
        {
            cooldown = GetRuneBaseCooldown(i);
            break;
        }

    if (cooldown <= 0)
        return;

    float regen = float(1 * IN_MILLISECONDS) / float(cooldown);
    // @tswow-begin
    FIRE(
          Player,OnUpdateRuneRegen
        , TSPlayer(this)
        , TSMutableNumber<float>(&regen)
        , uint32(rune)
    );
    // @tswow-end
    SetFloatValue(PLAYER_RUNE_REGEN_1 + uint8(rune), regen);
}

void Unit::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraStatMods();
    if (IsPlayer())
    {
        Player* player = dynamic_cast<Player*>(this);
        player->_ApplyAllItemMods();
    }

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    if (IsPlayer())
    {
        Player* player = dynamic_cast<Player*>(this);
        player->_RemoveAllItemMods();
    }
    _RemoveAllAuraStatMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

