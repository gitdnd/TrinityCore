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

bool SpellSupports::ProcGeneric(float chance)
{
    if (frand(1.f, 0.f) >= chance)
    {
        return true
    }
    return false;
}
bool SpellSupports::Proc20Pct()
{
    ProcGeneric(0.2f);
}
bool SpellSupports::Proc30Pct()
{
    ProcGeneric(0.3f);
}
bool SpellSupports::Proc40Pct()
{
    ProcGeneric(0.4f);
}

bool Unit::ModSpellSupport(uint32 spellSupport, uint32 supportData, uint32 spell = 0, bool add = true)
{
    if (spell)
    {
        if (add)
        {
            if (!GemSupports.count(spell))
                GemSupports.emplace(spell, SpellSupports(spellSupport, supportData));
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
                         [spellSupport, supportData](const SpellSupports& genSup)
                         { return genSup.SpellSupportFunction == spellSupport && genSup.SupportData == supportData; });
        if (add)
        {
            if (it == GenericSupports.end())
            {
                GenericSupports.push_back(SpellSupports(spellSupport, supportData));
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
                    ModSpellSupport(supp.SupportType, supp.SecondData, supp.Spell);
                }
    }
    else
    {
        // code for mobs that use support system
        return;
    }
}
