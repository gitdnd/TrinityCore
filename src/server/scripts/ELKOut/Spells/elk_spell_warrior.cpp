#include "elk_spell_script_include.h"



class elk_warrior_star_jump_two : public SpellScript
{
    PrepareSpellScript(elk_warrior_star_jump_two);

    void SpellWhile()
    {
        Unit* caster = GetCaster();
        if (caster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            GetSpell()->ReSetTimer();
        }
        else
        {
            GetSpell()->cancel();
            caster->AddSpellCooldown(elk_warrior_STAR_JUMP, 0, 10000, true);
        }
    }
    void SpellCast()
    {
        Unit* caster = GetCaster();
        caster->AddSpellCooldown(elk_warrior_STAR_JUMP, 0, 10000, true);
        Spell* spell = GetSpell();
        auto target = spell->GetDestTargets(0);
        if (!target)
            return;

        caster->CastSpell(target->_position.GetPositionX(), target->_position.GetPositionY(), target->_position.GetPositionZ(), elk_warrior_LEAP, true);
        caster->CastSpell(caster, elk_warrior_THUNDER_CLAP, false);
    }

    void Register() override
    {
        WhileCast += SpellCastFn(elk_warrior_star_jump_two::SpellWhile);
        AfterCast += SpellCastFn(elk_warrior_star_jump_two::SpellCast);
    }
};


class elk_warrior_star_jump_one : public SpellScript
{
    PrepareSpellScript(elk_warrior_star_jump_one);


    void Register() override
    {
    }
};

class elk_warrior_star_jump : public SpellScript
{
    PrepareSpellScript(elk_warrior_star_jump);

    void SpellCast()
    {
        Unit* caster = GetCaster();
        if (caster->FindCurrentSpellBySpellId(elk_warrior_STAR_JUMP_TWO))
        {
            caster->FindCurrentSpellBySpellId(elk_warrior_STAR_JUMP_TWO)->SetSpellTimer(0);
        }
        else if (caster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            caster->CastSpell(caster, elk_warrior_STAR_JUMP_ONE, false);
            caster->CastSpell(caster, elk_warrior_STAR_JUMP_TWO, false);
        }
    }

    void Register() override
    {
        BeforeCastTime += SpellCastFn(elk_warrior_star_jump::SpellCast);
    }
};

class elk_warrior_thunder_clap : public SpellScript
{
    PrepareSpellScript(elk_warrior_thunder_clap);

    bool wasInAir = false;
    int  airTicks = 250;
    WorldLocation castingPoint;
    void StartCast()
    {
        auto  spell = GetSpell();
        Unit* caster = GetCaster();
        if (caster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
        {
            caster->CastSpell(caster, elk_warrior_SLAM_DUNK, true);
            wasInAir = true;
        }
        else
            castingPoint = caster->GetWorldLocation();


        spell->SetSpellTimer(250);
    }
    void SpellCast()
    {
        Unit* caster = GetCaster();
        auto spell = GetSpell();
        if (wasInAir)
        {
            if (caster->GetUnitMovementFlags() & MOVEMENTFLAG_FALLING)
            {
                airTicks -= spell->GetCastTime() - spell->GetSpellTimer();
                spell->ReSetTimer();

            }
            else
            {
                castingPoint = caster->GetWorldLocation();
                wasInAir = false;
                spell->SetSpellTimer(std::max(airTicks, 0));
                spell->SetSpellValue(SPELLVALUE_BASE_POINT0, (int)(float(spell->GetSpellValue()->EffectBasePoints[0]) * (1.f + (250.f - float(airTicks)) / 250.f)));
            }
        }
        else if (caster->GetWorldLocation() != castingPoint)
        {
            spell->cancel();
        }
    }

    void Register() override
    {
        BeforeCastTime += SpellCastFn(elk_warrior_thunder_clap::StartCast);
        WhileCast += SpellCastFn(elk_warrior_thunder_clap::SpellCast);
    }
};

void SC_warrior_scripts()
{
    RegisterSpellScript(elk_warrior_star_jump_two);
    RegisterSpellScript(elk_warrior_star_jump_one);
    RegisterSpellScript(elk_warrior_star_jump);
    RegisterSpellScript(elk_warrior_thunder_clap);
}
