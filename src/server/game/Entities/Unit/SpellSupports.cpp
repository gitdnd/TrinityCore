#include "Unit.h"
#include "Player.h"

void Unit::DoBeforeSpellCastScripts(Spell* spell)
{
    CallScriptIteration(CallScriptBeforeSpellCast(spell));
}

void Unit::DoOnSpellCastScripts(Spell* spell)
{
    CallScriptIteration(CallScriptOnSpellCast(spell));
}

void Unit::DoOnAuraStackScripts(Aura* aura, int16 amount)
{
    CallScriptIteration(CallScriptOnAuraStack(aura, amount));
}

void Unit::CastSpellFromSupport(Spell* spell, uint32 id)
{
    TriggerCastFlags static const flags =
        TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_CAST_DIRECTLY |
                         TRIGGERED_IGNORE_SET_FACING | TRIGGERED_DONT_REPORT_CAST_ERROR);
    if (spell->m_targets.GetUnitTarget())
        CastSpell(spell->m_targets.GetUnitTarget(), id, flags);
    else if (WorldLocation const* pos = m_targets.GetDstPos())
        player->CastSpell(spell->pos->GetPositionX(), spell->pos->GetPositionY(), spell->pos->GetPositionZ(), id, flags);
}

SpellSupports::SupportPhase Unit::ConvertEventToPhase(uint32 event)
{
    switch (event)
    {
        case SPELL_EFFECT_HANDLE_HIT:
            return ON_HIT;
        default:
            return 0;
    }
}
void SpellSupports::DoSupport(Spell* spell, uint32 phase)
{
    if (!ConvertEventToPhase(phase) & Phase)
        return;
    AllSupportSpells[SpellSupportFunction](spell);
}
bool SpellSupports::ProcGeneric(Spell* spell, float chance)
{
    if (frand(1.f, 0.f) >= chance)
    {
        Owner->CastSpellFromSupport(spell, supportData);
        return true
    }
    return false;
}
bool SpellSupports::Proc20Pct(Spell* spell)
{
    return ProcGeneric(0.2f);
}
bool SpellSupports::Proc30Pct(Spell* spell)
{
    return ProcGeneric(0.3f);
}
bool SpellSupports::Proc40Pct(Spell* spell)
{
    return ProcGeneric(0.4f);
}

bool Unit::ModSpellSupport(uint32 spellSupport, uint32 supportData, uint32 phase, uint32 spell = 0, bool add = true)
{
    if (spell)
    {
        if (add)
        {
            if (!GemSupports.count(spell))
                GemSupports.emplace(spell, SpellSupports(this, spellSupport, supportData, phase));
            GemSupports[spell].Amount++;
            return true;
        }
        else
        {
            if (!GemSupports.count(spell))
                return true;
            SpellSupports* ss = &GemSupports[spell];
            ss->Amount--;
            if (!ss->Amount)
            {
                GemSupports.erase(spell);
                return true;
            }
            return false;
        }
    }
    else
    {
        auto it =
            std::find_if(GenericSupports.begin(), GenericSupports.end(),
                         [spellSupport, supportData, phase](const SpellSupports& genSup)
            { return genSup.SpellSupportFunction == spellSupport && genSup.SupportData == supportData&&, genSup.Phase == phase; });
        if (add)
        {
            if (it == GenericSupports.end())
            {
                GenericSupports.push_back(SpellSupports(this, spellSupport, supportData, phase));
            }
            it->Amount++;
            return true;
        }
        else
        {
            if (it == GenericSupports.end())
            {
                return true;
            }
            it->Amount--;
            if (!it->Amount)
            {
                GenericSupports.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}
void Unit::RecountSpellSupports()
{
    GemSupports.clear();
    GenericSupports.clear();
    if (Player* player = ToPlayer())
    {
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
            if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (auto& supp : pItem->h_allSupportGems)
                {
                    ModSpellSupport(supp.SupportType, supp.SecondData, supp.Spell, supp.Phase);
                }
    }
    else
    {
        // code for mobs that use support system
        return;
    }
}
