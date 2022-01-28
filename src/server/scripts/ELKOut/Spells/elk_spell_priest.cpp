#include "action_include.h"



class spell_action_heal_flash_heal : public SpellScript
{
    PrepareSpellScript(spell_action_heal_flash_heal);

    void  Recasting()
    {
        Unit* caster = GetCaster();
        Spell* spell = GetSpell();
        auto  spell2 = caster->FindCurrentSpellBySpellId(SPELL_ACTION_HEAL);
        Unit* target = spell->m_targets.GetUnitTarget();
        if (spell2)
        {
            int castTime = spell2->GetCastTime() - spell2->GetSpellTimer();
            spell2->cancel();
            caster->CastSpell(target, SPELL_ACTION_FLASH_HEAL, false);
            auto spell3 = caster->FindCurrentSpellBySpellId(SPELL_ACTION_FLASH_HEAL);
            spell3->ModifySpellTimer(-1 * castTime);
        }
        else
        {
            auto spell3 = caster->FindCurrentSpellBySpellId(SPELL_ACTION_FLASH_HEAL);
            if (!spell3)
                caster->CastSpell(target, SPELL_ACTION_HEAL, false);
        }
    }
    void Register() override
    {
        BeforeCastTime += SpellCastFn(spell_action_heal_flash_heal::Recasting);
    }
};

void SC_priest_scripts()
{
    RegisterSpellScript(spell_action_heal_flash_heal);
}
