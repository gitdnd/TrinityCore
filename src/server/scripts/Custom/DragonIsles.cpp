#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CreatureTextMgr.h"
#include "DBCStores.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "Weather.h"

class spell_vrykul_flag : public SpellScriptLoader
{
public:
    spell_vrykul_flag() : SpellScriptLoader("spell_vrykul_flag") { }

    class spell_vrykul_flag_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vrykul_flag_AuraScript);

    private:
        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            // Summon Vrykul Flag
            if (auto caster = GetCaster())
            {
                // Only summon if not next to drop point, and not directly on another flag
                if (!caster->FindNearestCreature(52024, 5.0f) && !caster->FindNearestCreature(52022, 2.0f))
                {
                    caster->SummonCreature(52022, GetCaster()->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                }
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_vrykul_flag_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    /*class spell_vrykul_flag_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_vrykul_flag_SpellScript);

    private:
        void OnSomething()
        {

        }

        void Register() override
        {
            //OnEffectHitTarget += SpellEffectFn(spell_request_second_mug::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };*/

    AuraScript* GetAuraScript() const override
    {
        return new spell_vrykul_flag_AuraScript();
    }

    /*SpellScript* GetSpellScript() const override
    {
        return new spell_vrykul_flag_SpellScript();
    }*/
};

void AddSC_DragonIsles()
{
    new spell_vrykul_flag();
}
