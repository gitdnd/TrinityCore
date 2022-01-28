#include "Spell.h"
#include "Group.h"
#include "PetDefines.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Totem.h"
#include "UnitAI.h"
#include <MoveSplineInit.h>
#include "Containers.h"
#include "ObjectMgr.h"
#include "SpellHistory.h"
#include <numeric>

enum OutsideSpells
{

    SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE = 91004,
    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING = 91005,
    SPELL_DK_DEATH_COIL_DAMAGE = 90008,
    SPELL_DK_DEATH_COIL_HEAL = 90009,
};
enum ActionSpells
{
    SPELL_ACTION_STAR_JUMP_TWO = 89978,
    SPELL_ACTION_STAR_JUMP_ONE = 89979,
    SPELL_ACTION_STAR_JUMP = 89980,
    SPELL_ACTION_AIR_DODGE = 89981,
    SPELL_ACTION_AIR_DEFLECT_HIT = 89982,
    SPELL_ACTION_AIR_DEFLECT = 89983,
    SPELL_ACTION_SLAM_DUNK = 89984,
    SPELL_ACTION_HARD_ATTACK = 89900,
    SPELL_ACTION_HEAVY_ATTACK = 89901,
    SPELL_ACTION_DIVINE_BLOOD = 89985,
    SPELL_ACTION_HEALING_POTION = 89986,
    SPELL_ACTION_POTION_FATIGUE = 89987,
    SPELL_ACTION_POTION = 89988,
    SPELL_ACTION_ATTACK_SLOW_DEBUFF = 89989,
    SPELL_ACTION_POSTURE_STUN = 89990,
    SPELL_ACTION_POSTURE_DAMAGE = 89991,
    SPELL_ACITON_QUICK_FOLLOW_UP = 89992,
    SPELL_ACITON_QUICK_ATTACK = 89993,
    SPELL_ACITON_ATTACK_SLOW = 89994,
    SPELL_ACITON_ATTACK_SLOW_HEAVY = 89995,
    SPELL_ACTION_THRUST_ATTACK = 89996,
    SPELL_ACTION_STAB_ATTACK = 89997,
    SPELL_ACTION_BASIC_ATTACK = 89998,
    SPELL_ACTION_DEFLECT = 89999,
    SPELL_ACTION_ATTACK = 90000,
    SPELL_ACTION_UNDEATH_STRIKE = 90001,
    SPELL_ACTION_RAISE_GHOUL = 90002,
    SPELL_ACTION_DEATH_COIL = 90004,
    SPELL_ACTION_FESTERING_PLAGUE = 90005,
    SPELL_ACTION_DEATH_COIL_KILLS = 90006,
    SPELL_ACTION_PRESENCE_OF_SOUL = 91001,
    SPELL_ACTION_HOLY_SHOCK = 91002,
    SPELL_ACTION_CRUSADER_STRIKE = 91003,
    SPELL_ACTION_THUNDER_CLAP = 92003,

    SPELL_ACTION_WRATH = 96001,
    SPELL_ACTION_WRATH_DUMMY = 96002,
    SPELL_ACTION_WRATH_READY = 96003,

    SPELL_ACTION_HEAL = 97001,
    SPELL_ACTION_FLASH_HEAL = 97002,
    SPELL_ACTION_HEAL_FLASH_HEAL = 97003,

    SPELL_ACTION_LEAP = 100001,
    SPELL_ACTION_BLOOD_HOWL = 100003,
    SPELL_WOLF_FORM = 100004,
    SPELL_ACTION_WOLF_BITE = 100005,
    SPELL_ACTION_SHOCKWAVE_GRAPHIC = 100007,
    SPELL_ACTION_MASSIVE_SMASH = 100008,
};

class ElkSpellScript : public SpellScript
{
protected:
    void ForceAttack()
    {
        Unit* unitCaster = GetCaster();
        Position         CasterPnt = unitCaster->GetWorldLocation();
        const SpellInfo* spellInfo = GetSpellInfo();

        HandleAtkSpell(unitCaster);

        unitCaster->CastSpell(unitCaster, SPELL_ACITON_ATTACK_SLOW, true);
        unitCaster->CastSpell(unitCaster, SPELL_ACTION_BASIC_ATTACK, false);


        Spell* spell = unitCaster->FindCurrentSpellBySpellId(SPELL_ACTION_BASIC_ATTACK);
        HandleTriggerDummy(spell);
        HandleAttackCD(unitCaster, spellInfo);
    }

    void ForceHeavy()
    {
        Unit* unitCaster = GetCaster();
        Position         CasterPnt = unitCaster->GetWorldLocation();
        const SpellInfo* spellInfo = GetSpellInfo();

        HandleAtkSpell(unitCaster);

        unitCaster->CastSpell(unitCaster, SPELL_ACITON_ATTACK_SLOW_HEAVY, true);
        unitCaster->CastSpell(unitCaster, SPELL_ACTION_HEAVY_ATTACK, false);

        Spell* spell = unitCaster->FindCurrentSpellBySpellId(SPELL_ACTION_HEAVY_ATTACK);
        HandleTriggerDummy(spell);
        HandleAttackCD(unitCaster, spellInfo);
    }

    void ForceQuick()
    {
        Unit* unitCaster = GetCaster();
        Position         CasterPnt = unitCaster->GetWorldLocation();
        const SpellInfo* spellInfo = GetSpellInfo();

        HandleAtkSpell(unitCaster);

        unitCaster->CastSpell(unitCaster, SPELL_ACITON_ATTACK_SLOW, true);
        unitCaster->CastSpell(unitCaster, SPELL_ACITON_QUICK_FOLLOW_UP, false);

        Spell* spell = unitCaster->FindCurrentSpellBySpellId(SPELL_ACITON_QUICK_FOLLOW_UP);
        HandleTriggerDummy(spell);
        HandleAttackCD(unitCaster, spellInfo);
    }

    void ForceThrust()
    {
        Unit* unitCaster = GetCaster();
        Position         CasterPnt = unitCaster->GetWorldLocation();
        const SpellInfo* spellInfo = GetSpellInfo();

        HandleAtkSpell(unitCaster);

        unitCaster->CastSpell(unitCaster, SPELL_ACITON_ATTACK_SLOW_HEAVY, true);
        unitCaster->CastSpell(unitCaster, SPELL_ACTION_THRUST_ATTACK, false);

        Spell* spell = unitCaster->FindCurrentSpellBySpellId(SPELL_ACTION_THRUST_ATTACK);
        HandleTriggerDummy(spell);
        HandleAttackCD(unitCaster, spellInfo);
    }

    void HandleTriggerDummy(Spell*& spell)
    {
        if (spell)
        {
            uint32 spellId = (GetSpellInfo()->Id);
            auto& spellMap = spell->GetTriggerDummy();
            spellMap[MapDummy::TriggeringSpell] = spellId;
        }
    }
    void HandleAtkSpell(Unit*& unitCaster)
    {
        if (unitCaster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            unitCaster->CastSpell(unitCaster, SPELL_ACTION_SLAM_DUNK, true);
            auto& spellMap = GetSpell()->GetTriggerDummy();
            spellMap[MapDummy::WasInAir] = true;
        }
    }
    void HandleAttackCD(Unit*& unitCaster, const SpellInfo* spellInfo)
    {
        int32 atkTime = unitCaster->GetBaseAttackTime(BASE_ATTACK);
        atkTime *= unitCaster->m_modAttackSpeedPct[BASE_ATTACK];
        int32 CD;
        if (unitCaster->haveOffhandWeapon() == true)
        {
            int32 atkTime2 = unitCaster->GetBaseAttackTime(OFF_ATTACK);
            atkTime2 *= unitCaster->m_modAttackSpeedPct[OFF_ATTACK];
            CD = (atkTime + atkTime2) / 2;
        }
        else
        {
            CD = atkTime;
        }
        auto spellMap = GetSpell()->GetTriggerDummy();
        if (spellMap[MapDummy::WasInAir].has_value())
        {
            if (std::any_cast<bool>(spellMap[MapDummy::WasInAir].value()) == true)
                CD += 2000;
        }
        /*

        uint32 category = spellInfo->GetCategory();

        unitCaster->_AddSpellCooldown(spellInfo->Id, category, 0, CD, true, true);
        if (unitCaster->GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data;
            Player* tempPlayer = dynamic_cast<Player*>(unitCaster);

            tempPlayer->BuildCooldownPacket(data, SPELL_COOLDOWN_FLAG_NONE, spellInfo->Id, CD);
            tempPlayer->SendDirectMessage(&data);
        }

        SpellCategoryStore::const_iterator i_scstore = sSpellsByCategoryStore.find(category);
        if (i_scstore != sSpellsByCategoryStore.end())
        {
            for (SpellCategorySet::const_iterator i_scset = i_scstore->second.begin(); i_scset != i_scstore->second.end(); ++i_scset)
            {
                if (i_scset->second == spellInfo->Id) // skip main spell, already handled above
                {
                    continue;
                }

                // Only within the same spellfamily
                SpellInfo const* categorySpellInfo = sSpellMgr->GetSpellInfo(i_scset->second);
                if (!categorySpellInfo || categorySpellInfo->SpellFamilyName != spellInfo->SpellFamilyName)
                {
                    continue;
                }

                unitCaster->_AddSpellCooldown(i_scset->second, category, 0, CD, true);
            }
        }


        */
    }
};
