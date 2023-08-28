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
        GetCaster()->Say("hit", LANG_UNIVERSAL); // debug

        auto target = GetHitUnit();
        if (!target)
            return;

        GetCaster()->Say("acquired target", LANG_UNIVERSAL); // debug

        target->CastSpell(target, 90522, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_icespike_glacier::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_IcecrownGlacier()
{
    RegisterSpellScript(spell_icespike_glacier);
}
