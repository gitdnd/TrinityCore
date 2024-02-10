
#ifndef _GROUPAFFIXMGR_H
#define _GROUPAFFIXMGR_H

#include <Define.h>
#include <Creature.h>
#include <list>
#include <map>

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

    void Apply(Creature* creature, uint8 event = 0);

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

    static AffixMgr* instance();

private:
	std::list<AffixItem> m_items;
	std::list<AffixEffect> m_effects;
};

#define sAffixMgr AffixMgr::instance()
#endif
