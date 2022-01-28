#include "elk_spell_script_include.h"


class elk_dk_undeath_strike : public ElkSpellScript
{
    PrepareSpellScript(elk_dk_undeath_strike);

    void Register() override
    {
        AfterCast += SpellCastFn(elk_dk_undeath_strike::ForceAttack);
    }
};

class elk_dk_death_coil : public ElkSpellScript
{
    PrepareSpellScript(elk_dk_death_coil);

    Unit* caster;
    int   damage = 50;
    void  Recasting()
    {
        Spell* spell = GetSpell();
        auto dcSpell = caster->FindCurrentSpellBySpellId(SPELL_DK_DEATH_COIL_DAMAGE);
        if (dcSpell && dcSpell->GetSpellTimer() > 0 && (caster->HasSpell(90003) || (caster->GetCreatureType() == CREATURE_TYPE_UNDEAD)))
        {
            caster->ModifyPower(POWER_MANA, 250);
            int castTime = dcSpell->GetSpellTimer();
            dcSpell->cancel();

            //CustomSpellValues values;
            //values.AddSpellMod(SPELLVALUE_BASE_POINT0, damage);
            //caster->CastCustomSpell(SPELL_DK_DEATH_COIL_HEAL, values, caster, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);



            // >>>> //caster->CastSpell(caster, SPELL_DK_DEATH_STRIKE_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, heal));

            auto dcSpell2 = caster->FindCurrentSpellBySpellId(SPELL_DK_DEATH_COIL_HEAL);
            dcSpell2->SetSpellTimer(castTime);
        }
        else
        {
            auto  dcSpell2 = caster->FindCurrentSpellBySpellId(SPELL_DK_DEATH_COIL_HEAL);
            if (dcSpell2)
                return;
            Unit* target = GetExplTargetUnit();
            if (target)
            {
                if (caster->IsFriendlyTo(target))
                {
                    //caster->CastSpell(target, SPELL_DK_DEATH_COIL_HEAL, &damage, nullptr, nullptr, false);
                }
                else
                {
                    //caster->CastCustomSpell(target, SPELL_DK_DEATH_COIL_DAMAGE, &damage, nullptr, nullptr, false);
                }
            }
        }
    }
    SpellCastResult CheckCast()
    {
        caster = GetCaster();
        if (Unit* target = GetExplTargetUnit())
        {
            if (!caster->IsFriendlyTo(target) && !caster->isInFront(target))
                return SPELL_FAILED_UNIT_NOT_INFRONT;

            bool undead = target->HasSpell(90003);

            if (target->IsFriendlyTo(caster))
            {
                if (target->GetCreatureType() != CREATURE_TYPE_UNDEAD && !undead)
                    return SPELL_FAILED_BAD_TARGETS;
                if (target != caster && (caster->GetCreatureType() == CREATURE_TYPE_UNDEAD || caster->HasSpell(90003)))
                {
                    GetSpell()->m_targets.SetUnitTarget(caster);
                    return SPELL_CAST_OK;
                }
            }
        }
        else
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }
    void Register() override
    {
        BeforeCastTime += SpellCastFn(elk_dk_death_coil::Recasting);
        OnCheckCast += SpellCheckCastFn(elk_dk_death_coil::CheckCast);
    }
};

class elk_dk_lich_touch : public ElkSpellScript
{
    PrepareSpellScript(elk_dk_lich_touch);
};
class elk_dk_death_and_decay : public ElkSpellScript
{
    PrepareSpellScript(elk_dk_death_and_decay);
};

class spell_dk_death_coil_damage : public ElkSpellScript
{
    PrepareSpellScript(spell_dk_death_coil_damage);

    void SpellHit()
    {
        Unit* target = GetHitUnit();
        if (target->isDead())
        {
            Unit* caster = GetCaster();
            // caster->CastSpell(caster, elk_dk_DEATH_COIL_KILLS, true);
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_death_coil_damage::SpellHit);
    }

};

class elk_dk_dark_strangulate : public ElkSpellScript
{
    PrepareSpellScript(elk_dk_dark_strangulate);

    void SpellCast()
    {
        Unit* target = GetHitUnit();
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(elk_dk_dark_strangulate::SpellCast);
    }
};

void AddSC_elk_dk_scripts()
{

    // RegisterSpellScript(elk_dk_spell_scripts);

}
