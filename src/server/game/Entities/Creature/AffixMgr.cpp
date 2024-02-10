
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
    if (QueryResult result = WorldDatabase.Query("SELECT id, `rank`, baseSpell, targetSpell FROM affix_effect"))
    {
        do
        {
            Field* fields = result->Fetch();
            m_effects.push_back(
                AffixEffect(
                    fields[0].GetUInt32(),
                    fields[2].GetUInt32(),
                    fields[3].GetUInt32(),
                    fields[1].GetUInt8()));
        } while (result->NextRow());
    }
}

AffixMgr* AffixMgr::instance()
{
    static AffixMgr instance;
    return &instance;
}

AffixItem* AffixMgr::getAffixItem(uint32 id)
{
    for (AffixItem item : m_items)
    {
        if (item.GetId() == id)
        {
            return &item;
        }
    }
    return nullptr;
}

AffixEffect* AffixMgr::GetAffixEffect(uint32 id)
{
    if (id == 0)
        return nullptr;
    for (AffixEffect effect : m_effects)
    {
        if (effect.GetId() == id)
        {
            return &effect;
        }
    }
    return nullptr;
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

AffixEffect::AffixEffect(uint32 id, uint32 baseSpell, uint32 targetSpell, uint8 rank) :
    m_id(id), m_baseSpell(baseSpell), m_targetSpell(targetSpell), m_rank(rank)
{
}

void AffixEffect::Apply(Creature* creature, uint8 event)
{
    // Spawn/Leave combat
    if (event > 1) {
        // FIXME: This is a temporary implementation, just cast whatever spell set
        if (!creature->HasAura(GetTargetSpell()))
            creature->AddAura(GetTargetSpell(), creature);
    }
    // Add to world
    else if (event == 1)
    {
        // Debugging
        creature->SetMaxHealth(1000);
        creature->SetHealth(1000);
    }
}
