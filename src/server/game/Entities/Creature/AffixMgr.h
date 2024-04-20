
#ifndef _GROUPAFFIXMGR_H
#define _GROUPAFFIXMGR_H

#include <Define.h>
#include <Creature.h>
#include <Group.h>
#include <list>
#include <map>

enum AffixEvent
{
    AFFIX_EVENT_ADD_TO_WORLD = 1,
    AFFIX_EVENT_LEAVE_COMBAT = 2,
    AFFIX_EVENT_UPDATE_ENTRY = 3,
    AFFIX_EVENT_RESPAWN      = 4,
    AFFIX_EVENT_REACH_HOME   = 5
};

class AffixGroupSlot
{
public:
    AffixGroupSlot();
    AffixGroupSlot(uint32 affixId, int numRolls, uint8 rank);

    uint32 const GetAffixId() { return m_affixId; }
    int const GetNumRolls() { return m_numRolls; }
    uint8 const GetRank() { return m_rank; }

private:
    uint32 m_affixId;
    int m_numRolls;
    uint8 m_rank;
};

class AffixGroup
{
public:
    AffixGroup();

    void SetSlot(int slot, uint32 affixId, int numRolls, uint8 rank);
    uint32 GetAffixId(uint8 slot);
    int GetNumRolls(uint8 slot);
    uint8 GetRank(uint8 slot);

private:
    AffixGroupSlot m_slots[4];
};

class AffixItem
{
public:
    AffixItem(uint32 id, uint8 rank);

    uint32 const GetId() { return m_id; };
    uint8 const GetRank() { return m_rank; }

private:
    uint32 m_id;
    uint8 m_rank;
};

class AffixEffect
{
public:
    AffixEffect(uint32 id, uint32 baseSpell, uint32 targetSpell, int m_dungeonLevelBonus, uint8 rank);

    uint32 const GetId() { return m_id; }
    uint32 const GetBaseSpell() { return m_baseSpell; }
    uint32 const GetTargetSpell() { return m_targetSpell; }
    int const GetDungeonLevelBonus() { return m_dungeonLevelBonus; }
    uint8 const GetRank() { return m_rank; }

    void Apply(Creature* creature, AffixEvent event);

private:
    uint32 m_id;
    uint32 m_baseSpell;
    uint32 m_targetSpell;
    int m_dungeonLevelBonus;
    uint8 m_rank;
};

class AffixMgr
{
public:
	void LoadDatabaseData();
	
	AffixItem GetAffixItem(uint32 id);
	AffixEffect GetAffixEffect(uint32 id);

    int GetDungeonLevelBonus(uint32 affix1, uint32 affix2, uint32 affix3, uint32 affix4);

    AffixGroup& GetAffixGroup(Group* group);
    void ClearAffixGroup(Group* group);


    static AffixMgr* instance();

private:
	std::list<AffixItem> m_items;
	std::list<AffixEffect> m_effects;
    std::map<uint32, AffixGroup> m_groups;
};

#define sAffixMgr AffixMgr::instance()
#endif
