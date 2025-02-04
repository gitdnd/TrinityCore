#include "ScriptMgr.h"
#include "Containers.h"
#include "GameTime.h"
#include "Group.h"
#include "Player.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "ItemTemplate.h"
#include "Creature.h"
#include "WorldSession.h"


enum CustomClassSpells
{
    SPELL_CLASS_SEAL_OF_RIGHTEOUSNESS = 97002,
    SPELL_CLASS_SEAL_OF_COMMAND_MH = 97012,
    SPELL_CLASS_SEAL_OF_COMMAND_OH = 97013,
    SPELL_CLASS_SEAL_OF_COMMAND_RANGED = 97014,
    SPELL_CLASS_SEAL_OF_ROCKBITER = 97022,
    SPELL_CLASS_ROCKBITER_REFLECT = 97023,
    SPELL_CLASS_SEAL_OF_SPELLBLADE = 97032,
    SPELL_CLASS_SEAL_OF_DARKTIDE = 97042,
    SPELL_CLASS_SEAL_OF_FLAMETONGUE = 97052,
    SPELL_CLASS_SEAL_OF_LIGHT = 97062,
    SPELL_CLASS_SEAL_OF_LIGHT_HEAL = 97063,
    SPELL_CLASS_DEADLY = 97072,
    SPELL_CLASS_SEAL_OF_VENOMSTRIKE_DAMAGE = 97073,
    SPELL_CLASS_SEAL_OF_WINDFURY = 97091,
    SPELL_CLASS_SEAL_OF_WINDFURY_MH = 97092,
    SPELL_CLASS_SEAL_OF_WINDFURY_OH = 97093,
    SPELL_CLASS_SEAL_OF_WINDFURY_RANGED = 97094,
    SPELL_CLASS_SEAL_OF_BLOODGRIP_DUMMY = 97102,
    SPELL_CLASS_SEAL_OF_BLOODGRIP_BLEED = 97103,
    SPELL_CLASS_SEAL_OF_BLOODGRIP_RANGED = 97104,
    SPELL_TALENT_IGNITE = 97307,
    SPELL_TALENT_OVERLOAD = 94206,
    SPELL_CLASS_HOLY_SLASH = 97316,
    SPELL_CLASS_FIRE_SLASH = 97317,
    SPELL_CLASS_LIGHTNING_SLASH = 97318,
    SPELL_CLASS_FROST_SLASH = 97319,
    SPELL_CLASS_SHADOW_SLASH = 97320,
    SPELL_CLASS_ARCANE_SLASH = 97321,
    SPELL_TALENT_SECRETS_OF_MANA = 94252,
    SPELL_TALENT_SECRETS_OF_MANA_BUFF = 94253,
    SPELL_TALENT_BLOOD_DRIVE_BUFF = 94259,
    SPELL_TALENT_ENERGY_SHIELD_BUFF = 94276,
    SPELL_TALENT_CHAMPION = 94278,
    SPELL_TOTEM_TOTEM_WITHDRAWAL = 94280,
    SPELL_TALENT_RUNE_WEAPON = 94263,
    SPELL_TALENT_RUNE_WEAPON_HIDDEN_PASSIVE = 94282,
    SPELL_TALENT_RUNE_DEBUFF = 94281,
    SPELL_TALENT_RUNE_WEAPON_DRAIN = 94283,
    SPELL_TALENT_DRUID_OF_THE_MYCELIUM_PROC = 94286,
    SPELL_TALENT_DRUID_OF_THE_MYCELIUM_DUMMY = 94287,
    SPELL_TALENT_BASILISK_BITE_PASSIVE = 94288,
    SPELL_TALENT_BASILISK_BITE = 97331,
    SPELL_TALENT_CRUICIBLE_OF_FAITH = 94290,
    SPELL_CLASS_EMPOWERED_ATTACK = 96560,
    SPELL_CLASS_EMPOWERED_ATTACK_STACKS = 96561,
    SPELL_TALENT_BOOMING_VOICE = 93194,
    SPELL_ITEM_LOTUS_RESTORE   = 91048,
    SPELL_CLASS_FINGERS_OF_FROST     = 97332,
    SPELL_CLASS_FINGERS_OF_FROST_STACKS = 97333,
    SPELL_FREEZE               = 97334,
    SPELL_TALENT_UNINSPIRED = 94293,
    SPELL_TALENT_BRAINSTORM = 94294,
    SPELL_ITEM_MIRAGE_TALON_INT = 91231,
    SPELL_ITEM_MIRAGE_TALON_AGI = 91233,
    SPELL_ITEM_CINDERBREAKER_STR = 91235,
    SPELL_ITEM_GOLEMS_EMBRACE = 91237,
    SPELL_TALENT_PROFICIENCY = 94609,
    SPELL_TALENT_DEADLY_CALM = 94610

};

// 97200 - scourge strike
class spell_class_scourge_strike : public SpellScriptLoader
{
public:
    spell_class_scourge_strike() : SpellScriptLoader("spell_class_scourge_strike") { }


    class spell_class_scourge_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_scourge_strike_SpellScript);


        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
            {
                if (target->HasAuraState(AURA_STATE_FROZEN))
                {
                    SetHitDamage(GetHitDamage() * 4);
                }
            }

            if (Unit* caster = GetCaster())
            {

                if (Unit* target = GetHitUnit())
                {

                    int32 bonusDamage = 0;

                    uint32 shadowDotCount = 0;

                    Unit::AuraApplicationMap const& auras = target->GetAppliedAuras();

                    for (auto const& auraPair : auras)
                    {

                        Aura* aura = auraPair.second->GetBase();

                        if (!aura || aura->GetCasterGUID() != caster->GetGUID())
                            continue;

                        SpellInfo const* spellInfo = aura->GetSpellInfo();

                        if (!spellInfo)
                            continue;

                        if ((spellInfo->SchoolMask & SPELL_SCHOOL_MASK_SHADOW) == 0)
                            continue;

                        if (spellInfo->HasAura(SPELL_AURA_PERIODIC_DAMAGE)) shadowDotCount++;

                    }
                    if (shadowDotCount > 0)
                    {
                        SetHitDamage(GetHitDamage() * (1 + shadowDotCount * 0.10f));

                    }
                }
            }
        }


        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_class_scourge_strike_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };


    SpellScript* GetSpellScript() const override
    {
        return new spell_class_scourge_strike_SpellScript();
    }

};

// 57993 - Envenom
class spell_class_envenom : public SpellScriptLoader
{
public:
    spell_class_envenom() : SpellScriptLoader("spell_class_envenom") { }
    class spell_class_envenom_spellscript : public SpellScript
    {

        PrepareSpellScript(spell_class_envenom_spellscript);

        void HandleEffect(SpellEffIndex)
        {
            if (Unit* caster = GetCaster())
            {


                float const spellPowerPerCombo[6] =
                {
                    0.0f,
                    0.4f,    
                    0.8f,   
                    1.2f,    
                    1.6f,    
                    2.f      
                };

                float const attackPowerPerCombo[6] =
                {
                    0.0f,
                    0.12f,
                    0.24f,
                    0.36f,
                    0.48f,
                    0.6f
                };

                uint8 cp = caster->ToPlayer()->GetComboPoints();
                if (cp > 5)
                    cp = 5;
                int32 amount = ((caster->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE) * spellPowerPerCombo[cp]) + (caster->GetTotalAttackPowerValue(BASE_ATTACK) * attackPowerPerCombo[cp]));
                SetEffectValue(amount);
            }
        }
        void Register() override
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_class_envenom_spellscript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_envenom_spellscript();
    }
};

//  48668 - Eviscerate
class spell_class_eviscerate : public SpellScriptLoader
{
public:
    spell_class_eviscerate() : SpellScriptLoader("spell_class_eviscerate") { }
    class spell_class_eviscerate_spellscript : public SpellScript
    {

        PrepareSpellScript(spell_class_eviscerate_spellscript);

        void HandleEffect(SpellEffIndex)
        {
            if (Unit* caster = GetCaster())
            {


                float const attackPowerPerCombo[6] =
                {
                    0.0f,
                    0.2f,   
                    0.4f,  
                    0.6f,   
                    0.8f,   
                    1.f     
                };

                uint8 cp = caster->ToPlayer()->GetComboPoints();
                if (cp > 5)
                    cp = 5;

                int32 amount = (caster->GetTotalAttackPowerValue(BASE_ATTACK) * attackPowerPerCombo[cp]);
                SetEffectValue(amount);
            }
        }
        void Register() override
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_class_eviscerate_spellscript::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_eviscerate_spellscript();
    }
};

namespace SharedData
{
    static std::map<uint64, std::deque<uint32>> playerSpellHistory;
}

// 94292 - Prodigy
class spell_talent_prodigy : public AuraScript
{
    PrepareAuraScript(spell_talent_prodigy);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster || !caster->IsPlayer())
            return;

        SpellInfo const* triggeredSpell = eventInfo.GetDamageInfo()->GetSpellInfo();
        if (!triggeredSpell)
            return;

        uint32 currentSpellId = triggeredSpell->Id;
        uint64 playerGUID = caster->GetGUID();


        auto& history = SharedData::playerSpellHistory[playerGUID];
        int stacksToAdd = 1;


        int stacks = 0;
        auto itr = std::find(history.begin(), history.end(), currentSpellId);

        if (itr != history.end())
        {
            int distance = std::distance(itr, history.end()) - 1;

            if (distance == 0)
                stacks = 4;
            else if (distance == 1)
                stacks = 3;
            else if (distance == 2)
                stacks = 2;
            else if (distance == 3)
                stacks = 1;
        }

        if (stacks == 0)
        {
            caster->CastSpell(caster, SPELL_TALENT_BRAINSTORM, true);
        }
        else
        {
            for (int i = 0; i < stacks; i++)
            {
                caster->CastSpell(caster, SPELL_TALENT_UNINSPIRED, true);
            }
            GetTarget()->RemoveAurasDueToSpell(SPELL_TALENT_BRAINSTORM);
        }

        if (itr != history.end())
            history.erase(itr);

        history.push_back(currentSpellId);
        if (history.size() > 4)
            history.pop_front();

    }
    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_prodigy::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

};


// 94294 - Brainstorm
class spell_talent_brainstorm : public AuraScript
{
    PrepareAuraScript(spell_talent_brainstorm);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_EMPOWERED_ATTACK });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        Unit* target = eventInfo.GetProcTarget();
        if (!caster || !caster->IsPlayer())
            return;
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        Aura* newAura = caster->GetAura(SPELL_TALENT_BRAINSTORM);
        uint32 stacks = newAura->GetStackAmount();
        uint32 spellId = eventInfo.GetSpellInfo()->Id;

        if (stacks >= 5)
        {
            if (caster->GetSpellHistory()->HasCooldown(spellId))
            {
                caster->GetSpellHistory()->ResetCooldown(spellId, true);
                GetTarget()->RemoveAurasDueToSpell(GetId());

                uint64 playerGUID = caster->GetGUID();
                SharedData::playerSpellHistory.erase(playerGUID);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_brainstorm::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// deep freeze
class spell_class_deep_freeze : public SpellScriptLoader
{
public:
    spell_class_deep_freeze() : SpellScriptLoader("spell_class_deep_freeze") { }

    class spell_class_deep_freeze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_deep_freeze_SpellScript);

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();


            if (target->HasAuraState(AURA_STATE_FROZEN) || caster->GetAura(SPELL_CLASS_FINGERS_OF_FROST))
            {
                SetHitDamage(GetHitDamage() * 2);
                caster->CastSpell(target, 97712, true);
            }

            SpellInfo const* ccSpellInfo = sSpellMgr->GetSpellInfo(97712);

            if (target->IsImmunedToSpell(ccSpellInfo, caster) || caster->GetAura(SPELL_CLASS_FINGERS_OF_FROST))
            {
                SetHitDamage(GetHitDamage() * 2);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_class_deep_freeze_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_deep_freeze_SpellScript();
    }
};

// 94201 - Absolute Zero
class spell_talent_absolute_zero : public AuraScript
{
    PrepareAuraScript(spell_talent_absolute_zero);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();

        if (!caster || !target)
            return;

        SpellInfo const* ccSpellInfo = sSpellMgr->GetSpellInfo(SPELL_FREEZE);

        if (target->IsImmunedToSpell(ccSpellInfo, caster))
        {
            caster->CastSpell(caster, SPELL_CLASS_FINGERS_OF_FROST, true);
            caster->CastSpell(caster, SPELL_CLASS_FINGERS_OF_FROST, true);
        }
        else
        {
            caster->CastSpell(target, SPELL_FREEZE, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_absolute_zero::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 97333 - Fingers of Frost Stacks
class spell_class_fingers_of_frost_stacks : public AuraScript
{
    PrepareAuraScript(spell_class_fingers_of_frost_stacks);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_FINGERS_OF_FROST });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->RemoveAuraFromStack(GetId());
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_CLASS_FINGERS_OF_FROST);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_class_fingers_of_frost_stacks::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_class_fingers_of_frost_stacks::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 97332 Fingers of Frost Hidden Proc
class spell_class_fingers_of_frost : public SpellScriptLoader
{
public:
    spell_class_fingers_of_frost() : SpellScriptLoader("spell_class_fingers_of_frost") { }

    class spell_class_fingers_of_frost_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_fingers_of_frost_SpellScript);

        void HandleOnCast()
        {
            Unit* caster = GetCaster();

            CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
            args.AddSpellMod(SPELLVALUE_AURA_STACK, 1);
            caster->CastSpell(caster, SPELL_CLASS_FINGERS_OF_FROST_STACKS, args);
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_class_fingers_of_frost_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_fingers_of_frost_SpellScript();
    }
};

// aura 220- SPELL_AURA_MOD_RATING_FROM_STAT does not work properly for haste rating
class spell_item_stat_to_haste : public AuraScript
{
    PrepareAuraScript(spell_item_stat_to_haste);

    void OnTick(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();

        uint32 statType = GetAura()->GetEffect(EFFECT_1)->GetMiscValue();
        int32 stat = 0;

        switch (statType)
        {
        case 0: stat = player->GetStat(STAT_STRENGTH); break;
        case 1: stat = player->GetStat(STAT_AGILITY); break;
        case 2: stat = player->GetStat(STAT_STAMINA); break;
        case 3: stat = player->GetStat(STAT_INTELLECT); break;
        case 4: stat = player->GetStat(STAT_SPIRIT); break;
        default:
            return;
        }

        int32 statPerc = GetAura()->GetEffect(EFFECT_1)->GetAmount();

        if (Aura* aura = player->GetAura(GetSpellInfo()->Id))
        {
            if (AuraEffect* effect = aura->GetEffect(EFFECT_0))
            {
                effect->ChangeAmount(stat * statPerc / 100);
            }
        }

    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_stat_to_haste::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }

};

// 91047 - Lotus Restore
class spell_item_lotus_restore : public AuraScript
{
    PrepareAuraScript(spell_item_lotus_restore);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_ITEM_LOTUS_RESTORE
            });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();


        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        Unit* target = eventInfo.GetActor();

        int32 percRestored = GetAura()->GetEffect(EFFECT_0)->GetAmount();

        CastSpellExtraArgs args(aurEff);
        CastSpellExtraArgs args1(aurEff);
        CastSpellExtraArgs args2(aurEff);
        int32 cost = procSpell->CalcPowerCost(GetTarget(), eventInfo.GetSchoolMask());

        args.AddSpellMod(SPELLVALUE_BASE_POINT0, cost * percRestored / 100);
        args1.AddSpellMod(SPELLVALUE_BASE_POINT1, cost * percRestored / 100);
        args2.AddSpellMod(SPELLVALUE_BASE_POINT2, cost * percRestored / 100);

        if (procSpell->PowerType == POWER_HEALTH)
        {
            target->CastSpell(target, SPELL_ITEM_LOTUS_RESTORE, args);
        }

        else if (procSpell->PowerType == POWER_MANA)
        {
            target->CastSpell(target, SPELL_ITEM_LOTUS_RESTORE, args1);
        }

        else if (procSpell->PowerType == POWER_FOCUS)
        {
            target->CastSpell(target, SPELL_ITEM_LOTUS_RESTORE, args2);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_lotus_restore::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};


// Empowered Attacks Stacks - 96561
class spell_class_empowered_attack_stacks : public AuraScript
{
    PrepareAuraScript(spell_class_empowered_attack_stacks);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_EMPOWERED_ATTACK });
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->RemoveAuraFromStack(GetId());
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_CLASS_EMPOWERED_ATTACK);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_class_empowered_attack_stacks::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_class_empowered_attack_stacks::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Empowered Attacks - 96560
class spell_class_empowered_attack : public SpellScriptLoader
{
public:
    spell_class_empowered_attack() : SpellScriptLoader("spell_class_empowered_attack") { }

    class spell_class_empowered_attack_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_empowered_attack_SpellScript);

        void HandleOnCast()
        {

            Unit* caster = GetCaster();


            if (caster->GetAura(SPELL_TALENT_BOOMING_VOICE))
            {
                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.AddSpellMod(SPELLVALUE_AURA_STACK, 2);
                caster->CastSpell(caster, SPELL_CLASS_EMPOWERED_ATTACK_STACKS, args);
            }
            else
            {
                CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
                args.AddSpellMod(SPELLVALUE_AURA_STACK, 1);
                caster->CastSpell(caster, SPELL_CLASS_EMPOWERED_ATTACK_STACKS, args);
            }
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_class_empowered_attack_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_empowered_attack_SpellScript();
    }
};

// 94291 - Blessed Life
class spell_talent_blessed_life : public AuraScript
{
    PrepareAuraScript(spell_talent_blessed_life);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();
        Unit* caster = GetCaster();
        Player* player = caster->ToPlayer();
        int32 spirit = player->GetStat(STAT_SPIRIT);

        if (!roll_chance_f(lround((float)spirit / 20)))
            return;

        GetTarget()->CastSpell(victim, 31934, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_blessed_life::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 94289 - Crucible of Faith
class spell_talent_cruicible_of_faith : public AuraScript
{
    PrepareAuraScript(spell_talent_cruicible_of_faith);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();
        int32 spirit = player->GetStat(STAT_SPIRIT);
        int32 healCritChance = player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 1);


        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_CRUICIBLE_OF_FAITH))
        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(spirit * 0.01);
            existingBuff->GetEffect(EFFECT_1)->ChangeAmount(healCritChance * -1);
        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_cruicible_of_faith::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 94288 - Basilisk Bite
class spell_talent_basilisk_bite : public AuraScript
{
    PrepareAuraScript(spell_talent_basilisk_bite);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TALENT_BASILISK_BITE,SPELL_TALENT_BASILISK_BITE_PASSIVE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetProcTarget();

        if (!caster || !target)
            return;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK || eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            if (!roll_chance_f(50))
                return;
        }
        else
        {
            if (!roll_chance_f(25))
                return;
        }

        int32 totalDamage = 0;
        Unit::AuraApplicationMap const& auras = target->GetAppliedAuras();
        for (auto const& auraPair : auras)
        {
            Aura* aura = auraPair.second->GetBase();
            if (!aura || aura->GetCasterGUID() != caster->GetGUID())
                continue;

            SpellInfo const* spellInfo = aura->GetSpellInfo();
            if (!spellInfo)
                continue;

            flag96 familyFlag = aura->GetSpellInfo()->SpellFamilyFlags;
            if (familyFlag[0] & 0x00080000 || familyFlag[0] & 0x01000000)
            {
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (AuraEffect const* effect = aura->GetEffect(i))
                    {
                        totalDamage += effect->GetAmount() * effect->GetRemainingTicks();
                    }
                }
            }
        }
        Aura* basiliskPAssive = caster->GetAura(SPELL_TALENT_BASILISK_BITE_PASSIVE);
        int32 percDamage = basiliskPAssive->GetEffect(EFFECT_0)->GetAmount();
        ApplyPct(totalDamage, percDamage);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(totalDamage);
        GetTarget()->CastSpell(target, SPELL_TALENT_BASILISK_BITE, args);
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_basilisk_bite::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_basilisk_bite::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 94285 - Druid of the Mycelium
class spell_talent_druid_of_the_mycelium : public AuraScript
{
    PrepareAuraScript(spell_talent_druid_of_the_mycelium);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TALENT_DRUID_OF_THE_MYCELIUM_DUMMY });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        if (!roll_chance_f(caster->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + 3)))
            return;
        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(healInfo->GetHeal(), aurEff->GetAmount()));
        eventInfo.GetActor()->CastSpell(nullptr, SPELL_TALENT_DRUID_OF_THE_MYCELIUM_DUMMY, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_druid_of_the_mycelium::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 94287 - Druid of the Mycelium (Proc)
class spell_talent_druid_of_the_mycelium_proc : public SpellScript
{
    PrepareSpellScript(spell_talent_druid_of_the_mycelium_proc);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TALENT_DRUID_OF_THE_MYCELIUM_PROC });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.size() < 2)
            return;

        targets.sort(Trinity::HealthPctOrderPred());

        WorldObject* target = targets.front();
        targets.clear();
        targets.push_back(target);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.AddSpellBP0(GetEffectValue());
        GetCaster()->CastSpell(target, SPELL_TALENT_DRUID_OF_THE_MYCELIUM_PROC, args);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_talent_druid_of_the_mycelium_proc::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
        OnEffectHitTarget += SpellEffectFn(spell_talent_druid_of_the_mycelium_proc::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 94540 - tg, the talent system doesn't provide a sufficient way to unlearn it atm
class spell_talent_titans_grip : public AuraScript
{
    PrepareAuraScript(spell_talent_titans_grip);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetCaster()->ToPlayer();
        player->SetCanTitanGrip(true);
    }
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetCaster()->ToPlayer();
        if (player->GetSession()->isLogingOut())
            return;
        player->SetCanTitanGrip(false);
        player->AutoUnequipOffhandIfNeed(true);
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_talent_titans_grip::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        //AfterEffectRemove += AuraEffectRemoveFn(spell_talent_titans_grip::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 94279 - Turret Totems
class spell_talent_turret_totems : public AuraScript
{
    PrepareAuraScript(spell_talent_turret_totems);

    Creature* GetTotem(Unit* caster, uint8 slot)
    {
        Creature* cre = caster->GetMap()->GetCreature(caster->m_SummonSlot[slot]);
        if (cre && cre->IsTotem() && caster->GetDistance(cre) <= 5.0f)
            return cre;
        return nullptr;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        Unit* target = eventInfo.GetProcTarget();
        if (!procSpell)
            return;

        Unit* caster = GetCaster();
        if (Creature* fireTotem = GetTotem(caster, SUMMON_SLOT_TOTEM_FIRE))
        {
            if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE)
                fireTotem->CastSpell(target, procSpell->Id, true);
        }

        if (Creature* earthTotem = GetTotem(caster, SUMMON_SLOT_TOTEM_EARTH))
        {
            if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE)
                earthTotem->CastSpell(target, procSpell->Id, true);
        }

        if (Creature* waterTotem = GetTotem(caster, SUMMON_SLOT_TOTEM_WATER))
        {
            if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST)
                waterTotem->CastSpell(target, procSpell->Id, true);
        }

        if (Creature* airTotem = GetTotem(caster, SUMMON_SLOT_TOTEM_AIR))
        {
            if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_ARCANE)
                airTotem->CastSpell(target, procSpell->Id, true);
        }

    }

    bool IsNearTotem(Unit* caster)
    {
        for (uint8 i = SUMMON_SLOT_TOTEM_FIRE; i < MAX_TOTEM_SLOT; ++i)
        {
            if (Creature* totem = caster->GetMap()->GetCreature(caster->m_SummonSlot[i]))
            {
                if (totem && totem->IsTotem() && caster->GetDistance(totem) <= 5.0f)
                    return true;
            }
        }

        return false;
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!IsNearTotem(caster))
        {
            if (!caster->HasAura(SPELL_TOTEM_TOTEM_WITHDRAWAL))
                caster->CastSpell(caster, SPELL_TOTEM_TOTEM_WITHDRAWAL, true);
        }
        else
        {
            if (caster->HasAura(SPELL_TOTEM_TOTEM_WITHDRAWAL))
                caster->RemoveAura(SPELL_TOTEM_TOTEM_WITHDRAWAL);
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (Aura* existingBuff = caster->GetAura(SPELL_TOTEM_TOTEM_WITHDRAWAL))
            caster->RemoveAura(SPELL_TOTEM_TOTEM_WITHDRAWAL);;
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_turret_totems::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_turret_totems::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_talent_turret_totems::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 94278 - champion
class spell_talent_champion : public AuraScript
{
    PrepareAuraScript(spell_talent_champion);

    enum Spell
    {
        champion_DR = 94611
    };

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ champion_DR });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetCaster()->ToPlayer();
        player->SetCanDodge(false);
    }
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetCaster()->ToPlayer();
        player->SetCanDodge(true);
    }
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Player* player = GetCaster()->ToPlayer();
        int32 dodgeRating = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + 2);
        int32 defRating = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + 1);
        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_CHAMPION))
        {
            existingBuff->GetEffect(EFFECT_2)->ChangeAmount(dodgeRating + defRating * 0.2);
        }

        float parryChance = player->GetFloatValue(PLAYER_PARRY_PERCENTAGE);
        if (Aura* existingBuff = player->GetAura(champion_DR))

        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(-(parryChance * 100 * 0, 3));
        }
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_talent_champion::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_talent_champion::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_champion::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 94275 - energy shield
class spell_talent_energy_shield : public AuraScript
{
    PrepareAuraScript(spell_talent_energy_shield);


    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        if (Aura* aura = GetAura())
        {
            AuraEffect* periodicEffect = aura->GetEffect(EFFECT_1);
            if (periodicEffect)
            {
                periodicEffect->SetAmplitude(5000);
                periodicEffect->ResetTicks();
            }
        }
    }
    void OnTick(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        int32 totalMana = caster->GetPower(POWER_MANA);
        int32 mp5 = caster->GetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER) * 5;

        if (Aura* aura = GetAura())
        {
            AuraEffect* periodicEffect = aura->GetEffect(EFFECT_1);
            if (periodicEffect->GetTickNumber() % 2 == 0)
            {
                periodicEffect->SetAmplitude(1000);
            }
        }
        if (Aura* existingBuff = caster->GetAura(SPELL_TALENT_ENERGY_SHIELD_BUFF))
        {
            int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
            int32 newAmount = existingAmount + mp5;
            if (existingAmount >= totalMana * 0.3f - mp5)
            {
                existingBuff->GetEffect(EFFECT_0)->ChangeAmount(totalMana * 0.3f);
            }
            else
            {
                existingBuff->GetEffect(EFFECT_0)->ChangeAmount(newAmount);
            }
        }
        else
        {
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(mp5);
            caster->CastSpell(caster, SPELL_TALENT_ENERGY_SHIELD_BUFF, args);
        }
    }
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (Aura* existingBuff = caster->GetAura(SPELL_TALENT_ENERGY_SHIELD_BUFF))
        {
            {
                caster->RemoveAura(SPELL_TALENT_ENERGY_SHIELD_BUFF);;
            }
        }
    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_energy_shield::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_talent_energy_shield::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_talent_energy_shield::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 94261 - Left Handed
class spell_talent_left_handed : public AuraScript
{
    PrepareAuraScript(spell_talent_left_handed);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = eventInfo.GetActor()->ToPlayer();
        uint32 spellIcon = eventInfo.GetSpellInfo()->SpellIconID;
        uint32 spellId = 0;

        for (uint32 id = 97700; id < 97800; ++id)
        {
            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(id);
            if (spellInfo && spellInfo->SpellIconID == spellIcon)
            {
                spellId = id;
                break;
            }
        }

        if (spellId != 0)
        {
            player->CastSpell(eventInfo.GetProcTarget(), spellId, true);
        }
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_left_handed::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_left_handed::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 94261 - Left Handed Passive
class spell_talent_left_handed_passive : public SpellScriptLoader
{
public:
    spell_talent_left_handed_passive() : SpellScriptLoader("spell_talent_left_handed_passive") { }

    class spell_talent_left_handed_passive_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_talent_left_handed_passive_AuraScript);
        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                {
                    caster->SetStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, BASE_PCT, 0.5f);
                }
            }
        }
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                {
                    caster->SetStatPctModifier(UNIT_MOD_DAMAGE_MAINHAND, BASE_PCT, 1.f);;
                }
            }
        }
        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_talent_left_handed_passive_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_talent_left_handed_passive_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };
    AuraScript* GetAuraScript() const override
    {
        return new spell_talent_left_handed_passive_AuraScript();
    }

};

// 94258 - blood drive
class spell_talent_blood_drive : public AuraScript
{
    PrepareAuraScript(spell_talent_blood_drive);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Player* player = eventInfo.GetActor()->ToPlayer();
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();;
        if (!procSpell)
            return;

        int32 healthCost = procSpell->CalcPowerCost(GetTarget(), eventInfo.GetSchoolMask());


        if (procSpell->PowerType == POWER_HEALTH)
        {
            if (Aura* existingBuff = player->GetAura(SPELL_TALENT_BLOOD_DRIVE_BUFF))
            {
                int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
                int32 newAmount = std::lroundf(existingAmount + healthCost * 0.2f);
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(newAmount);
                player->CastSpell(player, SPELL_TALENT_BLOOD_DRIVE_BUFF, args);
            }
            else
            {
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(healthCost * 0.2f);
                player->CastSpell(player, SPELL_TALENT_BLOOD_DRIVE_BUFF, args);
            }
        }
    }
    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_blood_drive::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 94259 - Blood Drive Buff
class spell_blood_drive_reduction : public AuraScript
{
    PrepareAuraScript(spell_blood_drive_reduction);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* player = GetCaster();

        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_BLOOD_DRIVE_BUFF))
        {
            int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
            int32 newAmount = existingAmount * 0.9;
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(newAmount);

        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_blood_drive_reduction::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 94252 - Secrets of Mana
class spell_talent_secrets_of_mana : public AuraScript
{
    PrepareAuraScript(spell_talent_secrets_of_mana);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Player* player = eventInfo.GetActor()->ToPlayer();
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        int32 manaCost = procSpell->CalcPowerCost(GetTarget(), eventInfo.GetSchoolMask());

        if (procSpell->PowerType == POWER_MANA)
        {
            // Find existing buff
            if (Aura* existingBuff = player->GetAura(SPELL_TALENT_SECRETS_OF_MANA_BUFF))
            {
                int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
                int32 newAmount = std::lroundf(existingAmount + manaCost * 0.5f);
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(newAmount);
                player->CastSpell(player, SPELL_TALENT_SECRETS_OF_MANA_BUFF, args);
            }
            else
            {
                // Cast new buff with initial amount
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(manaCost * 0.5f);
                player->CastSpell(player, SPELL_TALENT_SECRETS_OF_MANA_BUFF, args);
            }
        }
    }
    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_secrets_of_mana::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 94253 - Secrets of Mana Buff
class spell_talent_secrets_of_mana_reduction : public AuraScript
{
    PrepareAuraScript(spell_talent_secrets_of_mana_reduction);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* player = GetCaster();

        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_SECRETS_OF_MANA_BUFF))
        {
            int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
            int32 newAmount = existingAmount * 0.9;
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(newAmount);

        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_secrets_of_mana_reduction::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};


// - 94245 sacred echoes damage
class spell_talent_sacred_echoes_damage : public AuraScript
{
    PrepareAuraScript(spell_talent_sacred_echoes_damage);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ 94247 });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        HealInfo* healInfo = eventInfo.GetHealInfo();
        if (!healInfo || !healInfo->GetHeal())
            return;

        uint32 damage = CalculatePct(healInfo->GetHeal(), aurEff->GetAmount());
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(damage * 10);
        GetTarget()->CastSpell(victim, 94247, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_sacred_echoes_damage::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_sacred_echoes_damage::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// - 94246 sacred echoes heal
class spell_talent_sacred_echoes_heal : public AuraScript
{
    PrepareAuraScript(spell_talent_sacred_echoes_heal);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ 94248 });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        uint32 damage = CalculatePct(damageInfo->GetDamage(), aurEff->GetAmount());
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(damage * 10);
        GetTarget()->CastSpell(victim, 94248, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_sacred_echoes_heal::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_sacred_echoes_heal::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};



// - 94250 wandering affliction
class spell_talent_wandering_affliction : public AuraScript
{
    PrepareAuraScript(spell_talent_wandering_affliction);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ 94251 });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetProcTarget();
        int32 spirit = caster->GetStat(STAT_SPIRIT);

        if (!roll_chance_f(lround((float)spirit / 100 + 15)))
            return;

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        int32 amount = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(amount * 0.3);
        caster->CastSpell(target, 94251, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_wandering_affliction::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};





// - 94223 Acclimation
class spell_talent_acclimation : public AuraScript
{
    PrepareAuraScript(spell_talent_acclimation);


    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        //Player* player = eventInfo.GetActor()->ToPlayer();
        Unit* victim = eventInfo.GetProcTarget();

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;
        uint32 spellId = 0;

        if (eventInfo.GetTypeMask() & PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG)
        {
            if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY)
                spellId = 97323;
            else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE)
                spellId = 97324;
            else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE)
                spellId = 97325;
            else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST)
                spellId = 97326;
            else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW)
                spellId = 97327;
            else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_ARCANE)
                spellId = 97328;

        }
        GetTarget()->CastSpell(victim, spellId, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_acclimation::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_acclimation::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 97322 - Spell Eater
class spell_talent_spelleater : public AuraScript
{
    PrepareAuraScript(spell_talent_spelleater);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        PreventDefaultAction();

        Player* player = GetCaster()->ToPlayer();
        float ilvl = player->GetAverageItemLevel();
        int32 bp = std::lroundf(10 + ilvl * 0.05f);

        amount += int32(player->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_talent_spelleater::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_DONE);
    }
};

// 94241 - maelstrom weapon
class spell_class_maelstrom_weapon : public AuraScript
{
    PrepareAuraScript(spell_class_maelstrom_weapon);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_CLASS_HOLY_SLASH,
                SPELL_CLASS_FIRE_SLASH,
                SPELL_CLASS_LIGHTNING_SLASH,
                SPELL_CLASS_FROST_SLASH,
                SPELL_CLASS_SHADOW_SLASH,
                SPELL_CLASS_ARCANE_SLASH
            });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        
            PreventDefaultAction();

            Player* player = eventInfo.GetActor()->ToPlayer();

            SpellInfo const* procSpell = eventInfo.GetSpellInfo();
            if (!procSpell)
                return;


            uint32 spellId = 0;

            if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG)
            {
                if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY)
                    spellId = SPELL_CLASS_HOLY_SLASH;
                else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE)
                    spellId = SPELL_CLASS_FIRE_SLASH;
                else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE)
                    spellId = SPELL_CLASS_LIGHTNING_SLASH;
                else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST)
                    spellId = SPELL_CLASS_FROST_SLASH;
                else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW)
                    spellId = SPELL_CLASS_SHADOW_SLASH;
                else if (procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_ARCANE)
                    spellId = SPELL_CLASS_ARCANE_SLASH;
            }

            player->CastSpell(eventInfo.GetProcTarget(), spellId);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_maelstrom_weapon::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_maelstrom_weapon::HandleProc, EFFECT_2, SPELL_AURA_DUMMY);
    }
};

// 94206 - Lightning Overload
class spell_talent_lightning_overload : public AuraScript
{
    PrepareAuraScript(spell_talent_lightning_overload);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TALENT_OVERLOAD });
    }
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }
    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        Player* player = eventInfo.GetActor()->ToPlayer();
        int32 intel = player->GetStat(STAT_INTELLECT);

        if (!roll_chance_f(lround(10 + (float)intel * 0.02)))
            return;

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;
        uint32 spellId = eventInfo.GetSpellInfo()->Id;

        player->GetSpellHistory()->ResetCooldown(spellId);
        player->CastSpell(eventInfo.GetProcTarget(), spellId, true);

    }
    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_lightning_overload::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97001 - Seal of Righteousness 
class spell_class_seal_of_righteousness : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_righteousness);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_RIGHTEOUSNESS });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
        ap += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);

        int32 sph = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
        sph += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_HOLY);

        // Taking Weapon Speed of the Caster
        float mws = GetTarget()->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(OFF_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }

        int32 bp = std::lroundf(mws * (0.036f * ap + 0.05f * sph));
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_RIGHTEOUSNESS, args);

    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_righteousness::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_righteousness::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97011 - Seal of Command
class spell_class_seal_of_command : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_command);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_COMMAND_MH,
                                   SPELL_CLASS_SEAL_OF_COMMAND_OH,
                                   SPELL_CLASS_SEAL_OF_COMMAND_RANGED
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();



        //checking proc type
        uint32 spellId = 0;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)
            spellId = SPELL_CLASS_SEAL_OF_COMMAND_MH;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_COMMAND_OH;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_COMMAND_RANGED;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            spellId = SPELL_CLASS_SEAL_OF_COMMAND_RANGED;
        }
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), spellId);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_command::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_command::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97022 - Seal of Rockbiter
class spell_class_seal_of_rockbiter : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_rockbiter);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_ROCKBITER });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Player* player = GetCaster()->ToPlayer();
        Unit* victim = eventInfo.GetProcTarget();

        //int32 parry = player->GetMaxRatingValue(CR_PARRY);
        int32 armor = GetTarget()->GetTotalAuraModValue(UNIT_MOD_ARMOR);
        float ilvl = player->GetAverageItemLevel();



        int32 bp = std::lroundf(ilvl + armor * 0.1f);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_ROCKBITER, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_rockbiter::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_rockbiter::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Rockbiter Reflect
class spell_class_rockbiter_shield : public AuraScript
{
    PrepareAuraScript(spell_class_rockbiter_shield);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_ROCKBITER_REFLECT });
    }

    void OnAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        Unit* target = GetTarget();
        if (dmgInfo.GetAttacker() == target)
            return;

        target->CastSpell(dmgInfo.GetAttacker(), SPELL_CLASS_ROCKBITER_REFLECT, CastSpellExtraArgs(aurEff).AddSpellBP0(absorbAmount));
    }

    void Register() override
    {
        AfterEffectAbsorb += AuraEffectAbsorbFn(spell_class_rockbiter_shield::OnAbsorb, EFFECT_0);
    }
};


// 97031 - Seal of Spellblade 
class spell_class_seal_of_spellblade : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_spellblade);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_SPELLBLADE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        //Unit* caster = eventInfo.GetActor();
        return eventInfo.GetProcTarget() != nullptr;

    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        // Taking Arcane Spell Power of the Caster
        int32 spA = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE);
        spA += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_ARCANE);


        int32 mana = GetTarget()->GetMaxPower(POWER_MANA);
        int32 currentMana = GetTarget()->GetPower(POWER_MANA);

        // Damage calc of the hit
        int32 bp = std::lroundf(((0.01f * spA) + 0.02f * mana) * (1.f + currentMana * 0.00001f));

        // Mana Burn calc
        int32 manaCost = std::lroundf((float)250 * currentMana / mana);
        int32 powerCostMod = GetTarget()->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT, SPELL_SCHOOL_MASK_ARCANE);
        int32 finalManaCost = std::lroundf(manaCost * (1.f + (float)powerCostMod / 100));


        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        CastSpellExtraArgs args2(aurEff);
        args2.AddSpellMod(SpellValueMod(SPELLVALUE_BASE_POINT1), finalManaCost);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_SPELLBLADE, args);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_SPELLBLADE, args2);


        // Secrets of Mana interaction
        if (Aura* secretsOfManaBuff = GetTarget()->GetAura(SPELL_TALENT_SECRETS_OF_MANA))
        {
            int32 secretsOfManaArcanePower = std::lroundf(finalManaCost * 0.5f);

            if (Aura* existingBuff = GetTarget()->GetAura(SPELL_TALENT_SECRETS_OF_MANA_BUFF))
            {
                int32 existingAmount = existingBuff->GetEffect(EFFECT_0)->GetAmount();
                int32 newAmount = existingAmount + secretsOfManaArcanePower;

                CastSpellExtraArgs secretsArgs(aurEff);
                secretsArgs.AddSpellBP0(newAmount);
                GetTarget()->CastSpell(GetTarget(), SPELL_TALENT_SECRETS_OF_MANA_BUFF, secretsArgs);
            }
            else
            {
                CastSpellExtraArgs secretsArgs(aurEff);
                secretsArgs.AddSpellBP0(secretsOfManaArcanePower);
                GetTarget()->CastSpell(GetTarget(), SPELL_TALENT_SECRETS_OF_MANA_BUFF, secretsArgs);
            }
        }

    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_spellblade::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_spellblade::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97041 - Seal of Darktide
class spell_class_seal_of_darktide : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_darktide);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_DARKTIDE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();
        //Unit* caster = eventInfo.GetActor();

        // Taking Arcane Spell Power of the Caster
        int32 spa = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW);
        spa += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_SHADOW);


        // Taking Weapon Speed of the Caster
        float mws = GetTarget()->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(OFF_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }

        int32 hp = GetTarget()->GetMaxHealth();


        // Damage calculation of the hit
        int32 bp = std::lroundf(mws * ((0.01f * spa) + 0.004f * hp));
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_DARKTIDE, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_darktide::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_darktide::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97051 - Seal of Flametongue 
class spell_class_seal_of_flametongue : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_flametongue);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_FLAMETONGUE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();


        // Taking Fire Spell Power of the Caster
        int32 spf = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FIRE);
        spf += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_FIRE);

        // Taking Weapon Speed of the Caster
        float mws = GetTarget()->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(OFF_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            mws = GetTarget()->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
        }
        // Damage calculation of the hit
        int32 bp = std::lroundf(mws * (0.08f * spf) + (25 * mws));
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_FLAMETONGUE, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_flametongue::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_flametongue::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// flametongue passive
class spell_class_seal_of_flametongue_passive : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_flametongue_passive);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
            {
                float ilvl = player->GetAverageItemLevel();
                int32 bp = std::lroundf(50 + ilvl * 0.5f);
                amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));
            }
            else
            {
                amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), 50));
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_class_seal_of_flametongue_passive::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_DAMAGE_DONE);
    }
};


// 97062 - Seal of Light
class spell_class_seal_of_light : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_light);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_LIGHT });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
        ap += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);

        int32 sph = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
        sph += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_HOLY);

        int32 bp = std::lroundf(0.15f * ap + 0.15f * sph + 150);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_LIGHT, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_light::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_light::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Seal of Light heal
class spell_class_seal_of_light_heal : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_light_heal);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_LIGHT_HEAL });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
        ap += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);

        int32 sph = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
        sph += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_HOLY);

        int32 bp = std::lroundf(0.034f * ap + 0.034f * sph + 250);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_LIGHT_HEAL, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_light_heal::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_light_heal::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


//97071 - Seal of Venomstrike
template <uint32 DoTSpellId, uint32 DamageSpellId>
class spell_class_seal_of_venomstrike : public SpellScriptLoader
{
public:
    spell_class_seal_of_venomstrike(char const* ScriptName) : SpellScriptLoader(ScriptName) { }

    template <uint32 DoTSpell, uint32 DamageSpell>
    class spell_class_seal_of_venomstrike_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_class_seal_of_venomstrike_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    DoTSpell,
                    DamageSpell
                });
        }

        void HandleApplyDoT(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            // don't cast triggered, spell already has SPELL_ATTR4_CAN_CAST_WHILE_CASTING attr
            eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), DoTSpell, CastSpellExtraArgs(TRIGGERED_DONT_RESET_PERIODIC_TIMER).SetTriggeringAura(aurEff));
        }

        void HandleSeal(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* caster = eventInfo.GetActor();
            Unit* target = eventInfo.GetProcTarget();

            AuraEffect const* sealDot = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_CLASSLESS, 0x00082200, 0x00001000, 0x00000000, caster->GetGUID());
            if (!sealDot)
                return;

            uint8 const stacks = sealDot->GetBase()->GetStackAmount();
            uint8 const maxStacks = sealDot->GetSpellInfo()->StackAmount > 0 ? sealDot->GetSpellInfo()->StackAmount : 1;

            if (stacks < maxStacks && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS))
                return;

            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(DamageSpell);
            int32 amount = spellInfo->_effects[EFFECT_0].CalcValue();
            amount *= stacks;
            amount /= maxStacks;

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            caster->CastSpell(target, DamageSpell, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_class_seal_of_venomstrike_AuraScript::HandleApplyDoT, EFFECT_0, SPELL_AURA_DUMMY);
            OnEffectProc += AuraEffectProcFn(spell_class_seal_of_venomstrike_AuraScript::HandleSeal, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_class_seal_of_venomstrike_AuraScript<DoTSpellId, DamageSpellId>();
    }
};


// 97091 - Seal of Windfury
class spell_class_seal_of_windfury : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_windfury);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_WINDFURY_MH,
                                   SPELL_CLASS_SEAL_OF_WINDFURY_OH,
                                   SPELL_CLASS_SEAL_OF_WINDFURY_RANGED
        });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = eventInfo.GetActor()->GetCharmerOrOwnerPlayerOrPlayerItself();
        Unit* caster = eventInfo.GetActor();

        if (!player)
            return;

        float ilvl = player->GetAverageItemLevel();

        int32 extraAttackPower = ilvl * 4;

        if (!extraAttackPower)
            return;

        // Weapon dmg calculations mh
        //float ap = player->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 mws = caster->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;
        float mhdmg = (extraAttackPower / 14.f * mws);

        // Weapon dmg calculations oh
        int32 omws = caster->GetAttackTime(OFF_ATTACK);
        omws /= 1000.0f;
        float ohdmg = (extraAttackPower / 14.f * omws);

        //Weapon dmg calculations ranged
        //float rap = player->GetTotalAttackPowerValue(RANGED_ATTACK);
        int32 rws = caster->GetAttackTime(RANGED_ATTACK);
        rws /= 1000.0f;
        float rdmg = (extraAttackPower / 14.f * rws);

        //checking proc type
        uint32 spellId = 0;
        //WeaponAttackType attType = BASE_ATTACK;
        float wfdmg = mhdmg;
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)
            spellId = SPELL_CLASS_SEAL_OF_WINDFURY_MH;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_WINDFURY_OH;
            wfdmg = ohdmg;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_WINDFURY_RANGED;
            wfdmg = rdmg;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            spellId = SPELL_CLASS_SEAL_OF_WINDFURY_RANGED;
            wfdmg = rdmg;
        }
        // Value gained from additional AP
        int32 amount = static_cast<int32>(wfdmg);

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(amount);
        // Attack twice
        for (uint8 i = 0; i < 2; ++i)
            caster->CastSpell(eventInfo.GetProcTarget(), spellId, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_windfury::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_windfury::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


// 97101 - Seal of Bloodgrip
class spell_class_seal_of_bloodgrip : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_bloodgrip);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_BLOODGRIP_DUMMY });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();


        Unit* caster = GetTarget();
        Unit* victim = eventInfo.GetProcTarget();

        int ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
        ap += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);

        int rAp = GetTarget()->GetTotalAttackPowerValue(RANGED_ATTACK);
        rAp += victim->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);


        float mws = caster->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;
        int usedAp = ap;

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            mws = caster->GetAttackTime(OFF_ATTACK);
            mws /= 1000.0f;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            mws = caster->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
            usedAp = rAp;
        }
        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            mws = caster->GetAttackTime(RANGED_ATTACK);
            mws /= 1000.0f;
            usedAp = rAp;
        }
        //int apScaling = GetEffect(EFFECT_1)->GetAmount();


        int bp = std::lroundf(mws * 70  + mws * 0.06 * usedAp);

        
        SpellInfo const* bloodGripDot = sSpellMgr->AssertSpellInfo(SPELL_CLASS_SEAL_OF_BLOODGRIP_DUMMY);
        Aura* existingDot = victim->GetAura(SPELL_CLASS_SEAL_OF_BLOODGRIP_DUMMY, caster->GetGUID());
        if (existingDot)
        {
            AuraEffect* existingBleed = existingDot->GetEffect(EFFECT_0);
            if (existingBleed)
            {
                int remainingTicks = existingBleed->GetRemainingTicks();

                int tickAmount = existingBleed->GetAmount();
                int remainingDamage = tickAmount * remainingTicks;
                int addedDamage = remainingDamage / bloodGripDot->GetMaxTicks();

                bp += addedDamage;
            }
        }
        
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        caster->CastSpell(victim, SPELL_CLASS_SEAL_OF_BLOODGRIP_DUMMY, args);
        caster->CastSpell(victim, SPELL_CLASS_SEAL_OF_BLOODGRIP_BLEED, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_bloodgrip::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_bloodgrip::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};


//97102 - Blodgrip dot
class spell_class_seal_of_bloodgrip_dot : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_bloodgrip_dot);

    void OnTick(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        Unit* victim = GetTarget();

        Aura* existingDot = victim->GetAura(SPELL_CLASS_SEAL_OF_BLOODGRIP_BLEED, caster->GetGUID());
        if (existingDot)
        {
            AuraEffect* currentValue = GetAura()->GetEffect(EFFECT_0);
            if (currentValue)
            {
                int tickAmount = currentValue->GetAmount();
                existingDot->GetEffect(EFFECT_0)->ChangeAmount(tickAmount);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_class_seal_of_bloodgrip_dot::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};


// 96521 Serrated Shot - Bleed
class spell_class_serrated_shot_bleed : public AuraScript
{
    PrepareAuraScript(spell_class_serrated_shot_bleed);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        if (Unit* caster = GetCaster())
        {
            canBeRecalculated = false;

            // $0.3 * (($MWB + $mwb) / 2 + $AP / 14 * $MWS) bonus per tick
            float rap = caster->GetTotalAttackPowerValue(RANGED_ATTACK);
            int32 rws = caster->GetAttackTime(RANGED_ATTACK);
            float rwbMin = 0.f;
            float rwbMax = 0.f;
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                rwbMin += caster->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE, i);
                rwbMax += caster->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE, i);
            }

            float rwb = ((rwbMin + rwbMax) / 2 + rap * rws / 14000) * 0.3f;
            amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), rwb));
            uint32 spellId = GetSpellInfo()->Id;
            uint32 damage = amount;
            if (Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(spellId, SPELLMOD_DOT, damage);
            amount = damage;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_class_serrated_shot_bleed::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 96011 Aura of Steel
class spell_class_aura_of_steel : public AuraScript
{
    PrepareAuraScript(spell_class_aura_of_steel);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        if (Unit* caster = GetCaster())
        {
            canBeRecalculated = false;


            //Player* player = GetCaster()->ToPlayer();
            int32 armor = GetTarget()->GetTotalAuraModValue(UNIT_MOD_ARMOR);

            int32 bp = std::lroundf(armor * 0.05f);

            amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));

        }
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_class_aura_of_steel::CalculateAmount, EFFECT_1, SPELL_AURA_DAMAGE_SHIELD);
    }
};

// -94212 - Ignite
class spell_talent_ignite : public AuraScript
{
    PrepareAuraScript(spell_talent_ignite);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TALENT_IGNITE });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() && eventInfo.GetProcTarget();
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        SpellInfo const* igniteDot = sSpellMgr->AssertSpellInfo(SPELL_TALENT_IGNITE);
        int32 pct = 5;
        Unit* victim = eventInfo.GetProcTarget();
        Unit* caster = GetTarget();

        int32 amount = int32(CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), pct));

        Aura* existingDot = victim->GetAura(SPELL_TALENT_IGNITE, caster->GetGUID());
        if (existingDot)
        {
            AuraEffect* existingIgnite = existingDot->GetEffect(EFFECT_0);
            if (existingIgnite)
            {
                int remainingTicks = existingIgnite->GetRemainingTicks();

                int tickAmount = existingIgnite->GetAmount();
                int remainingDamage = tickAmount * remainingTicks;
                int addedDamage = remainingDamage / igniteDot->GetMaxTicks();

                amount += addedDamage;
            }
        }

        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(amount);
        caster->CastSpell(victim, SPELL_TALENT_IGNITE, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_talent_ignite::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_ignite::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 49143 - Frost Strike
class spell_class_frost_strike_damage_frozen : public SpellScriptLoader
{
public:
    spell_class_frost_strike_damage_frozen() : SpellScriptLoader("spell_class_frost_strike_damage_frozen") { }

    class spell_class_frost_strike_damage_frozen_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_frost_strike_damage_frozen_SpellScript);

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();


            if (target->HasAuraState(AURA_STATE_FROZEN) || caster->GetAura(SPELL_CLASS_FINGERS_OF_FROST))
            {
                SetHitDamage(GetHitDamage() * 2);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_class_frost_strike_damage_frozen_SpellScript::HandleDamage, EFFECT_2, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_frost_strike_damage_frozen_SpellScript();
    }
};

// 49184 - Howling Blast
class spell_class_howling_blast_frozen : public SpellScriptLoader
{
public:
    spell_class_howling_blast_frozen() : SpellScriptLoader("spell_class_howling_blast_frozen") { }

    class spell_class_howling_blast_frozen_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_class_howling_blast_frozen_SpellScript);

        void HandleOnHit()
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();


            if (target->HasAuraState(AURA_STATE_FROZEN) || caster->GetAura(SPELL_CLASS_FINGERS_OF_FROST))
            {
                GetCaster()->CastSpell(target, 55095, true);
            }

        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_class_howling_blast_frozen_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_class_howling_blast_frozen_SpellScript();
    }
};

enum DancingRuneWeaponMisc
{
    DATA_INITIAL_TARGET_GUID = 1,
};
enum DeathKnightMisc
{
    NPC_DK_DANCING_RUNE_WEAPON = 27893,

};


// 94263 - Dancing Rune Weapon
class spell_talent_dancing_rune_weapon : public AuraScript
{
    PrepareAuraScript(spell_talent_dancing_rune_weapon);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({           });
    }

    void HandleTarget(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Creature*> runeWeapons;
        caster->GetAllMinionsByEntry(runeWeapons, NPC_DK_DANCING_RUNE_WEAPON);
        for (Creature* temp : runeWeapons)
        {
            if (temp->IsAIEnabled())
                temp->AI()->SetGUID(GetTarget()->GetGUID(), DATA_INITIAL_TARGET_GUID);
            temp->GetThreatManager().RegisterRedirectThreat(GetId(), caster->GetGUID(), 100);



            Aura* matchingAura = nullptr;
            for (auto const& auraPair : caster->GetAppliedAuras())
            {
                Aura* aura = auraPair.second->GetBase();
                SpellInfo const* spellInfo = aura->GetSpellInfo();

                if (spellInfo->GetSpellSpecific() == SPELL_SPECIFIC_SEAL)
                {
                    matchingAura = aura;
                    break;
                }
            }

            if (matchingAura)
            {
                uint32 spellId = matchingAura->GetId();
                temp->CastSpell(temp, spellId, true);
            }
        }
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {


        if (SpellInfo const* procSpell = eventInfo.GetSpellInfo())
        {
            if (procSpell->GetSpellSpecific() == SPELL_SPECIFIC_SEAL ||
                procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW ||
                procSpell->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY ||
                eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS)

                return true;
        }

        return false;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* owner = GetUnitOwner();
        if (!owner)
            return;

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        Unit* runeWeapon = nullptr;
        for (auto itr = owner->m_Controlled.begin(); itr != owner->m_Controlled.end() && !runeWeapon; itr++)
            if ((*itr)->GetEntry() == NPC_DK_DANCING_RUNE_WEAPON)
                runeWeapon = *itr;

        if (!runeWeapon)
            return;

        if (runeWeapon->IsInCombat() && runeWeapon->GetVictim())
            runeWeapon->CastSpell(runeWeapon->GetVictim(), procSpell->Id, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_talent_dancing_rune_weapon::HandleTarget, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoCheckProc += AuraCheckProcFn(spell_talent_dancing_rune_weapon::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_talent_dancing_rune_weapon::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 94282 - DRW Hidden Passive
class spell_talent_drw_passive : public AuraScript

{
    PrepareAuraScript(spell_talent_drw_passive);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        int32 currentFocus = GetTarget()->GetPower(POWER_FOCUS);
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Creature*> minions;
        caster->GetAllMinionsByEntry(minions, NPC_DK_DANCING_RUNE_WEAPON);
        for (Creature* minion : minions)
        {
            if (Aura* existingBuff = caster->GetAura(SPELL_TALENT_RUNE_WEAPON))
                if (currentFocus < 15)
                {
                    minion->DespawnOrUnsummon();
                    caster->RemoveAura(SPELL_TALENT_RUNE_WEAPON);
                    caster->RemoveAura(SPELL_TALENT_RUNE_WEAPON_HIDDEN_PASSIVE);
                }
                else
                {
                    caster->CastSpell(caster, SPELL_TALENT_RUNE_WEAPON_DRAIN, true);
                }
        }
    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_drw_passive::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};


// 94281 - DRW Debuff
class spell_talent_drw_debuff : public AuraScript
{
    PrepareAuraScript(spell_talent_drw_debuff);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (Aura* existingBuff = caster->GetAura(SPELL_TALENT_RUNE_WEAPON))
        {
            caster->RemoveAura(SPELL_TALENT_RUNE_DEBUFF);
        }
        else
        {
            caster->CastSpell(caster, SPELL_TALENT_RUNE_DEBUFF, true);
        }
    }
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (Aura* existingBuff = caster->GetAura(SPELL_TALENT_RUNE_DEBUFF))
        {
            {
                caster->RemoveAura(SPELL_TALENT_RUNE_DEBUFF);
            }
        }
    }
    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_talent_drw_debuff::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_drw_debuff::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 94592 - Blade Barrier
class spell_talent_blade_barrier : public AuraScript
{
    PrepareAuraScript(spell_talent_blade_barrier);

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ spellInfo->GetEffect(EFFECT_0).TriggerSpell });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = GetTarget();
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), aurEff->GetAmount()));
        caster->CastSpell(caster, aurEff->GetSpellEffectInfo().TriggerSpell, args);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_talent_blade_barrier::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// Mirage Talon - Damage - Int
class spell_item_mirage_talon_int : public AuraScript
{
    PrepareAuraScript(spell_item_mirage_talon_int);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ITEM_MIRAGE_TALON_INT });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        float intellect = GetTarget()->GetStat(STAT_INTELLECT);

        float dmg = intellect * 0.1;

        int32 bp = static_cast<int32>(std::lroundf(dmg)) * (std::rand() % 10 + 1);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_ITEM_MIRAGE_TALON_INT, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_item_mirage_talon_int::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_item_mirage_talon_int::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Mirage Talon - Attackspeed - Agi
class spell_item_mirage_talon_agi : public AuraScript
{
    PrepareAuraScript(spell_item_mirage_talon_agi);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();
        int32 agility = player->GetStat(STAT_AGILITY);

        if (Aura* existingBuff = player->GetAura(SPELL_ITEM_MIRAGE_TALON_AGI))
        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(agility * 0.01);
        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_mirage_talon_agi::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Cinderbreaker - Damage - Str
class spell_item_cinderbreaker_str : public AuraScript
{
    PrepareAuraScript(spell_item_cinderbreaker_str);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ITEM_CINDERBREAKER_STR });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        float str = GetTarget()->GetStat(STAT_STRENGTH);

        float dmg = str * 0.1;

        int32 bp = static_cast<int32>(std::lroundf(dmg)) * (std::rand() % 7 + 4);
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_ITEM_CINDERBREAKER_STR, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_item_cinderbreaker_str::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_item_cinderbreaker_str::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Golem's Embrace
class spell_item_golems_embrace : public AuraScript
{
    PrepareAuraScript(spell_item_golems_embrace);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();
        int32 strength = player->GetStat(STAT_STRENGTH);


        if (Aura* existingBuff = player->GetAura(SPELL_ITEM_GOLEMS_EMBRACE))
        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(strength * 0.02);
        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_golems_embrace::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Kinetic Bolt
class spell_class_kinetic_bolt : public SpellScript
{
    PrepareSpellScript(spell_class_kinetic_bolt);

    void HandleEffect(SpellEffIndex /*effIndex*/)
    {
        int32 mana = GetCaster()->GetMaxPower(POWER_MANA);
        if (Unit* victim = GetHitUnit())

            SetEffectValue(mana * 0.1);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_class_kinetic_bolt::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 93012 - Proficiency
class spell_talent_proficiency : public AuraScript
{
    PrepareAuraScript(spell_talent_proficiency);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();
        int32 expertiseMH = player->GetUInt32Value(PLAYER_EXPERTISE);
        int32 expertiseOH = player->GetUInt32Value(PLAYER_OFFHAND_EXPERTISE);
        int32 higherExpertise = (expertiseMH > expertiseOH) ? expertiseMH : expertiseOH;

        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_PROFICIENCY))
        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(higherExpertise * 10);
            existingBuff->GetEffect(EFFECT_1)->ChangeAmount(higherExpertise * 10);
            existingBuff->GetEffect(EFFECT_2)->ChangeAmount(higherExpertise * (-10));
        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_proficiency::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 93138 - Deadly Calm
class spell_talent_deadly_calm : public AuraScript
{
    PrepareAuraScript(spell_talent_deadly_calm);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        Player* player = caster->ToPlayer();
        int32 maxFocus = GetCaster()->GetMaxPower(POWER_FOCUS);

        if (Aura* existingBuff = player->GetAura(SPELL_TALENT_DEADLY_CALM))
        {
            existingBuff->GetEffect(EFFECT_0)->ChangeAmount(maxFocus);
            existingBuff->GetEffect(EFFECT_1)->ChangeAmount(maxFocus);
        }

    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_talent_deadly_calm::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

class spell_custom_ilvlmanacost : public AuraScript
{
    PrepareAuraScript(spell_custom_ilvlmanacost);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
            {
                float ilvl = player->GetAverageItemLevel();
                int32 bp = 0;

                if (ilvl < 100)
                {
                    bp = (100 - ilvl) * -1;
                    if (bp < -50)
                        bp = -50;
                }

                amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_custom_ilvlmanacost::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT);
    }
};

void AddSC_Spells_Custom_Class_scripts()
{
    new spell_class_scourge_strike();
    new spell_class_envenom();
    new spell_class_eviscerate();
    new spell_class_howling_blast_frozen();
    new spell_class_frost_strike_damage_frozen();
    new spell_class_seal_of_venomstrike<SPELL_CLASS_DEADLY, SPELL_CLASS_SEAL_OF_VENOMSTRIKE_DAMAGE>("spell_class_seal_of_venomstrike");
    new spell_talent_left_handed_passive();
    new spell_class_empowered_attack();
    new spell_class_fingers_of_frost();
    new spell_class_deep_freeze();
    RegisterSpellScript(spell_class_seal_of_righteousness);
    RegisterSpellScript(spell_class_seal_of_command);
    RegisterSpellScript(spell_class_seal_of_rockbiter);
    RegisterSpellScript(spell_class_rockbiter_shield);
    RegisterSpellScript(spell_class_seal_of_spellblade);
    RegisterSpellScript(spell_class_seal_of_darktide);
    RegisterSpellScript(spell_class_seal_of_flametongue);
    RegisterSpellScript(spell_class_seal_of_flametongue_passive);
    RegisterSpellScript(spell_class_seal_of_light);
    RegisterSpellScript(spell_class_seal_of_light_heal);
    RegisterSpellScript(spell_class_seal_of_windfury);
    RegisterSpellScript(spell_class_seal_of_bloodgrip);
    RegisterSpellScript(spell_class_seal_of_bloodgrip_dot);
    RegisterSpellScript(spell_class_serrated_shot_bleed);
    RegisterSpellScript(spell_class_aura_of_steel);
    RegisterSpellScript(spell_talent_ignite);
    RegisterSpellScript(spell_talent_lightning_overload);
    RegisterSpellScript(spell_class_maelstrom_weapon);
    RegisterSpellScript(spell_talent_spelleater);
    RegisterSpellScript(spell_talent_acclimation);
    RegisterSpellScript(spell_talent_sacred_echoes_damage);
    RegisterSpellScript(spell_talent_sacred_echoes_heal);
    RegisterSpellScript(spell_talent_wandering_affliction);
    RegisterSpellScript(spell_talent_secrets_of_mana);
    RegisterSpellScript(spell_talent_secrets_of_mana_reduction);
    RegisterSpellScript(spell_talent_blood_drive);
    RegisterSpellScript(spell_blood_drive_reduction);
    RegisterSpellScript(spell_talent_left_handed);
    RegisterSpellScript(spell_talent_dancing_rune_weapon);
    RegisterSpellScript(spell_talent_energy_shield);
    RegisterSpellScript(spell_talent_champion);
    RegisterSpellScript(spell_talent_turret_totems);
    RegisterSpellScript(spell_talent_titans_grip);
    RegisterSpellScript(spell_talent_druid_of_the_mycelium_proc);
    RegisterSpellScript(spell_talent_druid_of_the_mycelium);
    RegisterSpellScript(spell_talent_drw_passive);
    RegisterSpellScript(spell_talent_drw_debuff);
    RegisterSpellScript(spell_talent_basilisk_bite);
    RegisterSpellScript(spell_talent_cruicible_of_faith);
    RegisterSpellScript(spell_talent_blessed_life);
    RegisterSpellScript(spell_class_empowered_attack_stacks);
    RegisterSpellScript(spell_item_lotus_restore);
    RegisterSpellScript(spell_item_stat_to_haste);
    RegisterSpellScript(spell_talent_absolute_zero);
    RegisterSpellScript(spell_class_fingers_of_frost_stacks);
    RegisterSpellScript(spell_talent_blade_barrier);
    RegisterSpellScript(spell_talent_prodigy);
    RegisterSpellScript(spell_talent_brainstorm);
    RegisterSpellScript(spell_item_mirage_talon_int);
    RegisterSpellScript(spell_item_mirage_talon_agi);
    RegisterSpellScript(spell_item_cinderbreaker_str);
    RegisterSpellScript(spell_item_golems_embrace);
    RegisterSpellScript(spell_class_kinetic_bolt);
    RegisterSpellScript(spell_talent_proficiency);
    RegisterSpellScript(spell_talent_deadly_calm);
    RegisterSpellScript(spell_custom_ilvlmanacost);

};
