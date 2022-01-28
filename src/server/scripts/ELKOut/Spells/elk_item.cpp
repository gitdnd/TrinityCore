#include "action_include.h"

class spell_action_divine_blood : public SpellScript
{
    PrepareSpellScript(spell_action_divine_blood);

    void PotionFatigue()
    {
        auto  caster = GetCaster();
        Aura* potionFatigue = caster->GetAura(SPELL_ACTION_POTION_FATIGUE);
        if (potionFatigue)
        {
            potionFatigue->Remove();
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_action_divine_blood::PotionFatigue);
    }
};

class spell_action_potions : public SpellScript
{
    PrepareSpellScript(spell_action_potions);

    void PotionFatigue()
    {
        auto item = GetCastItem();
        if (!item)
        {
            return;
        }
        Unit* caster = GetCaster();

        Aura* potionFatigue = caster->GetAura(SPELL_ACTION_POTION_FATIGUE);
        int   potionFatStacks = 0;
        if (potionFatigue)
        {
            potionFatStacks = potionFatigue->GetStackAmount();
            potionFatStacks = std::min(potionFatStacks, 10);
        }
        caster->CastSpell(caster, SPELL_ACTION_POTION_FATIGUE, true);

        auto potionEnum = 0;
        switch (item->GetEntry())
        {
        case 60002:
        {
            potionEnum = SPELL_ACTION_HEALING_POTION;
            break;
        }
        default:
            break;
        }
        if (potionEnum)
        {
            const SpellInfo* si = sSpellMgr->GetSpellInfo(potionEnum);
            if (!si)
                return;

            caster->CastSpell(caster, SPELL_ACITON_ATTACK_SLOW, true);
            CustomSpellValues values;
            int32             effectiveness = si->Effects[0].CalcValue() * (1 - 0.1 * potionFatStacks);
            values.AddSpellMod(SPELLVALUE_BASE_POINT0, effectiveness);
            caster->CastCustomSpell(potionEnum, values, caster, TRIGGERED_NONE);
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_action_potions::PotionFatigue);
    }
};

void SC_item_scripts()
{

    RegisterSpellScript(spell_action_divine_blood);
    RegisterSpellScript(spell_action_potions);
}
