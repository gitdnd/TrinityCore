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

        float range = 30.0f;

        std::list<Unit*> list;

        Trinity::AnyFriendlyUnitInObjectRangeCheck checker(caster, caster, range);
        Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(caster, list, checker);
        Cell::VisitAllObjects(caster, searcher, range);

        uint32 spellId = GetSpellInfo()->Effects[0].TriggerSpell;
        for (std::list<Unit*>::const_iterator it = list.begin(); it != list.end(); ++it)
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

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_affix_avenging_wrath_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

void AddSC_Spells_Custom_Affix()
{
    RegisterAuraScript(spell_affix_avenging_wrath_aura);
}
