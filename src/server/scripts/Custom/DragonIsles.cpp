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
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            // Summon Vrykul Flag
            GetCaster()->SummonCreature(52022, GetCaster()->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0);
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_vrykul_flag_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_vrykul_flag_AuraScript();
    }
};

void AddSC_DragonIsles()
{
    new spell_vrykul_flag();
}
