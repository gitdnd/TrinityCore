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

class spell_familiar_split : public SpellScriptLoader
{
public:
    spell_familiar_split() : SpellScriptLoader("spell_familiar_split") { }

    class spell_familiar_split_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_familiar_split_AuraScript);

    private:
        void OnPeriodic(AuraEffect const* aurEff)
        {
            if ((aurEff->GetTickNumber() - 1) % 5)
                GetTarget()->CastSpell(nullptr, GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell, { aurEff, GetCasterGUID() });
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_familiar_split_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_familiar_split_AuraScript();
    }
};

void AddSC_FallOfDalaran()
{
    new spell_familiar_split();
}