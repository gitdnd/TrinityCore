#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "Item.h"
#include "Player.h"
#include "Spell.h"
#include "Unit.h"

#define SpellHoTMod(id, HoTmod) Aura* aura = GetAura(id);               \
if (!aura)                                                              \
    aura = AddAura(id, this);                                           \
aura->GetEffect(EFFECT_0)->SetAmount(HoTmod);


void Unit::DoOnAuraStackScripts(Aura* aura, int16 amount)
{
    CallScriptIteration(CallScriptOnAuraStack(aura, amount));
}
void Unit::DoBeforeSpellCastScripts(Spell* spell)
{
    CallScriptIteration(CallScriptBeforeSpellCast(spell));
}

void Unit::CastSpellFromSupport(Spell* spell, uint32 id)
{
    TriggerCastFlags static const flags =
        TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_CAST_DIRECTLY |
                         TRIGGERED_IGNORE_SET_FACING | TRIGGERED_DONT_REPORT_CAST_ERROR);
    if (spell->m_targets.GetUnitTarget())
        CastSpell(spell->m_targets.GetUnitTarget(), id, flags);
    else if (WorldLocation const* pos = spell->m_targets.GetDstPos())
        CastSpell(CastSpellTargetArg(spell->m_targets.GetDstPos()->GetPosition()), id, flags);
}

SpellSupports::SupportPhase SpellSupports::ConvertEventToPhase(uint32 event)
{
    switch (event)
    {
        case SPELL_EFFECT_HANDLE_HIT:
            return SUP_PHASE_ON_HIT;
        default:
            return SupportPhase::NONE;
    }
    return SupportPhase::NONE;
}
void SpellSupports::DoSupport(Spell* spell, uint32 phase)
{
    if (!(ConvertEventToPhase(phase) & Phase))
        return;

    auto it = AllSpellSupports.find(SpellSupportFunction);
    if (it != AllSpellSupports.end())
    {
        SpellSupportFunc func = it->second;
        (this->*func)(spell); // Correct way to call a member function pointer
    }
}
bool SpellSupports::ProcGeneric(Spell* spell, float chance)
{
    if (frand(1.f, 0.f) >= chance)
    {
        Owner->CastSpellFromSupport(spell, SupportData);
        return true;
    }
    return false;
}
bool SpellSupports::Proc20Pct(Spell* spell)
{
    return ProcGeneric(spell, 0.2f);
}
bool SpellSupports::Proc30Pct(Spell* spell)
{
    return ProcGeneric(spell, 0.3f);
}
bool SpellSupports::Proc40Pct(Spell* spell)
{
    return ProcGeneric(spell, 0.4f);
}
bool Unit::ModSpellSupport(uint32 spellSupport, uint32 supportData, uint32 phase, uint32 spell, bool add)
{
    if (spell)
    {
        if (add)
        {
            if (!GemSupports.count(spell))
                GemSupports.emplace(spell, SpellSupports(this, spellSupport, supportData, phase));
            GemSupports[spell].Amount++;
            return true;
        }
        else
        {
            if (!GemSupports.count(spell))
                return true;
            SpellSupports* ss = &GemSupports[spell];
            ss->Amount--;
            if (!ss->Amount)
            {
                GemSupports.erase(spell);
                return true;
            }
            return false;
        }
    }
    else
    {
        auto it =
            std::find_if(GenericSupports.begin(), GenericSupports.end(),
                         [spellSupport, supportData, phase](const SpellSupports& genSup)
            { return genSup.SpellSupportFunction == spellSupport && genSup.SupportData == supportData, genSup.Phase == phase; });
        if (add)
        {
            if (it == GenericSupports.end())
            {
                GenericSupports.push_back(SpellSupports(this, spellSupport, supportData, phase));
            }
            it->Amount++;
            return true;
        }
        else
        {
            if (it == GenericSupports.end())
            {
                return true;
            }
            it->Amount--;
            if (!it->Amount)
            {
                GenericSupports.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}
void Unit::RecountSpellSupports()
{
    GemSupports.clear();
    GenericSupports.clear();
    if (Player* player = ToPlayer())
    {
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
            if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (auto& supp : pItem->h_allSupportGems)
                {
                    ModSpellSupport(supp.SupportType, supp.SecondData, supp.Spell, supp.Phase);
                }
    }
    else
    {
        // code for mobs that use support system
        return;
    }
}
#define HoTMod(a, b, c, d) SimpleHoTStat[a] -= SimpleHoTStat[b];        \
SimpleHoTStat[b] =                                                      \
    GetHoTModAmount(c) * d;                                             \
SimpleHoTStat[a] += SimpleHoTStat[b];                                   

float Unit::GetArmorReduction()
{
    return 1.f - (((ARMOR_DMG_RED) * (float)GetArmor()) / (1 + ARMOR_DMG_RED * abs((float)GetArmor())));
}
void Unit::HoTModDamageTaken(Stats stat)
{
    HoTMod(HoTStat::DAMAGE_TAKEN_TOTAL, HoTStat(int32(HoTStat::DAMAGE_TAKEN_STR) + stat), HoTMods(UNIT_MOD_DMG_TAKEN_PER_100_STR + stat), (GetStat(Stats(STAT_STRENGTH + stat)) / 100.f));
}
void Unit::HoTModDamageDone(Stats stat)
{
    HoTMod(HoTStat::DAMAGE_DONE_TOTAL, HoTStat(int32(HoTStat::DAMAGE_DONE_STR) + stat), HoTMods(UNIT_MOD_DMG_DONE_PER_100_STR + stat), (GetStat(Stats(STAT_STRENGTH + stat)) / 100.f));
}
void Unit::HoTModPhysReducesEle(SpellSchools school)
{
    HoTMod(HoTStat::PHYS_RED_ELE_TOTAL, HoTStat(int32(HoTStat::PHYS_RED_HOLY) + school - 1),
           HoTMods(UNIT_MOD_PHYS_REDUCES_HOLY + school - 1), GetArmorReduction());
}
void Unit::HoTModEleReducesPhys(SpellSchools school)
{
    HoTMod(HoTStat::ELE_RED_PHYS_TOTAL, HoTStat(int32(HoTStat::HOLY_RED_PHYS) + school - 1), HoTMods(UNIT_MOD_HOLY_REDUCES_PHYS + school - 1), GetResistance(school));
}
void Unit::HoTModEleReducesEle(SpellSchools school)
{
    HoTMod(HoTStat::ELE_RED_ELE_TOTAL, HoTStat(int32(HoTStat::HOLY_RED_ELE) + school - 1), HoTMods(UNIT_MOD_HOLY_REDUCES_ELEMENTAL + school - 1), GetResistance(school));
}
float Unit::GetHoTModAmount(HoTMods unitMod)
{
    return TotalHoTMod[unitMod];
}
float Unit::GetHoTStatAmount(HoTStat unitStat)
{
    return SimpleHoTStat[unitStat];
}
void Unit::UpdateHoTMod(HoTMods unitMod, float amount, bool multi)
{
    if (multi)
        BaseHoTMod[unitMod] += amount;
    else
        MultiHoTMod[unitMod] += amount;
    TotalHoTMod[unitMod] = BaseHoTMod[unitMod] * (1.f + MultiHoTMod[unitMod] / 100.f);
    switch (unitMod)
    {
        case UNIT_MOD_EVASION:
            SimpleHoTStat[HoTStat::EVASION] = TotalHoTMod[UNIT_MOD_EVASION] * (1.f + GetStat(STAT_AGILITY) / 100.f);
            break;
        case UNIT_MOD_WINFDURY:
            break;
        case UNIT_MOD_RESISTANCE_HOLY_MAX:
            SimpleHoTStat[HoTStat::MAX_HOLY_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_RESISTANCE_FIRE_MAX:
            SimpleHoTStat[HoTStat::MAX_FIRE_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_RESISTANCE_NATURE_MAX:
            SimpleHoTStat[HoTStat::MAX_NATURE_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_RESISTANCE_FROST_MAX:
            SimpleHoTStat[HoTStat::MAX_FROST_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_RESISTANCE_SHADOW_MAX:
            SimpleHoTStat[HoTStat::MAX_SHADOW_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_RESISTANCE_ARCANE_MAX:
            SimpleHoTStat[HoTStat::MAX_ARCANE_RESIST] = BASE_MAX_RESIST + TotalHoTMod[unitMod];
            break;  
        case UNIT_MOD_MELEE_CLEAVE_CHANCE:
            break;
        case UNIT_MOD_SPELL_CLEAVE_CHANCE:
            break;
        case UNIT_MOD_SHOOT_CLEAVE_CHANCE:
            break;
        case UNIT_MOD_BONESHIELD_WHEN_HIT_CHANCE:
            break;
        case UNIT_MOD_BONESHIELD_MAX:
            SimpleHoTStat[HoTStat::BONESHIELD_CHARGE_MAX] = BASE_BONESHIELD_CHARGE_MAX + TotalHoTMod[unitMod];
            break;
        case UNIT_MOD_BLOCK_AMOUNT:
        case UNIT_MOD_PARRY_CHANCE:
        case UNIT_MOD_PARRY_CHANCE_MAX:
            break;
        case UNIT_MOD_CAST_RANGE:
        {
            SpellHoTMod(CAST_RANGE_SPELL_ID, TotalHoTMod[unitMod]);
            break;
        }
        case UNIT_MOD_AOE:
        {
            SpellHoTMod(AOE_SPELL_ID, TotalHoTMod[unitMod]);
            break;
        }
        case UNIT_MOD_HEALTH_REGEN:
        case UNIT_MOD_CONVERT_HALF_ATTACK_TO_ELEMENT:
            break;
        case UNIT_MOD_PHYS_REDUCES_HOLY:
        case UNIT_MOD_PHYS_REDUCES_FIRE:
        case UNIT_MOD_PHYS_REDUCES_NATURE:
        case UNIT_MOD_PHYS_REDUCES_FROST:
        case UNIT_MOD_PHYS_REDUCES_SHADOW:
        case UNIT_MOD_PHYS_REDUCES_ARCANE:
            HoTModPhysReducesEle(SpellSchools(UNIT_MOD_PHYS_REDUCES_HOLY - unitMod + 1));
            break;
        case UNIT_MOD_HOLY_REDUCES_PHYS:
        case UNIT_MOD_FIRE_REDUCES_PHYS:
        case UNIT_MOD_NATURE_REDUCES_PHYS:
        case UNIT_MOD_FROST_REDUCES_PHYS:
        case UNIT_MOD_SHADOW_REDUCES_PHYS:
        case UNIT_MOD_ARCANE_REDUCES_PHYS:
            HoTModEleReducesPhys(SpellSchools(UNIT_MOD_PHYS_REDUCES_HOLY - unitMod + 1));
            break;
        case UNIT_MOD_HOLY_REDUCES_ELEMENTAL:
        case UNIT_MOD_FIRE_REDUCES_ELEMENTAL:
        case UNIT_MOD_NATURE_REDUCES_ELEMENTAL:
        case UNIT_MOD_FROST_REDUCES_ELEMENTAL:
        case UNIT_MOD_SHADOW_REDUCES_ELEMENTAL:
        case UNIT_MOD_ARCANE_REDUCES_ELEMENTAL:
            HoTModEleReducesEle(SpellSchools(UNIT_MOD_PHYS_REDUCES_HOLY - unitMod + 1));
            break;
        case UNIT_MOD_PCT_INC_STRENGTH:
        case UNIT_MOD_PCT_INC_AGILITY:
        case UNIT_MOD_PCT_INC_STAMINA:
        case UNIT_MOD_PCT_INC_SPIRIT:
        case UNIT_MOD_PCT_INC_INTELLECT:
        case UNIT_MOD_MANA_SHIELD:
        case UNIT_MOD_GROUND_SPELL_WHILE_STATIONARY:
        case UNIT_MOD_HORNS:
        case UNIT_MOD_REFLECT_ELEMENT:
        case UNIT_MOD_MAX_CURSES:
        case UNIT_MOD_MAX_MAGE_ARMORS:
        case UNIT_MOD_IGNITE_CHANCE:
        case UNIT_MOD_SHOCK_CHANCE:
        case UNIT_MOD_FREEZE_CHANCE:
        case UNIT_MOD_SANCTIFY_CHANCE:
        case UNIT_MOD_DESECRATE_CHANCE:
        case UNIT_MOD_RESERVATION:

        case UNIT_MOD_GHOUL_MAX:
        case UNIT_MOD_SERPENT_WARD_MAX:
        case UNIT_MOD_SPIRIT_WOLF_MAX:
        case UNIT_MOD_WATER_ELEMENTAL_MAX:
        case UNIT_MOD_LAVA_SPAWN_MAX:
        case UNIT_MOD_SKELETON_MAX:
        case UNIT_MOD_HAWK_MAX:
        case UNIT_MOD_GARGOYLE_MAX:
            
        case UNIT_MOD_MINION_STR_INHERITANCE:
        case UNIT_MOD_MINION_AGI_INHERITANCE:
        case UNIT_MOD_MINION_STA_INHERITANCE:
        case UNIT_MOD_MINION_INT_INHERITANCE:
        case UNIT_MOD_MINION_SPI_INHERITANCE:
        case UNIT_MOD_MINION_BONUS_RESERVATION:

        case UNIT_MOD_EXECUTE_DAMAGE_PCT:
        case UNIT_MOD_PILLAGE:
        case UNIT_MOD_EXPLODE_20_HOLY:
        case UNIT_MOD_EXPLODE_20_FIRE:
        case UNIT_MOD_EXPLODE_20_NATURE:
        case UNIT_MOD_EXPLODE_20_FROST:
        case UNIT_MOD_EXPLODE_20_SHADOW:
        case UNIT_MOD_EXPLODE_20_PHYS:
        case UNIT_MOD_RETALIATE_PCT:
        case UNIT_MOD_CORRUPT_HEALTH_PCT:
        case UNIT_MOD_CORRUPT_MANA_PCT:

            break;
        case UNIT_MOD_DMG_TAKEN_PER_100_STR:
        case UNIT_MOD_DMG_TAKEN_PER_100_AGI:
        case UNIT_MOD_DMG_TAKEN_PER_100_STA:
        case UNIT_MOD_DMG_TAKEN_PER_100_INT:
        case UNIT_MOD_DMG_TAKEN_PER_100_SPI:
            HoTModDamageTaken(Stats(unitMod - UNIT_MOD_DMG_TAKEN_PER_100_STR));
            break;
        case UNIT_MOD_DMG_DONE_PER_100_STR:
        case UNIT_MOD_DMG_DONE_PER_100_AGI:
        case UNIT_MOD_DMG_DONE_PER_100_STA:
        case UNIT_MOD_DMG_DONE_PER_100_INT:
        case UNIT_MOD_DMG_DONE_PER_100_SPI:
            HoTModDamageDone(Stats(unitMod - UNIT_MOD_DMG_TAKEN_PER_100_STR));
            break;
        case UNIT_MOD_SHADOW_DANCE_AFTER_STEALTH:
        case UNIT_MOD_MIND_CHARGE_MAX:
        case UNIT_MOD_MIND_CHARGE_ON_HIT:
        case UNIT_MOD_ENRAGE_CHARGE_MAX:
        case UNIT_MOD_ENRAGE_CHARGE_ON_HIT:
        case UNIT_MOD_ENRAGE_CHARGE_WHEN_HIT:
        case UNIT_MOD_ENRAGE_CHARGE_ON_KILL:
        case UNIT_MOD_LIFESTEAL_3_PCT_RESERVE:
        case UNIT_MOD_LIFESTEAL_5_PCT_RESERVE:
        case UNIT_MOD_SHIV_CHARGE_MAX:
        case UNIT_MOD_ON_SHIV_POISON:
        case UNIT_MOD_ON_SHIV_REND:
        case UNIT_MOD_REAPER_DISCHARGE_ON_HIT:
        case UNIT_MOD_REAPER_THUNDER_ON_HIT:
        case UNIT_MOD_REAPER_LEECH_ON_HIT:
        case UNIT_MOD_REAPER_POISON_ON_HIT:
        case UNIT_MOD_REAPER_THORNY_ON_HIT:
        case UNIT_MOD_SOULSTEAL_PCT:
        case UNIT_MOD_DRAIN_HEALTH_ON_HIT:
        case UNIT_MOD_DRAIN_MANA_ON_HIT:
        case UNIT_MOD_ON_MAX_HEALTH_GAIN_STR:
        case UNIT_MOD_ON_MAX_HEALTH_GAIN_AGI:
        case UNIT_MOD_ON_MAX_HEALTH_GAIN_STA:
        case UNIT_MOD_ON_MAX_HEALTH_GAIN_INT:
        case UNIT_MOD_ON_MAX_HEALTH_GAIN_SPI:
        case UNIT_MOD_ON_MAX_MANA_GAIN_STR:
        case UNIT_MOD_ON_MAX_MANA_GAIN_AGI:
        case UNIT_MOD_ON_MAX_MANA_GAIN_STA:
        case UNIT_MOD_ON_MAX_MANA_GAIN_INT:
        case UNIT_MOD_ON_MAX_MANA_GAIN_SPI:
        case UNIT_MOD_HEALTH_ON_SHOUT:
        case UNIT_MOD_EXERT_ATTACKS_ON_SHOUT:
        case UNIT_MOD_ENERGY_BARRIER_ON_HIT:
        case UNIT_MOD_ENERGY_BARRIER_DURATION:
        case UNIT_MOD_MINIMUM_ENERGY_BARRIER:
        case UNIT_MOD_PROC_CHANCE_MULTI_PCT:
        case UNIT_MOD_MOVE_WHILE_CASTING_SPEED:
        case UNIT_MOD_BRAWL:
        case UNIT_MOD_SANDIFY_KILL_PCT:
        case UNIIT_MOD_VERSATILITY:
        case UNIT_MOD_BERSERK:
        case UNIT_MOD_ANTI_DOT:
        case UNIT_MOD_SLOW_RESIST:
        case UNIT_MOD_STATUS_RESIST:
        case UNIT_MOD_OIL_CHANCE:
        case UNIT_MOD_GUARDIAN_SPRINT:
        case UNIT_MOD_RESILIENCE:
        case UNIT_MOD_RESILIENCE_TOKEN_MAX:
        case UNIT_MOD_SPAMCRIT_MAX:
        case UNIT_MOD_CHAIN_REACTION:
        case UNIT_MOD_CHARISMA:
            break;
        case UNIT_MOD_MANA_BATTERY:
            SimpleHoTStat[HoTStat::MANABATTERY_PROGRESS] = 0;
            SimpleHoTStat[HoTStat::MANABATTERY_CURRENT]  = 0;
            break;
        case UNIT_MOD_EMBERS:
            if (Player* p = ToPlayer())
                p->ClearComboPoints();
            break;
        case UNIT_MOD_RUNES:
        case UNIT_MOD_UNHOLYNESS:
        case UNIT_MOD_CORRUPTION_RESIST:
        default:
            break;
    } 
}
