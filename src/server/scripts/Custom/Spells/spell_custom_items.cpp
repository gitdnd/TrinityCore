#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

class spell_item_trinket_reset_cds : public SpellScript
{
    PrepareSpellScript(spell_item_trinket_reset_cds);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if(Player * plrTarget = GetHitPlayer())
            plrTarget->RemoveArenaSpellCooldowns();
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_trinket_reset_cds::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_Spells_Custom_Items()
{
    RegisterSpellScript(spell_item_trinket_reset_cds);
}
