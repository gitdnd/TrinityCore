/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /*
  * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
  * Ordered alphabetically using scriptname.
  * Scriptnames of files in this file should be prefixed with "spell_dk_".
  */

#include "ScriptMgr.h"
#include "Containers.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <numeric>
#include <iostream>

enum DeathKnightSpells
{
    SPELL_DK_ARMY_FLESH_BEAST_TRANSFORM = 127533,
    SPELL_DK_ARMY_GEIST_TRANSFORM = 127534,
    SPELL_DK_ARMY_NORTHREND_SKELETON_TRANSFORM = 127528,
    SPELL_DK_ARMY_SKELETON_TRANSFORM = 127527,
    SPELL_DK_ARMY_SPIKED_GHOUL_TRANSFORM = 127525,
    SPELL_DK_ARMY_SUPER_ZOMBIE_TRANSFORM = 127526,
    SPELL_DK_BLOOD = 137008,
    SPELL_DK_BLOOD_PLAGUE = 55078,
    SPELL_DK_BLOOD_SHIELD_ABSORB = 77535,
    SPELL_DK_BLOOD_SHIELD_MASTERY = 77513,
    SPELL_DK_CORPSE_EXPLOSION_TRIGGERED = 43999,
    SPELL_DK_DEATH_AND_DECAY_DAMAGE = 52212,
    SPELL_DK_DEATH_COIL_DAMAGE = 47632,
    SPELL_DK_DEATH_GRIP_DUMMY = 243912,
    SPELL_DK_DEATH_GRIP_JUMP = 49575,
    SPELL_DK_DEATH_GRIP_TAUNT = 51399,
    SPELL_DK_DEATH_STRIKE_HEAL = 45470,
    SPELL_DK_DEATH_STRIKE_OFFHAND = 66188,
    SPELL_DK_FESTERING_WOUND = 194310,
    SPELL_DK_FROST = 137006,
    SPELL_DK_GLYPH_OF_FOUL_MENAGERIE = 58642,
    SPELL_DK_GLYPH_OF_THE_GEIST = 58640,
    SPELL_DK_GLYPH_OF_THE_SKELETON = 146652,
    SPELL_DK_MARK_OF_BLOOD_HEAL = 206945,
    SPELL_DK_NECROSIS_EFFECT = 216974,
    SPELL_DK_RAISE_DEAD_SUMMON = 52150,
    SPELL_DK_RECENTLY_USED_DEATH_STRIKE = 180612,
    SPELL_DK_RUNIC_POWER_ENERGIZE = 49088,
    SPELL_DK_RUNIC_RETURN = 61258,
    SPELL_DK_SLUDGE_BELCHER = 207313,
    SPELL_DK_SLUDGE_BELCHER_SUMMON = 212027,
    SPELL_DK_DEATH_STRIKE_ENABLER = 89832, // Server Side
    SPELL_DK_TIGHTENING_GRASP = 206970,
    SPELL_DK_TIGHTENING_GRASP_SLOW = 143375,
    SPELL_DK_UNHOLY = 137007,
    SPELL_DK_UNHOLY_VIGOR = 196263,
    SPELL_DK_VOLATILE_SHIELDING = 207188,
    SPELL_DK_VOLATILE_SHIELDING_DAMAGE = 207194
};

enum Misc
{
    NPC_DK_DANCING_RUNE_WEAPON = 27893
};

// 70656 - Advantage (T10 4P Melee Bonus)
class spell_dk_advantage_t10_4p : public AuraScript
{
    PrepareAuraScript(spell_dk_advantage_t10_4p);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = eventInfo.GetActor())
        {
            Player* player = caster->ToPlayer();
            if (!player || caster->GetClass() != CLASS_DEATH_KNIGHT)
                return false;

            for (uint8 i = 0; i < player->GetMaxPower(POWER_RUNES); ++i)
                if (player->GetRuneCooldown(i) == 0)
                    return false;

            return true;
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_advantage_t10_4p::CheckProc);
    }
};

// 48707 - Anti-Magic Shell
class spell_dk_anti_magic_shell : public AuraScript
{
    PrepareAuraScript(spell_dk_anti_magic_shell);

public:
    spell_dk_anti_magic_shell()
    {
        absorbPct = 0;
        maxHealth = 0;
        absorbedAmount = 0;
    }

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_DK_RUNIC_POWER_ENERGIZE, SPELL_DK_VOLATILE_SHIELDING }) && spellInfo->GetEffects().size() > EFFECT_1;
    }

    bool Load() override
    {
        absorbPct = GetEffectInfo(EFFECT_1).CalcValue(GetCaster());
        maxHealth = GetCaster()->GetMaxHealth();
        absorbedAmount = 0;
        return true;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = CalculatePct(maxHealth, absorbPct);
    }

    void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
    {
        absorbedAmount += absorbAmount;

        if (!GetTarget()->HasAura(SPELL_DK_VOLATILE_SHIELDING))
        {
            CastSpellExtraArgs args(aurEff);
            args.AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(absorbAmount, 2 * absorbAmount * 100 / maxHealth));
            GetTarget()->CastSpell(GetTarget(), SPELL_DK_RUNIC_POWER_ENERGIZE, args);
        }
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (AuraEffect const* volatileShielding = GetTarget()->GetAuraEffect(SPELL_DK_VOLATILE_SHIELDING, EFFECT_1))
        {
            CastSpellExtraArgs args(volatileShielding);
            args.AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(absorbedAmount, volatileShielding->GetAmount()));
            GetTarget()->CastSpell(nullptr, SPELL_DK_VOLATILE_SHIELDING_DAMAGE, args);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell::Trigger, EFFECT_0);
        AfterEffectRemove += AuraEffectRemoveFn(spell_dk_anti_magic_shell::HandleEffectRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }

private:
    int32 absorbPct;
    int32 maxHealth;
    uint32 absorbedAmount;
};

static uint32 const ArmyTransforms[]
{
    SPELL_DK_ARMY_FLESH_BEAST_TRANSFORM,
    SPELL_DK_ARMY_GEIST_TRANSFORM,
    SPELL_DK_ARMY_NORTHREND_SKELETON_TRANSFORM,
    SPELL_DK_ARMY_SKELETON_TRANSFORM,
    SPELL_DK_ARMY_SPIKED_GHOUL_TRANSFORM,
    SPELL_DK_ARMY_SUPER_ZOMBIE_TRANSFORM
};
// 127517 - Army Transform
/// 6.x, does this belong here or in spell_generic? where do we cast this? sniffs say this is only cast when caster has glyph of foul menagerie.
class spell_dk_army_transform : public SpellScript
{
    PrepareSpellScript(spell_dk_army_transform);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_FOUL_MENAGERIE });
    }

    bool Load() override
    {
        return GetCaster()->IsGuardian();
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_FOUL_MENAGERIE))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), Trinity::Containers::SelectRandomContainerElement(ArmyTransforms), true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_army_transform::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_dk_army_transform::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
// 50842 - Blood Boil
class spell_dk_blood_boil : public SpellScript
{
    PrepareSpellScript(spell_dk_blood_boil);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_BLOOD_PLAGUE });
    }

    void HandleEffect()
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_BLOOD_PLAGUE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_blood_boil::HandleEffect);
    }
};

// 49028 - Dancing Rune Weapon
/// 7.1.5
class spell_dk_dancing_rune_weapon : public AuraScript
{
    PrepareAuraScript(spell_dk_dancing_rune_weapon);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sObjectMgr->GetCreatureTemplate(NPC_DK_DANCING_RUNE_WEAPON))
            return false;
        return true;
    }

    // This is a port of the old switch hack in Unit.cpp, it's not correct
    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = GetCaster();
        if (!caster)
            return;
        if (!eventInfo.GetActionTarget())
            return;
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo || spellInfo->Id == 49028)
            return;
        auto dummy = eventInfo.GetProcSpell()->triggerDummy;
        auto it = dummy.find(MapDummy::Scripted);
        {
            if (it != dummy.end())
            {
                auto& optional = (*it).second;
                if (optional.has_value())
                {
                    if (std::any_cast<bool>(optional.value()) == true)
                        return;
                }
            }
        }
        

        std::list<Unit*> drw;
        for (Unit* controlled : caster->m_Controlled)
        {
            if (controlled->GetEntry() == NPC_DK_DANCING_RUNE_WEAPON)
            {
                drw.push_back(controlled);
                break;
            }
        }

        if (drw.size() == 0)
            return;


        for (auto unit : drw)
        {
            caster->CastSpell(eventInfo.GetActionTarget(), spellInfo->Id, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddTriggerDummy(MapDummy::Scripted, true));
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_dancing_rune_weapon::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 43265 - Death and Decay
class spell_dk_death_and_decay : public SpellScript
{
    PrepareSpellScript(spell_dk_death_and_decay);
protected:
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_TIGHTENING_GRASP, SPELL_DK_TIGHTENING_GRASP_SLOW });
    }

    void HandleDummy()
    {
        if (GetCaster()->HasAura(SPELL_DK_TIGHTENING_GRASP))
            if (WorldLocation const* pos = GetExplTargetDest())
                GetCaster()->CastSpell(*pos, SPELL_DK_TIGHTENING_GRASP_SLOW, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_dk_death_and_decay::HandleDummy);
    }
};

// 43265 - Death and Decay (Aura)
class spell_dk_death_and_decay_AuraScript : public AuraScript
{
protected:
    PrepareAuraScript(spell_dk_death_and_decay_AuraScript);

    void HandleDummyTick(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();// guy who casts dummy spells that restores rune
        Unit* target = GetTarget();// the original caster of the ability.
        if (caster)
            caster->CastSpell(target, SPELL_DK_DEATH_AND_DECAY_DAMAGE, aurEff);
        if (caster->HasAura(277234) && urand(1, 10) <= 1)
        {
            target->AddAura(SPELL_DK_FESTERING_WOUND, caster);
        }
    }
    void Hit()
    {

    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_death_and_decay_AuraScript::HandleDummyTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};
class spell_dk_defile : public spell_dk_death_and_decay
{
    PrepareSpellScript(spell_dk_defile);
    void DefileGrow()
    {
        
    }
    void Register() override
    {
        spell_dk_death_and_decay::Register();
    }
};

class spell_dk_defile_AuraScript : public spell_dk_death_and_decay_AuraScript
{
    PrepareAuraScript(spell_dk_defile);
    bool hit = false;
    void ExpandHit(AuraEffect const* aurEff)
    {
        if (hit)
        {
            hit = false;
        }
    }
    void ExpandCheck()
    {
        hit = true;
    }
    void Register() override
    {
        spell_dk_death_and_decay_AuraScript::Register();
        //OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn()
        //OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_defile_AuraScript::ExpandD, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 47541 - Death Coil
class spell_dk_death_coil : public SpellScript
{
    PrepareSpellScript(spell_dk_death_coil);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_DEATH_COIL_DAMAGE, SPELL_DK_UNHOLY, SPELL_DK_UNHOLY_VIGOR });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->CastSpell(GetHitUnit(), SPELL_DK_DEATH_COIL_DAMAGE, true);
        if (AuraEffect const* unholyAura = caster->GetAuraEffect(SPELL_DK_UNHOLY, EFFECT_6)) // can be any effect, just here to send SPELL_FAILED_DONT_REPORT on failure
            caster->CastSpell(caster, SPELL_DK_UNHOLY_VIGOR, unholyAura);
        Aura* suddenDoom = caster->GetAura(81340);
        if (suddenDoom)
            suddenDoom->ModCharges(-1);
        Aura* damned = caster->GetAura(276837);
        if (damned)
        {
            SpellHistory* spellHis = caster->GetSpellHistory();
            spellHis->ModifyCooldown(275699, -Seconds(damned->GetEffect(EFFECT_0)->CalculateAmount(caster) / 10));
            spellHis->ModifyCooldown(42650, -Seconds(damned->GetEffect(EFFECT_1)->CalculateAmount(caster) / 10));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_coil::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 52751 - Death Gate
class spell_dk_death_gate : public SpellScript
{
    PrepareSpellScript(spell_dk_death_gate);

    SpellCastResult CheckClass()
    {
        if (GetCaster()->GetClass() != CLASS_DEATH_KNIGHT)
        {
            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        return SPELL_CAST_OK;
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        if (Unit* target = GetHitUnit())
            target->CastSpell(target, GetEffectValue(), false);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_death_gate::CheckClass);
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 49576 - Death Grip Initial
class spell_dk_death_grip_initial : public SpellScript
{
    PrepareSpellScript(spell_dk_death_grip_initial);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_DK_DEATH_GRIP_DUMMY,
                SPELL_DK_DEATH_GRIP_JUMP,
                SPELL_DK_BLOOD,
                SPELL_DK_DEATH_GRIP_TAUNT
            });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        // Death Grip should not be castable while jumping/falling
        if (caster->HasUnitState(UNIT_STATE_JUMPING) || caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
            return SPELL_FAILED_MOVING;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_DEATH_GRIP_DUMMY, true);
        GetHitUnit()->CastSpell(GetCaster(), SPELL_DK_DEATH_GRIP_JUMP, true);
        if (GetCaster()->HasAura(SPELL_DK_BLOOD))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_DEATH_GRIP_TAUNT, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_death_grip_initial::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_initial::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_dk_gorefiends_grasp_initial : public SpellScript
{
    PrepareSpellScript(spell_dk_gorefiends_grasp_initial);

    Unit* target = nullptr;
    void TargetSet(SpellEffIndex /*effIndex*/)
    {
        target = GetHitUnit();
    }
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* secTarget = GetHitUnit();
        if (!secTarget)
            return;
        secTarget->CastSpell(target, 146599, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_gorefiends_grasp_initial::TargetSet, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        OnEffectHitTarget += SpellEffectFn(spell_dk_gorefiends_grasp_initial::HandleDummy, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};


// 48743 - Death Pact
class spell_dk_death_pact : public AuraScript
{
    PrepareAuraScript(spell_dk_death_pact);

    void HandleCalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetUnitOwner();
        if (caster)
        {

            amount = int32((caster->GetMaxHealth() * 3) / 10);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_death_pact::HandleCalcAmount, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
    }
};

// 49998 - Death Strike
class spell_dk_death_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_death_strike);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo(
            {
                SPELL_DK_DEATH_STRIKE_ENABLER,
                SPELL_DK_DEATH_STRIKE_HEAL,
                SPELL_DK_BLOOD_SHIELD_MASTERY,
                SPELL_DK_BLOOD_SHIELD_ABSORB,
                SPELL_DK_FROST,
                SPELL_DK_DEATH_STRIKE_OFFHAND
            })
            && spellInfo->GetEffects().size() > EFFECT_2;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        if (AuraEffect* enabler = caster->GetAuraEffect(SPELL_DK_DEATH_STRIKE_ENABLER, EFFECT_0, GetCaster()->GetGUID()))
        {
            // Heals you for 25% of all damage taken in the last 5 sec,
            int32 heal = CalculatePct(enabler->CalculateAmount(GetCaster()), GetEffectInfo(EFFECT_1).CalcValue(GetCaster()));
            // minimum 7.0% of maximum health.
            int32 pctOfMaxHealth = CalculatePct(GetEffectInfo(EFFECT_2).CalcValue(GetCaster()), caster->GetMaxHealth());
            heal = std::max(heal, pctOfMaxHealth);

            Aura* hemostasis = caster->GetAura(273947);
            if (hemostasis)
            {
                heal = (heal  * (100 + hemostasis->GetEffect(EFFECT_0)->CalculateAmount(caster))) / 100;
            }
            caster->CastSpell(caster, SPELL_DK_DEATH_STRIKE_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, heal));

            AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_DK_BLOOD_SHIELD_MASTERY, EFFECT_0);
            if (aurEff)
            {
                Aura* absorb = caster->GetAura(SPELL_DK_BLOOD_SHIELD_ABSORB);
                if (absorb)
                {
                    absorb->GetEffect(EFFECT_0)->ModAmount(CalculatePct(heal, aurEff->GetAmount()));
                    absorb->RefreshDuration();
                }
                else
                    caster->CastSpell(caster, SPELL_DK_BLOOD_SHIELD_ABSORB, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(heal, aurEff->GetAmount())));

            }
            if (caster->HasAura(101568))
                caster->RemoveAura(101568);
            if (caster->HasAura(SPELL_DK_FROST))
                caster->CastSpell(GetHitUnit(), SPELL_DK_DEATH_STRIKE_OFFHAND, true);
        }
    }
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Aura* hemostasis = caster->GetAura(273947);
        if (hemostasis)
        {
            SetEffectValue((GetEffectValue() * (100 + hemostasis->GetStackAmount() * hemostasis->GetEffect(EFFECT_0)->CalculateAmount(caster))) / 100);
            caster->RemoveAura(273947);
        }
    }
    void TriggerRecentlyUsedDeathStrike()
    {
        Unit* caster = GetCaster();
        caster->CastSpell(GetCaster(), SPELL_DK_RECENTLY_USED_DEATH_STRIKE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_strike::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectLaunch += SpellEffectFn(spell_dk_death_strike::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_dk_death_strike::TriggerRecentlyUsedDeathStrike);
    }
};

// 89832 - Death Strike Enabler - SPELL_DK_DEATH_STRIKE_ENABLER
class spell_dk_death_strike_enabler : public AuraScript
{
    PrepareAuraScript(spell_dk_death_strike_enabler);

    // Amount of seconds we calculate damage over
    constexpr static uint8 LAST_SECONDS = 5;

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void Update(AuraEffect* /*aurEff*/)
    {
        // Move backwards all datas by one from [23][0][0][0][0] -> [0][23][0][0][0]
        std::move_backward(_damagePerSecond.begin(), std::next(_damagePerSecond.begin(), LAST_SECONDS - 1), _damagePerSecond.end());
        _damagePerSecond[0] = 0;
    }

    void HandleCalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = true;
        amount = int32(std::accumulate(_damagePerSecond.begin(), _damagePerSecond.end(), 0u));
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        _damagePerSecond[0] += eventInfo.GetDamageInfo()->GetDamage();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_death_strike_enabler::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dk_death_strike_enabler::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_death_strike_enabler::HandleCalcAmount, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_dk_death_strike_enabler::Update, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    std::array<uint32, LAST_SECONDS> _damagePerSecond = { };
};

// 85948 - Festering Strike
class spell_dk_festering_strike : public SpellScript
{
protected:
    PrepareSpellScript(spell_dk_festering_strike);
protected:
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_FESTERING_WOUND });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
        int stacks = GetEffectValue() + urand(0, 1);
        if (!aura)
        {
            caster->AddAura(SPELL_DK_FESTERING_WOUND, target);
            aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
            stacks--;
        }
        aura->ModStackAmount(stacks);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_festering_strike::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};
class spell_dk_clawing_shadows : public spell_dk_festering_strike
{
    PrepareSpellScript(spell_dk_clawing_shadows);


    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_clawing_shadows::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
// Festering Wound Damage - probably 295875 ? 
void BurstFesteringWound(Aura*& aura, Unit*& target, Unit*& caster)
{
    aura->ModStackAmount(-1);
    caster->CastSpell(target, 194311, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    if (caster->HasAura(207264))
    {
        caster->CastSpell(target, 207267, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
    if (caster->HasAura(194917))
    {
        caster->AddAura(51460, caster);
    }
}
class spell_dk_scourge_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_scourge_strike);

    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
        if (aura)
            BurstFesteringWound(aura, target, caster);
    }
    void OnPrecast() override
    {
        if (GetCaster()->HasAura(43265))
            return;
        GetSpell()->SetSpellValue(SPELLVALUE_MAX_TARGETS, 1);
    }
    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_scourge_strike::HandleScriptEffect);
    }
};
void RefreshRunes(Player*& player, uint8 amount)
{
    std::vector<std::pair<uint32, uint8>> runeCD;
    int runes = player->GetMaxPower(POWER_RUNES);
    if (amount > runes)
        amount = runes;
    runeCD.reserve(runes);
    runeCD.resize(runes);
    for (uint8 i = 0; i < runes; i++)
        runeCD[i] = std::pair(player->GetRuneCooldown(i), i);

    std::sort(runeCD.begin(), runeCD.end());
    for (uint8 i = runes - 1; i >= (runes - amount); i--)
    {
        player->SetRuneCooldown(runeCD[i].second, 1);
        player->ResyncRunes();
    }
    player->UpdateAllRunesRegen();
}
class spell_dk_apocalypse : public SpellScript
{
    PrepareSpellScript(spell_dk_apocalypse);

    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
        if (aura)
        {
            int stacks = std::min(aura->GetStackAmount(), (uint8)GetEffectInfo(EFFECT_1).CalcValue(GetCaster()));
            for (int i = 0; i < stacks; i++)
            {
                BurstFesteringWound(aura, target, caster);
                caster->CastSpell(caster, 42651, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            }
            if (caster->HasAura(276837))
            {
                caster->CastSpell(caster, 288544, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            }
        }
        if (caster->HasAura(343755))
        {
            Player* player = caster->ToPlayer();

            RefreshRunes(player, 2);
        }
    }
    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_apocalypse::HandleScriptEffect);
    }
};


class spell_dk_unholy_assault : public SpellScript
{
    PrepareSpellScript(spell_dk_unholy_assault);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_FESTERING_WOUND });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
        int stacks = GetEffectInfo(EFFECT_2).CalcValue(GetCaster());
        if (!aura)
        {
            caster->AddAura(SPELL_DK_FESTERING_WOUND, target);
            aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID());
            stacks--;
        }
        aura->ModStackAmount(stacks);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_unholy_assault::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_dk_army_of_the_dead : public SpellScript
{
    PrepareSpellScript(spell_dk_army_of_the_dead);

    void AotDCast()
    {
        Unit* caster = GetCaster();
        if (caster->HasAura(276837))
        {
            caster->CastSpell(caster, 288544, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        }
    }
    void Register() override
    {
        OnCast += SpellCastFn(spell_dk_army_of_the_dead::AotDCast);
    }
};
class spell_dk_heart_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_heart_strike);


    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Aura* heartbreaker = caster->GetAura(221536);
        if (heartbreaker)
        {
            caster->ModifyPower(POWER_RUNIC_POWER, 20);
        }
    }
    void OnPrecast() override
    {
        if (GetCaster()->HasAura(43265))
            GetSpell()->SetSpellValue(SPELLVALUE_MAX_TARGETS, 5);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_heart_strike::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
    // if heartbreaker, effecttargethit adds 2 runic power to caster.
};
class spell_dk_tombstone_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_tombstone_AuraScript);
    // on cast remove up to X bone shield charges, set shield (6% max hp * charges ) and add RP * 6 * 10 * charges used (max 5)

    void Apply(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        Unit* owner = GetUnitOwner();
        Aura* aura = owner->GetAura(195181);
        if (!aura)
            return;
        int8 stacks = std::min((uint8)5, aura->GetStackAmount());
        aura->ModStackAmount(-stacks);
        amount = ((stacks * owner->GetMaxHealth() * 6) / 100);
        owner->ModifyPower(POWER_RUNIC_POWER, stacks * 60);
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_tombstone_AuraScript::Apply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);

    }
};
class spell_dk_blood_tap : public SpellScript
{
    PrepareSpellScript(spell_dk_blood_tap);
    void Rune()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;
        RefreshRunes(player, 1);
    }
    void Register() override
    {
        OnCast += SpellCastFn(spell_dk_blood_tap::Rune);
    }
};

class spell_dk_vampiric_blood_PowerScript : public PowerScript
{

    PrepareAuraScript(spell_dk_vampiric_blood_PowerScript);

    void SetAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        amount = GetUnitOwner()->CountPctFromMaxHealth(GetEffectInfo(EFFECT_3).CalcValue(GetCaster()));
    }
    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        if (amount >= 0 || power != POWER_RUNIC_POWER)
            return;
        Aura* redThis = user->GetAura(205723);
        if (!redThis)
            return;
        amount *= -1;
        Aura* aura = GetAura();
        user->GetSpellHistory()->ModifyCooldown(55233, -Seconds(redThis->GetEffect(EFFECT_0)->CalculateAmount(user) * amount * redThis->GetEffect(EFFECT_1)->CalculateAmount(user)) / 10);
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_vampiric_blood_PowerScript::SetAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH);
        AddPowerChange(spell_dk_vampiric_blood_PowerScript::PowerChanged);
    }
};
class spell_dk_marrowrend : public SpellScript
{
    PrepareSpellScript(spell_dk_marrowrend);



    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster->GetOwner();
        if (owner)
            caster = owner;
        Aura* aura = caster->GetAura(195181);
        int stacks = GetEffectInfo(EFFECT_2).CalcValue(GetCaster());
        if (!aura)
        {
            caster->CastSpell(caster, 195181, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            aura = caster->GetAura(195181);
            stacks--;
        }
        aura->ModStackAmount(stacks);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_marrowrend::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_DUMMY);
    }
};

class spell_dk_bone_shield_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_bone_shield_AuraScript);


    void BoneHit(ProcEventInfo& eventInfo)
    {
        Unit* owner = GetUnitOwner();
        if (eventInfo.GetActor() == owner)
            return;
        Aura* aura = owner->GetAura(195181);
        if (aura->GetStackAmount() > 1)
        {
            aura->ModStackAmount(-1);
            PreventDefaultAction();
        }
        if(owner->HasAura(221699))
            owner->GetSpellHistory()->ModifyCooldown(221699, -Seconds(2000));
        if (!aura)
            return;
        if (aura->GetStackAmount() >= 5 && owner->HasAura(219786))
        {
            owner->AddAura(219788, owner);
        }
        else
        {
            owner->RemoveAura(219788);
        }
    }
    void Removed(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Unit* owner = GetUnitOwner();
        owner->RemoveAura(219788);
    }
    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dk_bone_shield_AuraScript::Removed, EFFECT_0, SPELL_AURA_MOD_ARMOR_PCT_FROM_STAT, AURA_EFFECT_HANDLE_REAL);
        DoPrepareProc += AuraProcFn(spell_dk_bone_shield_AuraScript::BoneHit);
    }
};
class spell_dk_bone_shield : public SpellScript
{
    PrepareSpellScript(spell_dk_bone_shield);

    void CountBones()
    {
        Unit* caster = GetCaster();
        Aura* aura = caster->GetAura(195181);
        if (!aura)
        {
            caster->RemoveAura(219788);
            return;
        }
        if(aura->GetStackAmount() >= 5)
        {
            caster->AddAura(219788, caster);
        }
        else
        {
            caster->RemoveAura(219788);
        }
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_dk_bone_shield::CountBones);
    }
};
class spell_dk_rime_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_rime_AuraScript);
    bool RimePropperRNG(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetProcSpell()->m_spellInfo->Id == 49020)
        {
            if (urand(1, 100) <= 45)
            {
                return true;
            }
        }
        return false;
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_rime_AuraScript::RimePropperRNG);
    }
};

class spell_dk_remorseless_winter_PowerScript : public PowerScript
{

    PrepareAuraScript(spell_dk_remorseless_winter_PowerScript);

    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        if (amount >= 0 || power != POWER_RUNES)
            return;
        Aura* gatheringStorm = user->GetAura(194912);
        if (!gatheringStorm)
            return;
        Aura* aura = GetAura();
        aura->ModDuration(500);
        Unit* caster = GetCaster();
        caster->AddAura(211805, caster);
    }
    void RemoveRemorseless(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Unit* caster = GetCaster();
        Aura* aura = caster->GetAura(211805);
        aura->Remove();
    }
    void Register() override
    {
        AddPowerChange(spell_dk_remorseless_winter_PowerScript::PowerChanged);
        OnEffectRemove += AuraEffectRemoveFn(spell_dk_remorseless_winter_PowerScript::RemoveRemorseless, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }

};
/*
class spell_dk_icy_talons_PowerScript : public PowerScript
{

    PrepareAuraScript(spell_dk_icy_talons_PowerScript);

    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        if (amount >= 0 || power != POWER_RUNIC_POWER)
            return;
        Unit* caster = GetCaster();
        caster->AddAura(194879, caster);
    }
    void Register() override
    {
        AddPowerChange(spell_dk_icy_talons_PowerScript::PowerChanged);
    }

};
*/ 

class spell_dk_icy_talons_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_icy_talons_AuraScript);

    bool CheckIfSpender(ProcEventInfo& eventInfo)
    {
        if (!eventInfo.GetProcSpell())
            return false;
        for (auto power : eventInfo.GetProcSpell()->m_spellInfo->PowerCosts)
        {
            if (power == NULL)
                continue;
            if (power->PowerType == POWER_RUNIC_POWER && power->ManaCost > 0)
                return true;
        }
        return false;
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_icy_talons_AuraScript::CheckIfSpender);
    }
};
class spell_dk_frostscythe : public SpellScript
{
    PrepareSpellScript(spell_dk_frostscythe);
    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Aura* killingMachine = caster->GetAura(51124);
        if (killingMachine)
        {
            killingMachine->Remove();
            Aura* murderous = caster->GetAura(207061);
            if (murderous)
            {
                uint32 chance = murderous->GetEffect(EFFECT_0)->CalculateAmount(caster);
                if (urand(1, 100) <= chance)
                {
                    Player* player = caster->ToPlayer();
                    if (!player)
                        return;
                    RefreshRunes(player, 1);
                }
            }
        }
        
        Aura* icecap = caster->GetAura(207126);
        if (icecap)
        {
            caster->GetSpellHistory()->ModifyCooldown(51271, -Seconds(icecap->GetEffect(EFFECT_0)->CalculateAmount(caster) / 10));
        }
    }
    void Inexorable(SpellEffIndex)
    {
        Unit* caster = GetCaster();
        Aura* inexorable = caster->GetAura(253595);
        if (inexorable)
        {
            caster->CastSpell(GetHitUnit(), 253597, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        }
    }
    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_frostscythe::HandleScriptEffect);
        OnEffectHitTarget += SpellEffectFn(spell_dk_frostscythe::Inexorable, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
// cold heart 281209 , damage 281210
class spell_dk_chains_of_ice : public SpellScript
{
    PrepareSpellScript(spell_dk_chains_of_ice);
    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Aura* coldHeart = caster->GetAura(281209);
        if (coldHeart)
        {
            caster->CastSpell(GetHitUnit(), 281210, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddTriggerDummy(MapDummy::TriggerStacks, (uint8)coldHeart->GetStackAmount()));

            coldHeart->Remove();
        }
    }
    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_chains_of_ice::HandleScriptEffect);
    }
};
class spell_dk_cold_heart : public SpellScript
{
    PrepareSpellScript(spell_dk_cold_heart);

    void Hit(SpellEffIndex effIndex)
    {
        auto& triggerDummy = GetSpell()->GetTriggerDummy();
        auto it = triggerDummy.find(MapDummy::TriggerStacks);
        if (it == triggerDummy.end())
            return;
        auto& optional = (*it).second;
        if(!optional.has_value())
            return;
        SetHitDamage(GetHitDamage() * std::any_cast<uint8>(optional.value()));
    }
    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dk_cold_heart::Hit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
class spell_dk_frost_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_frost_strike);
    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Aura* icecap = caster->GetAura(207126);
        if (icecap)
        {
            caster->GetSpellHistory()->ModifyCooldown(51271, -Seconds(icecap->GetEffect(EFFECT_0)->CalculateAmount(caster) / 10));
        }
        if (caster->HasAura(281238) && caster->HasAura(51271))
        {
            caster->AddAura(51124, caster);
            if (urand(1, 10) <= 3)
            {
                Player* player = caster->ToPlayer();
                if (player)
                    RefreshRunes(player, 1);
            }
        }
    }
    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_frost_strike::HandleScriptEffect);
    }
};
class spell_dk_obliterate : public SpellScript
{
    PrepareSpellScript(spell_dk_obliterate);
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Aura* killingMachine = caster->GetAura(51124);
        if (killingMachine)
        {
            killingMachine->Remove();
            Aura* murderous = caster->GetAura(207061);
            if (murderous)
            {
                uint32 chance = murderous->GetEffect(EFFECT_0)->CalculateAmount(caster);
                if (urand(1, 100) <= chance)
                {
                    Player* player = caster->ToPlayer();
                    if (!player)
                        return;
                    RefreshRunes(player, 1);
                }
            }
        }
        Aura* icecap = caster->GetAura(207126);
        if (icecap)
        {
            caster->GetSpellHistory()->ModifyCooldown(51271, -Seconds(icecap->GetEffect(EFFECT_0)->CalculateAmount(caster) / 10));
        }
        Aura* inexorable = caster->GetAura(253595);
        if (inexorable)
        {
            caster->CastSpell(GetHitUnit(), 253597, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        }
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_obliterate::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
class spell_dk_breath_of_sindragosa_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_breath_of_sindragosa_AuraScript);
    void HandleScriptEffect(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        if (player)
        {
            RefreshRunes(player, 2);
        }
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dk_breath_of_sindragosa_AuraScript::HandleScriptEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_dk_breath_of_sindragosa_AuraScript::HandleScriptEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_dk_empower_rune_weapon_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_empower_rune_weapon_AuraScript);
    void HandleScriptEffect(AuraEffect const* aurEff)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        if (player)
        {
            RefreshRunes(player, 1);
        }
    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_empower_rune_weapon_AuraScript::HandleScriptEffect, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
    }
};
class spell_dk_inexorable_assault_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_inexorable_assault_AuraScript);
    void HandleScriptEffect(AuraEffect const* aurEff)
    {
        Unit* caster = GetUnitOwner();
        if (caster)
        {
            caster->AddAura(253595, caster);
        }
    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_inexorable_assault_AuraScript::HandleScriptEffect, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};
class spell_dk_frozen_pulse_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_frozen_pulse_AuraScript);

    bool CheckRunes(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (caster->GetPower(POWER_RUNES) >= 3)
        {
            return false;
        }
        return true;
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_frozen_pulse_AuraScript::CheckRunes);
    }
};
class spell_dk_inexorable_assault_dummy_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_inexorable_assault_dummy_AuraScript);
    bool procced = false;
    bool HandleProc(ProcEventInfo& eventInfo)
    {
        if(eventInfo.GetProcSpell()->m_spellInfo->Id != 49020 && eventInfo.GetProcSpell()->m_spellInfo->Id != 207230)
        {
            return false;
        }
        return true;
    }

    void PreventDefault(ProcEventInfo& eventInfo)
    {
        Aura* aura = GetAura();
        if (aura->GetStackAmount() > 1)
        {
            if (procced)
            {
                aura->ModStackAmount(-1);
                procced = false;
            }
            else
                procced = true;
            PreventDefaultAction();
            return;
        }
    }
    void Register() override
    {
        DoPrepareProc += AuraProcFn(spell_dk_inexorable_assault_dummy_AuraScript::PreventDefault);
        DoCheckProc += AuraCheckProcFn(spell_dk_inexorable_assault_dummy_AuraScript::HandleProc);
    }
};
class spell_dk_blinding_sleet_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_blinding_sleet_AuraScript);
    void HandleScriptEffect(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Unit* target = GetTarget();
        target->AddAura(317898, GetUnitOwner());
    }
    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_dk_blinding_sleet_AuraScript::HandleScriptEffect, EFFECT_0, SPELL_AURA_MOD_CONFUSE, AURA_EFFECT_HANDLE_REAL);
    }
};
class spell_dk_horn_of_winter : public SpellScript
{

    PrepareSpellScript(spell_dk_horn_of_winter);

    void Runes()
    {
        Player* player = GetCaster()->ToPlayer();
        if (player)
        {
            RefreshRunes(player, 2);
        }
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_dk_horn_of_winter::Runes);
    }
};
class spell_dk_howling_blast : public SpellScript
{
    PrepareSpellScript(spell_dk_howling_blast);
    void Hit(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        if(caster->HasAura(207142) && caster->HasAura(59052))
            caster->CastSpell(GetHitUnit(), 207150, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Aura* aura = caster->GetAura(59052);
        if (aura)
            aura->Remove();
        if (caster->HasAura(281238) && caster->HasAura(51271))
        {
            caster->AddAura(51124, caster);
            if (urand(1, 10) <= 3)
            {
                Player* player = caster->ToPlayer();
                if (player)
                    RefreshRunes(player, 1);
            }
        }
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_howling_blast::Hit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_dk_howling_blast::HandleScriptEffect);
    }
};

class spell_dk_will_of_the_necropolis_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_will_of_the_necropolis_AuraScript);

    void InfiniteAbs(AuraEffect const*, int32& amount, bool&)
    {
        amount = -1;
    }
    void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* caster = GetCaster();
        if (caster->GetHealthPct() < GetEffectInfo(EFFECT_1).CalcValue(caster))
        {
            absorbAmount = (dmgInfo.GetDamage() * GetEffectInfo(EFFECT_2).CalcValue(caster)) / 100;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_will_of_the_necropolis_AuraScript::InfiniteAbs, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_will_of_the_necropolis_AuraScript::Absorb, EFFECT_0);
    }

};
//191685 Virulent Eruption
class spell_dk_virulent_plague_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_virulent_plague_AuraScript);

    void End(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Unit* target = GetTarget();
        if (!target->isDead())
            return;
        Unit* caster = GetUnitOwner();
        caster->CastSpell(target, 191685, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_dk_virulent_plague_AuraScript::End, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};
class spell_dk_permafrost_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_permafrost_AuraScript);

    void Shield(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetUnitOwner();
        Aura* aura = caster->GetAura(207203);
        if (!aura)
        {
            caster->AddAura(207203, caster);
            aura = caster->GetAura(207203);
            if (!aura)
                return;
        }
        AuraEffect* eff = aura->GetEffect(EFFECT_0);
        eff->SetAmount(eff->GetAmount() + aura->GetEffect(EFFECT_0)->CalculateAmount(caster));
        aura->RefreshDuration();
    }

    void Register() override
    {
        DoPrepareProc += AuraProcFn(spell_dk_permafrost_AuraScript::Shield);
    }

};
/*
class spell_dk_virulent_plague : public SpellScript
{
    PrepareSpellScript(spell_dk_virulent_plague);
    void OnCast()
    {
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_dk_virulent_plague::OnCast);
    }
};
*/
// epidemic damage
class spell_dk_epidemic : public SpellScript
{
    PrepareSpellScript(spell_dk_epidemic);
    void Hit(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if(target->HasAura(191587))
            GetCaster()->CastSpell(target, 212739, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
    void OnCast()
    {
        Unit* caster = GetCaster();
        if (caster->HasAura(81340))
            caster->RemoveAura(81340);
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_dk_epidemic::OnCast);
        OnEffectHitTarget += SpellEffectFn(spell_dk_epidemic::Hit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_dk_unholy_blight_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_unholy_blight_AuraScript);
    void ApplyVirulentPlague(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        GetUnitOwner()->AddAura(191587, GetTarget());
    }
    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_dk_unholy_blight_AuraScript::ApplyVirulentPlague, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};
//currently gargoyle is supposed to fly away into the air after 30s when 50515 triggers but it just walks, need to set it to flying.
class spell_dk_gargoyle_PowerScript : public PowerScript
{
    PrepareAuraScript(spell_dk_gargoyle_PowerScript);
    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        amount /= 10;
        std::list<TempSummon*> minions;
        user->GetAllMinionsByEntry(minions, 27829);
        user->GetAllMinionsByEntry(minions, 100876);
        for (Unit* unit : minions)
        {
        }
    }
    void Register() override
    {
        AddPowerChange(spell_dk_gargoyle_PowerScript::PowerChanged);
    }
};
class spell_dk_runic_corruption_PowerScript : public PowerScript
{
    PrepareAuraScript(spell_dk_runic_corruption_PowerScript);

    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        if (amount >= 0 || power != POWER_RUNIC_POWER)
            return;
        float runicPower = -1 * amount;
        Aura* runicAura = user->GetAura(51462);
        if (!runicAura)
            return;
        if ((runicPower * (float)runicAura->GetEffect(EFFECT_0)->CalculateAmount(user)) / 1000.f > frand(1.f, 100.f))
        {
            user->AddAura(51460, user);
        }
    }
    void Register() override
    {
        AddPowerChange(spell_dk_runic_corruption_PowerScript::PowerChanged);
    }
};

class spell_dk_runic_empowerment_PowerScript : public PowerScript
{
    PrepareAuraScript(spell_dk_runic_empowerment_PowerScript);
    void PowerChanged(Unit* user, Powers power, int32 amount)
    {
        if (amount >= 0 || power != POWER_RUNIC_POWER)
            return;
        int32 runicPower = -1 * amount;
        Aura* runicAura = user->GetAura(81229);
        if (!runicAura)
            return;
        if ((runicPower * runicAura->GetEffect(EFFECT_0)->GetAmount()) / 100.f > frand(1.f, 100.f))
        {
            Player* player = user->ToPlayer();
            RefreshRunes(player, 1);
        }
    }
    void Register() override
    {
        AddPowerChange(spell_dk_runic_empowerment_PowerScript::PowerChanged);
    }
};
class spell_dk_pet_claw : public SpellScript
{
    PrepareSpellScript(spell_dk_pet_claw);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_FESTERING_WOUND });
    }

    void HandleScriptEffect()
    {
        Unit* caster = GetCaster();
        Unit* owner = caster->GetOwner();
        if (owner && owner->HasAura(207272))
        {
            Unit* target = GetHitUnit();
            if (urand(1, 10) <= 3)
                owner->AddAura(SPELL_DK_FESTERING_WOUND, target);
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_pet_claw::HandleScriptEffect);
    }
};
/*

class spell_dk_consumption : public SpellScript
{
    PrepareSpellScript(spell_dk_consumption);

    void Heal(SpellEffIndex effIndex)
    {
        if (GetCaster() == GetHitUnit())
            SetHitHeal(GetHitHeal() * 1.5);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_consumption::Heal, EFFECT_0, SPELL_EFFECT_HEALTH_LEECH);
    }

};
*/
class spell_dk_bonestorm : public SpellScript
{

    PrepareSpellScript(spell_dk_bonestorm);

    void SetDuration()
    {
        Unit* caster = GetCaster();
        Spell* spell = GetSpell();
        auto power = spell->GetPowerTypeCostAmount(POWER_RUNIC_POWER);
        if (!power.has_value())
            return;
        int32 duration = power.value() * 10;
        Aura* aura = caster->GetAura(194844);
        if (!aura)
            return;
        aura->GetEffect(EFFECT_2)->SetAmount(1);
        aura->SetDuration(duration);
        
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_dk_bonestorm::SetDuration);
    }
};
class spell_dk_bonestorm_dummy : public SpellScript
{

    PrepareSpellScript(spell_dk_bonestorm_dummy);
    int8 hit = 0;
    void Hit(SpellEffIndex effIndex)
    {
        if (hit < 5)
        {
            Unit* caster = GetCaster();
            caster->DealHeal(HealInfo(caster, caster, int32((caster->GetMaxHealth() / 100) * 3), GetSpellInfo(), SPELL_SCHOOL_MASK_NONE));
            hit++;
        }
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_bonestorm_dummy::Hit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};


class spell_dk_hemostasis_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_dk_hemostasis_AuraScript);
    bool HemoStasisPropper(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetProcSpell()->m_spellInfo->Id != 50842)
        {
            return false;
        }
        return true;
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_hemostasis_AuraScript::HemoStasisPropper);
    }
};

class spell_dk_sacrificial_pact : public SpellScript
{
    PrepareSpellScript(spell_dk_sacrificial_pact);
    void Sacrifice()
    {
        Unit* caster = GetCaster();

        std::list<TempSummon*> minions;
        caster->GetAllMinionsByEntry(minions, 26125);
        for (Unit* unit : minions)
        {
            if (unit->isDead())
                continue;
            caster->CastSpell(unit->GetWorldLocation(), 327611, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            caster->CastSpell(unit, 327965, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            caster->HealBySpell(HealInfo(caster, caster, (caster->GetMaxHealth() * 25) / 100, GetSpellInfo(), SPELL_SCHOOL_MASK_NORMAL));
            return;
        }
    }
    SpellCastResult CheckGhoul()
    {
        Unit* caster = GetCaster();
        std::list<TempSummon*> minions;
        caster->GetAllMinionsByEntry(minions, 26125);
        
        for (Unit* unit : minions)
        {
            if (unit->isDead())
                continue;
            return SPELL_CAST_OK;
        }
        return SPELL_FAILED_NO_PET;
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_dk_sacrificial_pact::Sacrifice);
        OnCheckCast += SpellCheckCastFn(spell_dk_sacrificial_pact::CheckGhoul);
    }
};

class spell_dk_frostwyrms_fury : public SpellScript
{

};

class spell_dk_glacial_advance : public SpellScript
{

};

class spell_dk_soul_reaper : public SpellScript
{

};

class spell_dk_dark_transformation : public SpellScript
{

};

class spell_dk_purgatory : public SpellScript
{

};


class spell_dk_bloodworms : public SpellScript
{

};


// hypothermic presence why doesn't it work stock?








// 47496 - Explode, Ghoul spell for Corpse Explosion
class spell_dk_ghoul_explode : public SpellScript
{
    PrepareSpellScript(spell_dk_ghoul_explode);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_DK_CORPSE_EXPLOSION_TRIGGERED }) && spellInfo->GetEffects().size() > EFFECT_2;
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        SetHitDamage(GetCaster()->CountPctFromMaxHealth(GetEffectInfo(EFFECT_2).CalcValue(GetCaster())));
    }

    void Suicide(SpellEffIndex /*effIndex*/)
    {
        if (Unit* unitTarget = GetHitUnit())
        {
            // Corpse Explosion (Suicide)
            unitTarget->CastSpell(unitTarget, SPELL_DK_CORPSE_EXPLOSION_TRIGGERED, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode::Suicide, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
/*
// 69961 - Glyph of Scourge Strike
class spell_dk_glyph_of_scourge_strike_script : public SpellScript
{
    PrepareSpellScript(spell_dk_glyph_of_scourge_strike_script);

    void HandleScriptEffect(SpellEffIndex )
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        Unit::AuraEffectList const& mPeriodic = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
        for (Unit::AuraEffectList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
        {
            AuraEffect const* aurEff = *i;
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            // search our Blood Plague and Frost Fever on target
            if (spellInfo->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT && spellInfo->SpellFamilyFlags[2] & 0x2 &&
                aurEff->GetCasterGUID() == caster->GetGUID())
            {
                uint32 countMin = aurEff->GetBase()->GetMaxDuration();
                uint32 countMax = spellInfo->GetMaxDuration();

                // this Glyph
                countMax += 9000;

                if (countMin < countMax)
                {
                    aurEff->GetBase()->SetDuration(aurEff->GetBase()->GetDuration() + 3000);
                    aurEff->GetBase()->SetMaxDuration(countMin + 3000);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_glyph_of_scourge_strike_script::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};
*/
// 206940 - Mark of Blood
class spell_dk_mark_of_blood : public AuraScript
{
    PrepareAuraScript(spell_dk_mark_of_blood);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_MARK_OF_BLOOD_HEAL });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* target = eventInfo.GetProcTarget();
        if (Unit* caster = GetCaster())
            caster->CastSpell(target, SPELL_DK_MARK_OF_BLOOD_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, (target->GetMaxHealth() * 3) / 100));
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_mark_of_blood::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};
// 207346 - Necrosis
class spell_dk_necrosis : public AuraScript
{
    PrepareAuraScript(spell_dk_necrosis);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_NECROSIS_EFFECT });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_DK_NECROSIS_EFFECT, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_necrosis::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 121916 - Glyph of the Geist (Unholy)
/// 6.x, does this belong here or in spell_generic? apply this in creature_template_addon? sniffs say this is always cast on raise dead.
class spell_dk_pet_geist_transform : public SpellScript
{
    PrepareSpellScript(spell_dk_pet_geist_transform);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_THE_GEIST });
    }

    bool Load() override
    {
        return GetCaster()->IsPet();
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_THE_GEIST))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_pet_geist_transform::CheckCast);
    }
};

// 147157 Glyph of the Skeleton (Unholy)
/// 6.x, does this belong here or in spell_generic? apply this in creature_template_addon? sniffs say this is always cast on raise dead.
class spell_dk_pet_skeleton_transform : public SpellScript
{
    PrepareSpellScript(spell_dk_pet_skeleton_transform);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_THE_SKELETON });
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_THE_SKELETON))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_pet_skeleton_transform::CheckCast);
    }
};

// 61257 - Runic Power Back on Snare/Root
/// 7.1.5
class spell_dk_pvp_4p_bonus : public AuraScript
{
    PrepareAuraScript(spell_dk_pvp_4p_bonus);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_RUNIC_RETURN });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        return (spellInfo->GetAllEffectsMechanicMask() & ((1 << MECHANIC_ROOT) | (1 << MECHANIC_SNARE))) != 0;
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActionTarget()->CastSpell(nullptr, SPELL_DK_RUNIC_RETURN, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_pvp_4p_bonus::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dk_pvp_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 46584 - Raise Dead
class spell_dk_raise_dead : public SpellScript
{
    PrepareSpellScript(spell_dk_raise_dead);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_RAISE_DEAD_SUMMON, SPELL_DK_SLUDGE_BELCHER, SPELL_DK_SLUDGE_BELCHER_SUMMON });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        uint32 spellId = SPELL_DK_RAISE_DEAD_SUMMON;
        if (caster->HasAura(SPELL_DK_SLUDGE_BELCHER))
            spellId = SPELL_DK_SLUDGE_BELCHER_SUMMON;
        if (caster->HasAura(194916))
            caster->CastSpell(caster, 196910, CastSpellExtraArgs(TRIGGERED_FULL_MASK));

        GetCaster()->CastSpell(nullptr, spellId, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_raise_dead::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_deathknight_spell_scripts()
{
    RegisterAuraScript(spell_dk_advantage_t10_4p);
    RegisterAuraScript(spell_dk_anti_magic_shell);
    RegisterSpellScript(spell_dk_army_transform);
    RegisterSpellScript(spell_dk_blood_boil);
    RegisterAuraScript(spell_dk_dancing_rune_weapon);
    RegisterSpellAndAuraScriptPair(spell_dk_death_and_decay, spell_dk_death_and_decay_AuraScript);
    RegisterSpellScript(spell_dk_death_coil);
    RegisterSpellScript(spell_dk_death_gate);
    RegisterSpellScript(spell_dk_death_grip_initial);
    RegisterAuraScript(spell_dk_death_pact);
    RegisterSpellScript(spell_dk_death_strike);
    RegisterAuraScript(spell_dk_death_strike_enabler);
    RegisterSpellScript(spell_dk_festering_strike);
    RegisterSpellScript(spell_dk_clawing_shadows);
    RegisterSpellScript(spell_dk_scourge_strike);
    RegisterSpellScript(spell_dk_apocalypse);
    RegisterSpellScript(spell_dk_unholy_assault);
    RegisterSpellScript(spell_dk_army_of_the_dead);
    RegisterSpellScript(spell_dk_marrowrend);
    RegisterAuraScript(spell_dk_vampiric_blood_PowerScript);
    RegisterSpellAndAuraScriptPair(spell_dk_bone_shield, spell_dk_bone_shield_AuraScript);
    RegisterSpellScript(spell_dk_obliterate);
    RegisterSpellScript(spell_dk_howling_blast);
    RegisterAuraScript(spell_dk_will_of_the_necropolis_AuraScript);
    RegisterSpellScript(spell_dk_pet_claw);
    RegisterAuraScript(spell_dk_unholy_blight_AuraScript);
    RegisterSpellAndAuraScriptPair(spell_dk_defile, spell_dk_defile_AuraScript);
    RegisterAuraScript(spell_dk_gargoyle_PowerScript);
    RegisterAuraScript(spell_dk_runic_corruption_PowerScript);
    RegisterAuraScript(spell_dk_runic_empowerment_PowerScript);
    RegisterAuraScript(spell_dk_rime_AuraScript);
    RegisterSpellScript(spell_dk_heart_strike);
    RegisterAuraScript(spell_dk_remorseless_winter_PowerScript);
    RegisterAuraScript(spell_dk_icy_talons_AuraScript);
    RegisterAuraScript(spell_dk_frozen_pulse_AuraScript);
    RegisterSpellScript(spell_dk_frostscythe);
    RegisterSpellScript(spell_dk_frost_strike);
    RegisterAuraScript(spell_dk_permafrost_AuraScript);
    RegisterAuraScript(spell_dk_virulent_plague_AuraScript);
    RegisterSpellScript(spell_dk_bonestorm);
    RegisterSpellScript(spell_dk_bonestorm_dummy);
    RegisterAuraScript(spell_dk_hemostasis_AuraScript);
    RegisterAuraScript(spell_dk_tombstone_AuraScript);
    RegisterSpellScript(spell_dk_blood_tap);
    RegisterSpellScript(spell_dk_epidemic); 
    RegisterSpellScript(spell_dk_horn_of_winter);
    RegisterAuraScript(spell_dk_breath_of_sindragosa_AuraScript);
    RegisterAuraScript(spell_dk_empower_rune_weapon_AuraScript);
    RegisterAuraScript(spell_dk_inexorable_assault_AuraScript);
    RegisterSpellScript(spell_dk_chains_of_ice);
    RegisterAuraScript(spell_dk_blinding_sleet_AuraScript); 
    RegisterSpellScript(spell_dk_gorefiends_grasp_initial); 
    RegisterAuraScript(spell_dk_inexorable_assault_dummy_AuraScript); 
        RegisterSpellScript(spell_dk_cold_heart);
    

    RegisterSpellScript(spell_dk_ghoul_explode);
    RegisterAuraScript(spell_dk_mark_of_blood);
    RegisterAuraScript(spell_dk_necrosis);
    RegisterSpellScript(spell_dk_pet_geist_transform);
    RegisterSpellScript(spell_dk_pet_skeleton_transform);
    RegisterAuraScript(spell_dk_pvp_4p_bonus);
    RegisterSpellScript(spell_dk_raise_dead);

}
