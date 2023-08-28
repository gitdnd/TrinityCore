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

class spell_icespike_glacier : public SpellScript
{
    PrepareSpellScript(spell_icespike_glacier);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        auto target = GetExplTargetUnit();
        if (!target)
            return;

        target->CastSpell(target, 90522, true);

        auto player = target->ToPlayer();
        if (!player)
            return;

        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        player->SetClientControl(player, 0);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_icespike_glacier::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_aura_icespike_glacier : public SpellScriptLoader
{
public:
    spell_aura_icespike_glacier() : SpellScriptLoader("spell_aura_icespike_glacier") { }

    class spell_aura_icespike_glacier_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_aura_icespike_glacier_AuraScript);

    private:
        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            auto caster = GetCaster();
            if (!caster)
                return;

            if (auto player = caster->ToPlayer())
            {
                player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
                player->SetClientControl(player, 1);
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_aura_icespike_glacier_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_aura_icespike_glacier_AuraScript();
    }
};

void AddSC_IcecrownGlacier()
{
    RegisterSpellScript(spell_icespike_glacier);
    new spell_aura_icespike_glacier();
}
