
#include "AffixMgr.h"
#include "DatabaseEnv.h"
#include "SpellAuras.h"
#include "World.h"
#include "string.h"

///////////////////////
// Affix Manager
///////////////////////
///////////////////////
//// AffixMgr Database
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

///////////////////////
//// AffixMgr API
///////////////////////

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

int AffixMgr::GetDungeonLevelBonus(uint32* affixes)
{
    int bonus = 0;
    for (uint8 i = 0; i < MAX_AFFIXES; ++i)
    {
        bonus += GetAffixEffect(affixes[i]).GetDungeonLevelBonus();
    }

    return bonus;
}

AffixGroup& AffixMgr::GetAffixGroup(Group* group)
{
    uint32 guid = group->GetLowGUID();
    if (!m_groups.count(guid))
        m_groups[guid] = AffixGroup();
    return m_groups[guid];
}

void AffixMgr::ClearAffixGroup(Group* group)
{
    std::map<uint32, AffixGroup>::iterator iter = m_groups.find(group->GetLeaderGUID());
    if (iter != m_groups.end())
        m_groups.erase(iter);
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

void AffixEffect::Apply(Creature* creature, AffixEvent event)
{
    if (GetId() == 0)
        return;

    // Only apply affixes to enemies
    const FactionTemplateEntry* selfFaction = sFactionTemplateStore.LookupEntry(creature->GetFaction());
    const FactionTemplateEntry* companionFaction = sFactionTemplateStore.LookupEntry(1665); // Timeway Companion faction
    if (selfFaction->IsFriendlyTo(*companionFaction))
        return;

    // If unselectable
    if (creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) ||
        // if GM trigger npc
        creature->IsTrigger() ||
        // if critter
        (creature->GetCreatureTemplate()->type == CREATURE_TYPE_CRITTER) ||
        // if totem
        (creature->GetCreatureTemplate()->type == CREATURE_TYPE_TOTEM) ||
        // if pet
        creature->IsPet() ||
        // if not alive
        !creature->IsAlive())
        // then do nothing
        return;

    // debug
    //creature->Yell("Applying affix based on event: " + std::to_string(event), LANG_UNIVERSAL);
 
    switch (event)
    {
    case AFFIX_EVENT_UPDATE_ENTRY:
    case AFFIX_EVENT_LEAVE_COMBAT:
    case AFFIX_EVENT_RESPAWN:
    {
        // Apply target spell or increase stacks up to 4
        if (!creature->HasAura(GetTargetSpell()))
            creature->AddAura(GetTargetSpell(), creature);
        else
        {
            if (Aura* aura = creature->GetAura(GetTargetSpell()))
            {
                uint8 amount = aura->GetStackAmount();
                if (amount < 4)
                    aura->SetStackAmount(amount + 1);
            }
        }
        break;
    }
    default:
        break;
    }
}

///////////////////////
// Affix Group & Slot
///////////////////////

AffixGroupSlot::AffixGroupSlot() :
    m_affixId(0), m_numRolls(0), m_rank(0)
{
}

AffixGroupSlot::AffixGroupSlot(uint32 affixId, int numRolls, uint8 rank) :
    m_affixId(affixId), m_numRolls(numRolls), m_rank(rank)
{
}

AffixGroup::AffixGroup()
{
}

void AffixGroup::SetSlot(int slot, uint32 affixId, int numRolls, uint8 rank)
{
    m_slots[slot - 1] = AffixGroupSlot(affixId, numRolls, rank);
}

uint32 AffixGroup::GetAffixId(uint8 slot)
{
    return m_slots[slot - 1].GetAffixId();
}

int AffixGroup::GetNumRolls(uint8 slot)
{
    return m_slots[slot - 1].GetNumRolls();
}

uint8 AffixGroup::GetRank(uint8 slot)
{
    return m_slots[slot - 1].GetRank();
}

