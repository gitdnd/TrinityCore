
#ifndef _GROUPAFFIXMGR_H
#define _GROUPAFFIXMGR_H

#include <Define.h>
#include <Creature.h>
#include <list>
#include <map>


class AffixMgr
{
public:
	void LoadDatabaseData();
	
	AffixItem* getAffixItem(uint32 id);
	AffixEffect* GetAffixEffect(uint32 id);

private:
	std::list<AffixItem> m_items;
	std::list<AffixEffect> m_effects;
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
	AffixEffect(uint32 id, uint32 baseSpell, uint32 targetSpell, uint8 rank);

	uint32 const GetId() { return m_id; }
    uint32 const GetBaseSpell() { return m_baseSpell; }
	uint32 const GetTargetSpell() { return m_targetSpell; }
	uint8 const GetRank() { return m_rank; }

	void Apply(Creature* creature);

private:
	uint32 m_id;
    uint32 m_baseSpell;
    uint32 m_targetSpell;
	uint8 m_rank;
};

#define sAffixMgr AffixMgr::instance()
#endif
