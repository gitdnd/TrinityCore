#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Item.h"
#include <GridNotifiers.h>

class spell_affix_avenging_wrath_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_avenging_wrath_aura);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        auto caster = GetCaster();
        if (!caster || !GetSpellInfo())
            return;

        auto baseAura = aurEff->GetBase();
        if (!baseAura)
            return;

        uint32 stackAmount = baseAura->GetStackAmount();
        float range = 30.0f;

        std::list<Creature*> creatureList;
        Trinity::AnyUnitInObjectRangeCheck go_check(caster, range);
        Trinity::CreatureListSearcher<Trinity::AnyUnitInObjectRangeCheck> go_search(caster, creatureList, go_check);
        Cell::VisitGridObjects(caster, go_search, range);

        uint32 spellId = GetSpellInfo()->Effects[0].TriggerSpell;
        for (std::list<Creature*>::const_iterator it = creatureList.begin(); it != creatureList.end(); ++it)
        {
            if (caster->GetFactionReactionTo((*it)->GetFactionTemplateEntry(), *it) >= REP_NEUTRAL)
            {
                for (int i = 0; i < stackAmount; ++i)
                {
                    if ((*it)->HasAura(spellId))
                    {
                        if (Aura* aura = (*it)->GetAura(spellId))
                            aura->SetStackAmount(aura->GetStackAmount() + 1);
                    }
                    else
                        caster->CastSpell(*it, spellId);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_affix_avenging_wrath_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_affix_arcane_unleashed_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_arcane_unleashed_aura);

    void OnPeriodicProc(AuraEffect const* aurEff)
    {
        auto caster = GetCaster();
        if (!caster || !GetSpellInfo())
        {
            PreventDefaultAction();
            return;
        }
        auto base = aurEff->GetBase();
        if (!base)
            return;

        if (!roll_chance_i(GetSpellInfo()->ProcChance * base->GetStackAmount()))
        {
            PreventDefaultAction();
            return;
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_arcane_unleashed_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_affix_barkskin_spores_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_barkskin_spores_aura);

    void OnPeriodicProc(AuraEffect const* aurEff)
    {
        auto caster = GetCaster();
        if (!caster || !caster->ToCreature() || !aurEff->GetBase())
        {
            PreventDefaultAction();
            return;
        }
        auto creature = caster->ToCreature();
        auto aura = aurEff->GetBase();
        for (int i = 0; i < aura->GetStackAmount(); ++i)
        {
            if (creature->GetCreatureTemplate()->rank == CREATURE_ELITE_WORLDBOSS &&
                (creature->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_BOSS_MOB) != 0)
            {
                if (i == 0)
                    PreventDefaultAction();

                caster->CastSpell(caster, 460178); // boss buff
            }
            if (aurEff->GetBase()->GetStackAmount() > 1)
            {
                if (i == 0)
                    PreventDefaultAction();

                caster->CastSpell(caster, 460177); // normal buff many times
            }
        }
        // default normal buff
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_barkskin_spores_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_Spells_Custom_Affix()
{
    RegisterAuraScript(spell_affix_avenging_wrath_aura);
    RegisterAuraScript(spell_affix_arcane_unleashed_aura);
    RegisterAuraScript(spell_affix_barkskin_spores_aura);
}
