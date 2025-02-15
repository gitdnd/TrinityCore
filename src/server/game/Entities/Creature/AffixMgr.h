
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

    uint32 const GetAffixId() const { return m_affixId; }
    int const GetNumRolls() const { return m_numRolls; }
    uint8 const GetRank() const { return m_rank; }

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
    AffixGroupSlot m_slots[MAX_AFFIXES];
};

class AffixItem
{
public:
    AffixItem(uint32 id, uint8 rank);

    uint32 const GetId() const { return m_id; };
    uint8 const GetRank() const { return m_rank; }

private:
    uint32 m_id;
    uint8 m_rank;
};

class AffixEffect
{
public:
    AffixEffect(uint32 id, uint32 baseSpell, uint32 targetSpell, uint32 m_dungeonLevelBonus, uint8 rank, bool applyToBoss, bool applyToSummon);

    uint32 const GetId() const { return m_id; }
    uint32 const GetBaseSpell() const { return m_baseSpell; }
    uint32 const GetTargetSpell() const { return m_targetSpell; }
    uint32 const GetDungeonLevelBonus() const { return m_dungeonLevelBonus; }
    uint8 const GetRank() const { return m_rank; }
    bool const GetApplyToBoss() const { return m_applyToBoss; }
    bool const GetApplyToSummon() const { return m_applyToSummon; }

    void Apply(Creature* creature, AffixEvent event) const;

private:
    uint32 m_id;
    uint32 m_baseSpell;
    uint32 m_targetSpell;
    uint32 m_dungeonLevelBonus;
    uint8 m_rank;
    bool m_applyToBoss;
    bool m_applyToSummon;
};

class AffixMgr
{
public:
	void LoadDatabaseData();
	
	AffixItem GetAffixItem(uint32 id);
	AffixEffect GetAffixEffect(uint32 id);

    uint32 GetDungeonLevelBonus(uint32* affixes);

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
