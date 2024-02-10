
#include <AffixMgr.h>
#include "DatabaseEnv.h"
#include "World.h"

///////////////////////
// Affix Manager
///////////////////////

void AffixMgr::LoadDatabaseData()
{
    if (QueryResult result = WorldDatabase.Query("SELECT itemid, `rank` FROM affix_item"))
    {
        do
        {
            Field* fields = result->Fetch();
            m_items.push_back(
                AffixItem(
                    fields[0].GetUInt32(),
                    fields[1].GetUInt8()));
        } while (result->NextRow());
    }
    if (QueryResult result = WorldDatabase.Query("SELECT id, `rank`, baseSpell, targetSpell, dungeonLevelBonus FROM affix_effect"))
    {
        do
        {
            Field* fields = result->Fetch();
            m_effects.push_back(
                AffixEffect(
                    fields[0].GetUInt32(),
                    fields[2].GetUInt32(),
                    fields[3].GetUInt32(),
                    fields[4].GetInt32(),
                    fields[1].GetUInt8()));
        } while (result->NextRow());
    }
}

AffixMgr* AffixMgr::instance()
{
    static AffixMgr instance;
    return &instance;
}

AffixItem AffixMgr::GetAffixItem(uint32 id)
{
    for (AffixItem item : m_items)
    {
        if (item.GetId() == id)
        {
            return item;
        }
    }
    return AffixItem(0, 0);
}

AffixEffect AffixMgr::GetAffixEffect(uint32 id)
{
    if (id > 0)
    {
        for (AffixEffect effect : m_effects)
        {
            if (effect.GetId() == id)
            {
                return effect;
            }
        }
    }
    return AffixEffect(0, 0, 0, 0, 0);
}

int AffixMgr::GetDungeonLevelBonus(uint32 affix1, uint32 affix2, uint32 affix3, uint32 affix4)
{
    int bonus = 0;
    uint32 affixes[] = {affix1, affix2, affix3, affix4};
    for (uint32 affix : affixes)
    {
        bonus += GetAffixEffect(affix).GetDungeonLevelBonus();
    }
    return bonus;
}

///////////////////////
// Affix Item
///////////////////////

AffixItem::AffixItem(uint32 id, uint8 rank) :
    m_id(id), m_rank(rank)
{
}

///////////////////////
// Affix Effect
///////////////////////

AffixEffect::AffixEffect(uint32 id, uint32 baseSpell, uint32 targetSpell, int dungeonLevelBonus, uint8 rank) :
    m_id(id), m_baseSpell(baseSpell), m_targetSpell(targetSpell), m_dungeonLevelBonus(dungeonLevelBonus), m_rank(rank)
{
}

void AffixEffect::Apply(Creature* creature, uint8 event)
{
    if (GetId() == 0)
        return;

    // Spawn/Leave combat
    if (event > 1) {
        // FIXME: This is a temporary implementation, just cast whatever spell set
        if (!creature->HasAura(GetTargetSpell()))
            creature->AddAura(GetTargetSpell(), creature);

        // TODO: Handle affix specific logic
    }
    // Add to world
    else if (event == 1)
    {
        // Apply health/damage scaling here
    }
}
