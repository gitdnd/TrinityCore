#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"

class spell_gen_between_cast_periodic : public AuraScript
{
    PrepareAuraScript(spell_gen_between_cast_periodic);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->Effects[EFFECT_0].TriggerSpell });
    }

    void PeriodicTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        if (!GetCaster())
            return;
        std::list<Player*> targets;
        Trinity::AnyPlayerInObjectRangeCheck check(GetCaster(), 100.f, false);
        Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(GetCaster(), targets, check);
        Cell::VisitWorldObjects(GetCaster(), searcher, 100.f);
        for (std::list<Player*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
        {
            Player* player = (*iter);
            if (player->GetGUID() == GetCasterGUID() || player->isDead())
                continue;

            // Check of player is between the caster and the target, and check player Z is within range of the caster and target Z
            if (player->IsInBetween(GetCaster(), GetTarget(), 2.f) && abs(GetCaster()->GetPositionZ() - player->GetPositionZ()) <= 3)
                player->CastSpell(player, GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_between_cast_periodic::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

static uint32 comboAuras[6] = { 180054, 180055, 180056, 180057, 180058, 180059 };

class spell_generate_combopoint_all : public SpellScript
{
    PrepareSpellScript(spell_generate_combopoint_all);

    void HitHandler()
    {
        bool hasAura = false;
        for (uint32 checkAura : comboAuras)
        {
            hasAura = GetCaster()->HasAura(checkAura);
            if (hasAura)
                break;
        }

        if (!hasAura)
            return;

        uint32 points = 1;
        if (GetCaster()->HasAura(180173)
            && roll_chance_i(5))
        {
            points += 1;
        }

        if ((GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE) != 0 && GetCaster()->HasAura(180247)
            && roll_chance_i(5))
        {
            points += 1;
        }

        if ((GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) != 0 && GetCaster()->HasAura(180253)
            && roll_chance_i(5))
        {
            points += 1;
        }
        //Generic Combo Point Add spell
        //CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        //args.AddSpellBP0(points);
        //GetCaster()->CastSpell(GetHitUnit(), 450003, args);
        GetCaster()->AddComboPoints(GetHitUnit(), points);
        //GetHitUnit()->AddComboPoints(1);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_generate_combopoint_all::HitHandler);
    }
};

void AddSC_Spells_Custom_Generic()
{
    RegisterAuraScript(spell_gen_between_cast_periodic);
    RegisterSpellScript(spell_generate_combopoint_all);
}
