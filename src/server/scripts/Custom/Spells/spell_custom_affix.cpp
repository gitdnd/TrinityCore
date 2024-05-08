#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Item.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "TemporarySummon.h"

class spell_affix_avenging_wrath_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_avenging_wrath_aura);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
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

        uint32 spellId = GetSpellInfo()->_effects[0].TriggerSpell;
        for (std::list<Creature*>::const_iterator it = creatureList.begin(); it != creatureList.end(); ++it)
        {
            if (caster->GetFactionReactionTo((*it)->GetFactionTemplateEntry(), *it) >= REP_NEUTRAL)
            {
                for (uint32 i = 0; i < stackAmount; ++i)
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

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        auto caster = GetCaster();
        if (!caster || !caster->ToCreature() || !aurEff->GetBase())
        {
            return;
        }

        auto creature = caster->ToCreature();
        auto aura = aurEff->GetBase();
        for (int i = 0; i < aura->GetStackAmount(); ++i)
        {
            if (creature->GetCreatureTemplate()->rank == CREATURE_ELITE_WORLDBOSS &&
                (creature->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_BOSS_MOB) != 0)
            {
                caster->CastSpell(caster, 460178); // boss buff
            }
            else
            {
                caster->CastSpell(caster, 460177); // normal buff
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_affix_barkskin_spores_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_affix_ordinance_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_ordinance_aura);

    void OnPeriodicProc(AuraEffect const* aurEff)
    {
        auto caster = GetCaster();
        if (!caster || !caster->ToCreature() || !aurEff->GetBase() || !GetSpellInfo() || !caster->IsInCombat())
        {
            PreventDefaultAction();
            return;
        }
        if (!roll_chance_i(GetSpellInfo()->ProcChance))
        {
            PreventDefaultAction();
            return;
        }
        PreventDefaultAction();
        auto creature = caster->ToCreature();
        auto aura = aurEff->GetBase();
        auto spellId = GetSpellInfo()->_effects[0].TriggerSpell;
        for (int i = 0; i < aura->GetStackAmount(); ++i)
        {
            float dist = frand(5.0f, 20.0f);
            float angle = frand(0.0f, 2.0f) * float(M_PI);
            Position pos = GetCaster()->GetNearPosition(dist, angle);

            creature->CastSpell(pos, spellId);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_ordinance_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_affix_corpse_explosion_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_corpse_explosion_aura);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        auto caster = GetCaster();
        if (!caster || !GetSpellInfo())
            return;

        auto baseAura = aurEff->GetBase();
        if (!baseAura)
            return;

        if (TempSummon* npc = caster->SummonCreature(60215, caster->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 3s))
        {
            npc->SetReactState(REACT_PASSIVE);
            // Repeat for each stack
            for (int i = 0; i < baseAura->GetStackAmount(); ++i)
            {
                npc->CastSpell(npc, GetSpellInfo()->_effects[0].TriggerSpell);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_affix_corpse_explosion_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_affix_mark_of_the_absolute_trigger_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_mark_of_the_absolute_trigger_aura);

    void OnPeriodicProc(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        auto caster = GetCaster();
        if (!caster || !caster->ToCreature() || !aurEff->GetBase() || !GetSpellInfo())
        {
            return;
        }

        if (!caster->ToCreature()->IsMarkOfTheAbsoluteEnabled())
        {
            // Base effect, will remove this aura too
            caster->RemoveAura(460104);
            return;
        }

        // if not in combat, or casting
        if (!caster->IsInCombat() || caster->HasUnitState(0x00008000))
        {
            return;
        }

        if (roll_chance_i(40))
        {
            // Anti-magic Shell
            CastSpellExtraArgs args;
            args.SetTriggerFlags(TRIGGERED_FULL_MASK);
            caster->CastSpell(caster, 7121, args);
        }

        if (roll_chance_i(50))
        {
            // Frostbolt Volley
            caster->CastSpell(caster, 460186);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_mark_of_the_absolute_trigger_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_affix_mark_of_the_absolute_chance_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_mark_of_the_absolute_chance_aura);

    void OnPeriodicProc(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
        auto caster = GetCaster();
        if (!caster || !caster->ToCreature())
        {
            return;
        }

        if (!caster->ToCreature()->IsMarkOfTheAbsoluteEnabled())
        {
            // Base effect, will remove this aura too
            caster->RemoveAura(460104);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_mark_of_the_absolute_chance_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_affix_wild_magic_aura : public AuraScript
{
    PrepareAuraScript(spell_affix_wild_magic_aura);

    void OnPeriodicProc(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        auto caster = GetCaster();
        if (!caster || !caster->ToCreature())
            return;

        caster->Say("Proc 1", LANG_UNIVERSAL);

        auto aura = aurEff->GetBase();
        if (!aura)
            return;

        auto stacks = aura->GetStackAmount();

        caster->Say("Proc 2", LANG_UNIVERSAL);

        if (!roll_chance_i(stacks * 2))
            return;

        caster->Say("Proc 3", LANG_UNIVERSAL);

        float range = 30.0f;

        std::list<Unit*> unitList;
        Trinity::AnyUnitInObjectRangeCheck go_check(caster, range);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> go_search(caster, unitList, go_check);
        Cell::VisitGridObjects(caster, go_search, range);

        uint32 spellId = GetSpellInfo()->_effects[0].TriggerSpell;
        for (std::list<Unit*>::const_iterator it = unitList.begin(); it != unitList.end(); ++it)
        {
            Unit* target = *it;
            target->Say("Proc 4", LANG_UNIVERSAL);
            if (caster->CanSeeOrDetect(target))
            {
                caster->Say("Proc 5", LANG_UNIVERSAL);
                if (Creature* creature = target->ToCreature())
                {
                    if (creature->IsDungeonBoss() ||
                        creature->isWorldBoss() ||
                        creature->IsTrigger() ||
                        creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) ||
                        (creature->GetCreatureTemplate()->type == CREATURE_TYPE_CRITTER) ||
                        (creature->GetCreatureTemplate()->type == CREATURE_TYPE_TOTEM) ||
                        !creature->IsAlive())
                        continue;
                }
                caster->Say("Proc 6", LANG_UNIVERSAL);
                target->CastSpell(target, spellId);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_affix_wild_magic_aura::OnPeriodicProc, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_Spells_Custom_Affix()
{
    RegisterSpellScript(spell_affix_avenging_wrath_aura);
    RegisterSpellScript(spell_affix_arcane_unleashed_aura);
    RegisterSpellScript(spell_affix_barkskin_spores_aura);
    RegisterSpellScript(spell_affix_ordinance_aura);
    RegisterSpellScript(spell_affix_corpse_explosion_aura);
    RegisterSpellScript(spell_affix_mark_of_the_absolute_trigger_aura);
    RegisterSpellScript(spell_affix_mark_of_the_absolute_chance_aura);
    RegisterSpellScript(spell_affix_wild_magic_aura);
}
