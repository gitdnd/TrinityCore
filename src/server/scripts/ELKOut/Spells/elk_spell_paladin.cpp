#include "action_include.h"

class spell_action_holy_shock : public SpellScript
{
    PrepareSpellScript(spell_action_holy_shock);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* unitTarget = GetHitUnit())
        {
            if (caster->IsFriendlyTo(unitTarget))
                caster->CastSpell(unitTarget, SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, true);
            else
                caster->CastSpell(unitTarget, SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, true);
        }
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetExplTargetUnit())
        {
            if (!caster->IsFriendlyTo(target))
            {
                if (!caster->IsValidAttackTarget(target))
                    return SPELL_FAILED_BAD_TARGETS;

                if (!caster->isInFront(target))
                    return SPELL_FAILED_UNIT_NOT_INFRONT;
            }
        }
        else
            return SPELL_FAILED_BAD_TARGETS;
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_action_holy_shock::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_action_holy_shock::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
class spell_action_crusader_strike : public SpellScript
{
    PrepareSpellScript(spell_action_crusader_strike);

    void Register() override
    {
        AfterCast += SpellCastFn(spell_action_crusader_strike::ForceAttack);
    }
};

void SC_paladin_scripts()
{
    RegisterSpellScript(spell_action_holy_shock);
    RegisterSpellScript(spell_action_crusader_strike);
}
