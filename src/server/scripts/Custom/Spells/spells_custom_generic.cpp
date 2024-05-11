#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "CharacterCache.h"
#include "Guild.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"

class spell_gen_between_cast_periodic : public AuraScript
{
    PrepareAuraScript(spell_gen_between_cast_periodic);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->_effects[EFFECT_0].TriggerSpell });
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
            //@todo: this should cast from the circle summoners perspective 
            if (player->GetGUID() == GetCasterGUID() || player->isDead() || player->IsGameMaster())
                continue;

            // Check of player is between the caster and the target, and check player Z is within range of the caster and target Z
            if (player->IsInBetween(GetCaster(), GetTarget(), 2.f) && abs(GetCaster()->GetPositionZ() - player->GetPositionZ()) <= 3)
                player->CastSpell(player, GetSpellInfo()->_effects[aurEff->GetEffIndex()].TriggerSpell, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_between_cast_periodic::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_gen_between_cast_periodic_nozcheck : public AuraScript
{
    PrepareAuraScript(spell_gen_between_cast_periodic_nozcheck);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->_effects[EFFECT_0].TriggerSpell });
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
            //@todo: this should cast from the circle summoners perspective 
            if (player->GetGUID() == GetCasterGUID() || player->isDead() || player->IsGameMaster())
                continue;

            // Check of player is between the caster and the target
            if (player->IsInBetween(GetCaster(), GetTarget(), 5.f))
                player->CastSpell(player, GetSpellInfo()->_effects[aurEff->GetEffIndex()].TriggerSpell, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_gen_between_cast_periodic_nozcheck::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
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

class spell_gen_fly_in_hub : public AuraScript
{
    PrepareAuraScript(spell_gen_fly_in_hub);

    void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        //@todo:Make a function to grab flyable maps
        if (GetCaster()->GetMapId() != 765 || !GetCaster()->HasSpell(450010))
            PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_gen_fly_in_hub::HandleApplyEffect, EFFECT_2, SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

class spell_gen_subclass : public AuraScript
{
    PrepareAuraScript(spell_gen_subclass);

    void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        uint8 subClass = GetClassForSpell(aurEff->GetSpellInfo()->Id);
        Player* plrCaster = GetCaster()->ToPlayer();
        if (plrCaster && subClass > 0)
        {
            //plrCaster->SetClass(subClass); // Debug
            plrCaster->SetSubClass(subClass);
            sCharacterCache->UpdateCharacterSubClass(plrCaster->GetGUID(), subClass, plrCaster->GetSession());
            if (Guild* guild = plrCaster->GetGuild())
                guild->UpdateMemberData(plrCaster, GUILD_MEMBER_DATA_CLASS, subClass);
        }
    }

    void HandleRemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* plrCaster = GetCaster()->ToPlayer();
        if (plrCaster)
        {
            plrCaster->SetSubClass(0);
            sCharacterCache->UpdateCharacterSubClass(plrCaster->GetGUID(), plrCaster->GetClass(), plrCaster->GetSession());
            if (Guild* guild = plrCaster->GetGuild())
                guild->UpdateMemberData(plrCaster, GUILD_MEMBER_DATA_CLASS, plrCaster->GetClass());
        }
    }

    uint8 GetClassForSpell(uint32 id)
    {
        switch (id)
        {
        case 95012:
            return CLASS_SUB_WARDEN;
            break;
        case 95013:
            return CLASS_SUB_HISTORIAN;
            break;
        case 95014:
            return CLASS_SUB_WEAVER;
            break;
        case 95015:
            return CLASS_SUB_SAVAGE;
            break;
        case 95016:
            return CLASS_SUB_RANGER;
            break;
        case 95017:
            return CLASS_SUB_WATCHER;
            break;
        }
        return CLASS_WARLOCK; // Debug means something gone wrong.
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_gen_subclass::HandleApplyEffect, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectApplyFn(spell_gen_subclass::HandleRemoveEffect, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};


// 69470 - Heat Drain
// 69487 - Overheat
class spell_igb_periodic_trigger_with_power_cost : public AuraScript
{
    PrepareAuraScript(spell_igb_periodic_trigger_with_power_cost);

    void HandlePeriodicTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), aurEff->GetSpellEffectInfo().TriggerSpell, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_igb_periodic_trigger_with_power_cost::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 69399, 70172 - Cannon Blast
class spell_igb_cannon_blast : public SpellScript
{
    PrepareSpellScript(spell_igb_cannon_blast);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void CheckEnergy()
    {
        if (GetCaster()->GetPower(POWER_ENERGY) >= 100)
        {
            GetCaster()->CastSpell(GetCaster(), 69487, TRIGGERED_FULL_MASK); // SPELL_OVERHEAT
            if (Vehicle* vehicle = GetCaster()->GetVehicleKit())
                if (Unit* passenger = vehicle->GetPassenger(0))
                    sCreatureTextMgr->SendChat(GetCaster()->ToCreature(), 0, passenger); // SAY_OVERHEAT
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_igb_cannon_blast::CheckEnergy);
    }
};

// 69402, 70175 - Incinerating Blast
class spell_igb_incinerating_blast : public SpellScript
{
    PrepareSpellScript(spell_igb_incinerating_blast);

public:
    spell_igb_incinerating_blast()
    {
        _energyLeft = 0;
    }

private:
    void StoreEnergy()
    {
        _energyLeft = GetCaster()->GetPower(POWER_ENERGY) - 10;
    }

    void RemoveEnergy()
    {
        GetCaster()->SetPower(POWER_ENERGY, 0);
    }

    void CalculateDamage(SpellEffIndex /*effIndex*/)
    {
        SetEffectValue(GetEffectValue() + _energyLeft * _energyLeft * 8);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_igb_incinerating_blast::StoreEnergy);
        AfterCast += SpellCastFn(spell_igb_incinerating_blast::RemoveEnergy);
        OnEffectLaunchTarget += SpellEffectFn(spell_igb_incinerating_blast::CalculateDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }

    uint32 _energyLeft;
};

// 69487 - Overheat
class spell_igb_overheat : public AuraScript
{
    PrepareAuraScript(spell_igb_overheat);

    bool Load() override
    {
        if (GetAura()->GetType() != UNIT_AURA_TYPE)
            return false;
        return GetUnitOwner()->IsVehicle();
    }

    void SendClientControl(uint8 value)
    {
        if (Vehicle* vehicle = GetUnitOwner()->GetVehicleKit())
        {
            if (Unit* passenger = vehicle->GetPassenger(0))
            {
                if (Player* player = passenger->ToPlayer())
                {
                    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, GetUnitOwner()->GetPackGUID().size() + 1);
                    data << GetUnitOwner()->GetPackGUID();
                    data << uint8(value);
                    player->SendDirectMessage(&data);
                }
            }
        }
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SendClientControl(0);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SendClientControl(1);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_igb_overheat::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_igb_overheat::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_Spells_Custom_Generic()
{
    RegisterSpellScript(spell_gen_between_cast_periodic);
    RegisterSpellScript(spell_gen_between_cast_periodic_nozcheck);
    RegisterSpellScript(spell_generate_combopoint_all);
    RegisterSpellScript(spell_gen_fly_in_hub);
    RegisterSpellScript(spell_gen_subclass);
    RegisterSpellScript(spell_igb_cannon_blast);
    RegisterSpellScript(spell_igb_incinerating_blast);
    RegisterSpellScript(spell_igb_overheat);
}
