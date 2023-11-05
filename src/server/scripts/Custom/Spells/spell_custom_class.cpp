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
    SPELL_CLASS_SEAL_OF_BLOODGRIP_MAINHAND = 97102,
    SPELL_CLASS_SEAL_OF_BLOODGRIP_OFFHAND = 97103,
    SPELL_CLASS_SEAL_OF_BLOODGRIP_RANGED = 97104
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

        int32 bp = std::lroundf(mws * (0.030f * ap + 0.035f * sph));
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

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = eventInfo.GetActor()->ToPlayer();


        //checking proc type
        uint32 spellId = 0;
        WeaponAttackType attType = BASE_ATTACK;

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
        player->CastSpell(eventInfo.GetProcTarget(), spellId);
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



        int32 bp = std::lroundf(ilvl + armor * 0.05f);
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
        Unit* caster = eventInfo.GetActor();
        return eventInfo.GetProcTarget() != nullptr;

    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetProcTarget();

        // Taking Arcane Spell Power of the Caster
        int32 spa = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE);
        spa += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_ARCANE);


        int32 mana = GetTarget()->GetMaxPower(POWER_MANA);
        int32 currentmana = GetTarget()->GetPower(POWER_MANA);

        // Damage calculation of the hit
        int32 bp = std::lroundf(((0.01f * spa) + 0.02f * mana) * (1.f + currentmana * 0.00001f));

        // Damage reduction when below 300 mana
        if (currentmana < 300)
        {
            bp = std::lroundf(((0.01f * spa) + 0.02f * mana) * (currentmana / 300.f));
        }
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(bp);
        GetTarget()->CastSpell(victim, SPELL_CLASS_SEAL_OF_SPELLBLADE, args);
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
        Unit* caster = eventInfo.GetActor();

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
        int32 bp = std::lroundf(mws * (0.055f * spf) + (25 * mws));
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

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        if (Unit* caster = GetCaster())
        {
            Player* player = GetCaster()->ToPlayer();
            float ilvl = player->GetAverageItemLevel();
            int32 bp = std::lroundf(50 + ilvl * 0.5f);
            amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));
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

        int32 bp = std::lroundf(0.15f * ap + 0.15f * sph);
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

        int32 bp = std::lroundf(0.034f * ap + 0.034f * sph);
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

            // get current aura on target, if any. SPELLFAMILY_ROGUE and 0x00000800 probably has to be changed.
            AuraEffect const* sealDot = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_ROGUE, 0x00000000, 0x00000800, 0x00000000, caster->GetGUID());
            if (!sealDot)
                return;

            uint8 const stacks = sealDot->GetBase()->GetStackAmount();
            uint8 const maxStacks = sealDot->GetSpellInfo()->StackAmount;

            if (stacks < maxStacks && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS))
                return;

            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(DamageSpell);
            int32 amount = spellInfo->Effects[EFFECT_0].CalcValue();
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

        Player* player = eventInfo.GetActor()->ToPlayer();


        float ilvl = player->GetAverageItemLevel();

        int32 extraAttackPower = ilvl * 4;

        if (!extraAttackPower)
            return;

        // Weapon dmg calculations mh
        float ap = player->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 mws = player->GetAttackTime(BASE_ATTACK);
        mws /= 1000.0f;
        float mhdmg = (extraAttackPower / 14.f * mws);

        // Weapon dmg calculations oh
        int32 omws = player->GetAttackTime(OFF_ATTACK);
        omws /= 1000.0f;
        float ohdmg = (extraAttackPower / 14.f * omws);

        //Weapon dmg calculations ranged
        float rap = player->GetTotalAttackPowerValue(RANGED_ATTACK);
        int32 rws = player->GetAttackTime(RANGED_ATTACK);
        rws /= 1000.0f;
        float rdmg = (extraAttackPower / 14.f * rws);

        //checking proc type
        uint32 spellId = 0;
        WeaponAttackType attType = BASE_ATTACK;
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
            player->CastSpell(eventInfo.GetProcTarget(), spellId, args);
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
        return ValidateSpellInfo({ SPELL_CLASS_SEAL_OF_BLOODGRIP_MAINHAND,
                                   SPELL_CLASS_SEAL_OF_BLOODGRIP_OFFHAND,
                                   SPELL_CLASS_SEAL_OF_BLOODGRIP_RANGED });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetProcTarget() != nullptr;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = eventInfo.GetActor()->ToPlayer();

        uint32 spellId = 0;

        spellId = SPELL_CLASS_SEAL_OF_BLOODGRIP_MAINHAND;

        // Offhand and Ranged Proc

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_BLOODGRIP_OFFHAND;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_RANGED_AUTO_ATTACK)
        {
            spellId = SPELL_CLASS_SEAL_OF_BLOODGRIP_RANGED;
        }

        if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)
        {
            spellId = SPELL_CLASS_SEAL_OF_BLOODGRIP_RANGED;
        }

        player->CastSpell(eventInfo.GetProcTarget(), spellId);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_class_seal_of_bloodgrip::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_class_seal_of_bloodgrip::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};
// Blodgrip Passive
class spell_class_seal_of_bloodgrip_passive : public AuraScript
{
    PrepareAuraScript(spell_class_seal_of_bloodgrip_passive);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        if (Unit* caster = GetCaster())
        {
            Player* player = GetCaster()->ToPlayer();
            float ilvl = player->GetAverageItemLevel();
            int32 bp = std::lroundf(25 + ilvl * 0.25f);
            amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), bp));
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_class_seal_of_bloodgrip_passive::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_DAMAGE_DONE);
    }
};


// 700051 Serrated Shot - Bleed
class spell_class_serrated_shot_bleed : public AuraScript
{
    PrepareAuraScript(spell_class_serrated_shot_bleed);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        if (Unit* caster = GetCaster())
        {
            canBeRecalculated = false;

            // $0.2 * (($MWB + $mwb) / 2 + $AP / 14 * $MWS) bonus per tick
            float rap = caster->GetTotalAttackPowerValue(RANGED_ATTACK);
            int32 rws = caster->GetAttackTime(RANGED_ATTACK);
            float rwbMin = 0.f;
            float rwbMax = 0.f;
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                rwbMin += caster->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE, i);
                rwbMax += caster->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE, i);
            }

            float rwb = ((rwbMin + rwbMax) / 2 + rap * rws / 14000) * 0.2f;
            amount += int32(caster->ApplyEffectModifiers(GetSpellInfo(), aurEff->GetEffIndex(), rwb));

        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_class_serrated_shot_bleed::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

void AddSC_Spells_Custom_Class_scripts()
{
    new spell_class_seal_of_venomstrike<SPELL_CLASS_DEADLY, SPELL_CLASS_SEAL_OF_VENOMSTRIKE_DAMAGE>("spell_class_seal_of_venomstrike");
    RegisterAuraScript(spell_class_seal_of_righteousness);
    RegisterAuraScript(spell_class_seal_of_command);
    RegisterAuraScript(spell_class_seal_of_rockbiter);
    RegisterAuraScript(spell_class_rockbiter_shield);
    RegisterAuraScript(spell_class_seal_of_spellblade);
    RegisterAuraScript(spell_class_seal_of_darktide);
    RegisterAuraScript(spell_class_seal_of_flametongue);
    RegisterAuraScript(spell_class_seal_of_flametongue_passive);
    RegisterAuraScript(spell_class_seal_of_light);
    RegisterAuraScript(spell_class_seal_of_light_heal);
    RegisterAuraScript(spell_class_seal_of_windfury);
    RegisterAuraScript(spell_class_seal_of_bloodgrip);
    RegisterAuraScript(spell_class_seal_of_bloodgrip_passive);
    RegisterAuraScript(spell_class_serrated_shot_bleed);
};
