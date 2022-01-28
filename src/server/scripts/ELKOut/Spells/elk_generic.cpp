#include "action_include.h"


class spell_action_air_dodge : public SpellScript
{
    PrepareSpellScript(spell_action_air_dodge);

    void RollFinish() {
        auto caster = GetCaster();
        Spell* spell = GetSpell();
        spell->EffectRoll(EFFECT_0);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_action_air_dodge::RollFinish);
    }
};

class spell_action_hard_attack : public SpellScript
{
    PrepareSpellScript(spell_action_hard_attack);

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_action_hard_attack::ForceHeavy);
    }
};

class spell_action_quick_attack : public SpellScript
{
    PrepareSpellScript(spell_action_quick_attack);

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_action_quick_attack::ForceQuick);
    }
};

class spell_action_stab_attack : public SpellScript
{
    PrepareSpellScript(spell_action_stab_attack);
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_action_stab_attack::ForceThrust);
    }
};


class spell_action_basic_attack : public SpellScript
{
    PrepareSpellScript(spell_action_basic_attack);

    bool WasInAir = false;
    std::map<MapDummy, std::optional<std::any>> spellMap;
    int32 dummies = 0;
    void SpellFinish()
    {
        Unit* caster = GetCaster();
        if (!(caster))
            return;

        Spell* spell = GetSpell();
        spellMap = spell->GetTriggerDummy();


        switch (dummies)
        {
        case SPELL_ACTION_CRUSADER_STRIKE:
            caster->CastSpell(caster, SPELL_ACTION_PRESENCE_OF_SOUL, true);
            break;
        }

    }
    void SpellCast()
    {

        Unit* caster = GetCaster();
        Aura* slowSpell = caster->GetAura(SPELL_ACITON_ATTACK_SLOW);
        if (slowSpell)
        {
            slowSpell->Remove();
        }
    }
    void SpellHit()
    {
        Unit* caster = GetCaster();
        Unit* unitHit = GetHitUnit();
        if (!(caster && unitHit) || GetHitDamage() <= 0)
        {
            return;
        }

        Spell* spell = GetSpell();

        caster->ModifyPower(POWER_MANA, caster->GetStat(STAT_SPIRIT));
        if (WasInAir)
        {
            CustomSpellValues values;
            values.AddSpellMod(SPELLVALUE_AURA_DURATION, 2000);
            caster->CastCustomSpell(SPELL_ACTION_ATTACK_SLOW_DEBUFF, values, caster, TRIGGERED_FULL_MASK);
        }
        else
        {
            caster->CastSpell(unitHit, SPELL_ACTION_ATTACK_SLOW_DEBUFF, true);
        }
        switch (dummies)

        {
        case 0:
            break;
        case SPELL_ACTION_UNDEATH_STRIKE:
        {
            caster->CastSpell(unitHit, SPELL_ACTION_FESTERING_PLAGUE, true);
            Aura* festeringStacks = unitHit->GetAura(SPELL_ACTION_FESTERING_PLAGUE);
            if (festeringStacks)
            {
                int stacks = festeringStacks->GetStackAmount();
                if (stacks >= 6)
                {
                    caster->DealDamage(caster, unitHit, stacks * 300);
                    unitHit->RemoveAura(festeringStacks);
                    caster->CastSpell(unitHit, SPELL_ACTION_RAISE_GHOUL, true);
                }
            }
            break;
        }
        case SPELL_ACTION_WOLF_BITE:
        {
            unitHit->CastSpell(unitHit, SPELL_WOLF_FORM, true);
            break;
        }
        }
    }
    void SpellBegin()
    {
        spellMap = GetSpell()->GetTriggerDummy();
        auto& inAir = spellMap[MapDummy::WasInAir];
        if (inAir.has_value())
        {
            if (std::any_cast<bool>(inAir.value()) == true)
            {
                WasInAir = true;
            }
            else
            {
                WasInAir = false;
            }
        }
        else
            WasInAir = false;
        if (spellMap[MapDummy::TriggeringSpell].has_value())
        {
            dummies = std::any_cast<uint32>(spellMap[MapDummy::TriggeringSpell].value());
        }
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_action_basic_attack::SpellBegin);
        AfterCast += SpellCastFn(spell_action_basic_attack::SpellFinish);
        OnCast += SpellCastFn(spell_action_basic_attack::SpellCast);
        AfterHit += SpellHitFn(spell_action_basic_attack::SpellHit);
    }
};
class spell_action_deflect_aura : public AuraScript
{
    PrepareAuraScript(spell_action_deflect_aura);

    void SpellFinish()
    {
        Unit* unitCast = GetCaster();
        unitCast->SetSheath(SHEATH_STATE_MELEE);
    }
    void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* unitCast = GetTarget();
        Unit* unitAttacker = dmgInfo.GetAttacker();

        if (unitCast->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            unitCast->CastSpell(unitCast, SPELL_ACTION_AIR_DEFLECT_HIT, true);
        }
        unitCast->PlayDirectSound(18023);
        dmgInfo.ModifyDamage(-1 * dmgInfo.GetDamage());
        if (unitAttacker)
        {
            unitCast->CastSpell(unitAttacker, SPELL_ACTION_POSTURE_DAMAGE, true);
            Aura* postureStacaks = unitAttacker->GetAura(SPELL_ACTION_POSTURE_DAMAGE);
            if (postureStacaks)
            {
                int stacks = postureStacaks->GetStackAmount();
                if (stacks >= 10)
                {
                    unitAttacker->RemoveAura(postureStacaks);
                    unitCast->CastSpell(unitAttacker, SPELL_ACTION_POSTURE_STUN, true);
                }
            }
            unitCast->ModifyPower(POWER_RUNIC_POWER, (dmgInfo.GetDamage() * unitCast->GetStat(STAT_STAMINA)) / 1000);

            Position tempPoint = unitCast->GetPosition();
            Position tempPoint2 = unitAttacker->GetPosition();
            Position tempPoint3 = tempPoint;
            float    tempO = tempPoint2.GetAbsoluteAngle(tempPoint);
            tempPoint3.SetOrientation(0);
            tempPoint3.RelocatePolarOffset(tempO, 16.f);
            float tempX = tempPoint3.GetPositionX();
            float tempY = tempPoint3.GetPositionY();
            float tempZ = tempPoint3.GetPositionZ();

            // I have no clue how to move
            // try this; disable ground movement, make it fly only then do the jump and spam it for 1s every frame.

            Player* tempPlayer = dynamic_cast<Player*>(unitCast);

            Aura* tempAura = GetAura();
            unitCast->RemoveSpellCooldown(SPELL_ACTION_DEFLECT, true);
            unitCast->CastSpell(unitCast, SPELL_ACTION_DEFLECT, false);
        }
        unitCast->RemoveSpellCooldown(SPELL_ACTION_DEFLECT, true);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_action_deflect_aura::Absorb, EFFECT_1);
    }
};
class spell_action_deflect : public SpellScript
{
    PrepareSpellScript(spell_action_deflect);

    void SpellFinish()
    {

        Unit* caster = GetCaster();

        caster->SetSheath(SHEATH_STATE_MELEE);
        if (caster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            caster->CastSpell(caster, SPELL_ACTION_AIR_DEFLECT, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_action_deflect::SpellFinish);
    }
};
class spell_action_attack : public SpellScript
{
    PrepareSpellScript(spell_action_attack);

    void Register() override
    {
        AfterCast += SpellCastFn(spell_action_attack::ForceAttack);
    }

};


void SC_enemy_scripts();
void SC_dk_scripts();
void SC_druid_scripts();
void SC_item_scripts();
void SC_paladin_scripts();
void SC_priest_scripts();
void SC_warrior_scripts();

void AddSC_action_spell_scripts()
{
    RegisterSpellScript(spell_action_air_dodge);
    RegisterSpellScript(spell_action_hard_attack);
    RegisterSpellScript(spell_action_quick_attack);
    RegisterSpellScript(spell_action_stab_attack);
    RegisterSpellScript(spell_action_basic_attack);
    RegisterSpellAndAuraScriptPair(spell_action_deflect, spell_action_deflect_aura);
    RegisterSpellScript(spell_action_attack);


    SC_enemy_scripts();
    SC_dk_scripts();
    SC_druid_scripts();
    SC_item_scripts();
    SC_paladin_scripts();
    SC_priest_scripts();
    SC_warrior_scripts();
}
