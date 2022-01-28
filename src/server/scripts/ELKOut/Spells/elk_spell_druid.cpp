#include "action_include.h"


class spell_action_wrath : public SpellScript
{
    PrepareSpellScript(spell_action_wrath);

    void SpellCast()
    {
        Unit* caster = GetCaster();
        if (caster->HasAura(SPELL_ACTION_WRATH_READY))
        {
            Spell* spell = GetSpell();
            spell->cancel();
            caster->CastSpell(GetExplTargetUnit(), SPELL_ACTION_WRATH_DUMMY, true);
            caster->RemoveAura(SPELL_ACTION_WRATH_READY);
        }
    }
    void SpellAfter()
    {
        Unit* caster = GetCaster();
        caster->AddAura(SPELL_ACTION_WRATH_READY, caster);
    }

    void Register() override
    {
        BeforeCastTime += SpellCastFn(spell_action_wrath::SpellCast);
        AfterCast += SpellCastFn(spell_action_wrath::SpellAfter);
    }
    // could remove the interrupt by making the dummy aura spell a self cast spell and just casting spell 2 if no aura and spell 1 if aura lmao.
    // distance determines damage of wrath
};

class spell_action_wrath_dummy : public SpellScript
{
    PrepareSpellScript(spell_action_wrath_dummy);

    void WrHit()
    {

        Unit* target = GetHitUnit();
        Unit* caster = GetCaster();
        float dist = target->GetDistance(caster);
        SetHitDamage(int(float(GetHitDamage()) / std::max(1.f, (std::sqrt(dist)))));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_action_wrath_dummy::WrHit);
    }
};
class spell_action_healing_touch : public SpellScript
{
    PrepareSpellScript(spell_action_healing_touch);

    Unit* caster;
    Spell* spell;
    void   SpellCast()
    {
        caster = GetCaster();
        spell = GetSpell();
        caster->AddAura(SPELL_ACITON_ATTACK_SLOW, caster);
    }
    void SpellWhile()
    {
        caster->GetAura(SPELL_ACITON_ATTACK_SLOW)->SetDuration(500);
        if (caster->isMoving())
        {
            spell->ModifySpellTimer(getWorldInstance()->GetUpdateTime() / 3);
        }
    }
    void SpellAfter()
    {
        caster->RemoveAura(SPELL_ACITON_ATTACK_SLOW);
    }

    void Register() override
    {
        BeforeCastTime += SpellCastFn(spell_action_healing_touch::SpellCast);
        WhileCast += SpellCastFn(spell_action_healing_touch::SpellWhile);
        AfterCast += SpellCastFn(spell_action_healing_touch::SpellAfter);
    }
};


void SC_druid_scripts()
{
    RegisterSpellScript(spell_action_wrath);
    RegisterSpellScript(spell_action_wrath_dummy);
    RegisterSpellScript(spell_action_healing_touch);
}
