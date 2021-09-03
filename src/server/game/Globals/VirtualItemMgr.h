#include "Define.h" // uint32 etc types
#include "ItemTemplate.h" // Item enums
#include "DBCStores.h" // Needed for item DBC lookup
#include "SFMTRand.h"
#include <map>
#include <random>
#include <set>
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/tss.hpp>

#ifndef VIRTUAL_ITEM_MGR_H
#define VIRTUAL_ITEM_MGR_H

class ObjectMgr;
class World;

enum StatGroup
{
    STAT_GROUP_HEALING,
    STAT_GROUP_INT_DPS,
    STAT_GROUP_STR_DPS,
    STAT_GROUP_STR_TANK,
    STAT_GROUP_AGI_DPS,
    STAT_GROUP_AGI_TANK,
    STAT_GROUP_AGI_RANGED,
    STAT_GROUP_ALL,
    STAT_GROUP_COUNT,
    STAT_GROUP_RANDOM = STAT_GROUP_COUNT,
};

enum StatGroupType
{
    STAT_GROUP_TYPE_PRIMARY,
    STAT_GROUP_TYPE_SECONDARY,
    STAT_GROUP_TYPE_GEMS,
    STAT_GROUP_TYPE_COUNT
};

struct VirtualItemTemplate : ItemTemplate
{
    VirtualItemTemplate(ItemTemplate const* base) : ItemTemplate(*base), base_entry(base->ItemId)
    {
        FlagsCu &= ~ITEM_FLAGS_CU_VIRTUAL_ITEM_BASE; // virtual item should not be revirtualized?
        //Bonding = BIND_WHEN_PICKED_UP; // All items MUST be bound on pickup so they can not be mailed and thus failing some cleanup and memory management
    }

    /**
     * Entry of the original item used to generate the template.
     */
    uint32 base_entry;

    /**
     * The items randomly generated seed
     */
    uint32 seed;

    /**
     * Updates the displayId used by the item from DBC data to match the current item entry's displayid.
     */
    void UpdateDisplay();

    StatGroup statGroup;

    // Seeds for various regeneration from item mods.
    uint32 socketSeed;
    uint32 qualitySeed;
    uint32 statSeed;
    uint32 nameSeed;
    uint32 displaySeed;
    uint32 spellSeed;
    uint32 statValueSeed;
};

struct VirtualModifier
{
    VirtualModifier();

    /**
     * Different modifiers that can be edited to change the output when the modifier is used to generate stats.
     */
    uint32 ilevel;
    uint8 quality;
    int16 statpool;
    StatGroup statgroup;
    uint32 seed;
    uint32 socketSeed;
    uint32 qualitySeed;
    uint32 statSeed;
    uint32 nameSeed;
    uint32 displaySeed;
    uint32 spellSeed;
    uint32 statValueSeed;
    uint32 plrAvgLvl;
    bool isCrafted = false;
    uint32 vLvlMod;

    class StatGroupData
    {
    public:
        /**
         * Generates the premade stat groups and similar in addition to constructing the object itself.
         */
        StatGroupData();

        /**
         * Returns the primary stats for the given stat group.
         */
        std::vector<ItemModType> const& GetStatGroupPrimaryStats(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const;

        /**
         * Returns the secondary stats for the given stat group.
         */
        std::vector<ItemModType> const& GetStatGroupSecondaryStats(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const;


        /**
         * Returns the sockets for the given stat group.
         */
        std::vector<SocketColor> const& GetStatGroupSockets(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const;

        /**
         * Returns the stat groups for the given armor subclass.
         */
        std::vector<StatGroup> const& GetArmorSubclassStatGroups(VirtualItemTemplate* item) const;

    private:

        std::vector<ItemModType> stat_group_primary_stats[STAT_GROUP_COUNT];
        std::vector<ItemModType> stat_group_secondary_stats[STAT_GROUP_COUNT];
        std::vector<SocketColor> stat_group_sockets[STAT_GROUP_COUNT];
        std::vector<StatGroup> armor_type_stat_groups[MAX_ITEM_SUBCLASS_ARMOR];
    };

    /**
     * A static object used to fetch different predefined stat groups and similar.
     * The object has various methods to access these lists.
     */
    static StatGroupData const premadeStatGroupData;

    /**
     * Fetches the rate (point*rate = stat_amount) for the given item equip type.
     * Returns the stat rate.
     */
    static float GetSlotStatModifier(VirtualItemTemplate* item);

    /**
     * Fetches the armor modifier for the subclass and inventory type combination.
     * Returns the armor modifier.
     */
    static float GetTypeSlotArmorModifier(VirtualItemTemplate* item);

    /**
     * Fetches the rate (point*rate = stat_amount) for the given stat type.
     * Returns the stat rate.
     */
    static float GetStatRate(ItemModType stat);
};

struct itemSpellInfo
{
    itemSpellInfo() { spellId = 0; }
    itemSpellInfo(uint32 sId, uint32 qual, int32 iClass, int32 sub, uint32 iType, int8 iGroup, int32 minILvL, int32 maxILvL, uint32 sTrig, int32 sCharge, float PPM, int32 CD, uint32 sCat, int32 SCC) : spellId(sId), quality(qual),
        itemClass(iClass), subClass(sub), inventoryType(iType), statGroup(iGroup), minItemLevel(minILvL), maxItemLevel(maxILvL), SpellTrigger(sTrig), SpellCharges(sCharge), SpellPPMRate(PPM), SpellCooldown(CD), SpellCategory(sCat), SpellCategoryCooldown(SCC) {}
    uint32 spellId;
    uint32 quality;
    int32 itemClass;
    int32 subClass;
    int32 inventoryType;
    int8 statGroup;
    int32 minItemLevel;
    int32 maxItemLevel;
    uint32 SpellTrigger;
    int32  SpellCharges;
    float  SpellPPMRate;
    int32  SpellCooldown;
    uint32 SpellCategory;
    int32  SpellCategoryCooldown;
};

class VirtualItemMgr
{
    friend class ObjectMgr;
    friend class World;
public:
    typedef std::unordered_map<uint32, VirtualItemTemplate*> Store;

    typedef boost::shared_mutex LockType;
    typedef boost::shared_lock<LockType> ReadGuard;
    typedef boost::unique_lock<LockType> WriteGuard;

    /**
     * minEntry and maxEntry are used as the boundaries for the entries the EntryGenerators use.
     * The whole range defined by minEntry and maxEntry can be divided for the EntryGenerators created in VirtualItemMgr.
     * All virtual entries are assumed to be between this range.
     */
    static const uint32 minEntry = 1000000;
    static const uint32 maxEntry = 0xFFFFFF;
    static_assert(minEntry < maxEntry, "Min entry must be smaller than max entry");

    /**
     * armorSubclasses and weaponSubclasses contain the valid subclasses for items of armor and weapon.
     */
    static const std::vector<InventoryType> armorInventoryType;
    static const std::vector<ItemSubclassWeapon> weaponSubclasses;

    /**
     * Returns a single static instance of VirtualItemMgr.
     */
    static VirtualItemMgr& instance();

    /**
     * Not thread safe.
     * Loads all possible stat group entries from the stat group table into memory.
     */
    void LoadStatGroupInfoFromDB();

    struct StatGroupInfo
    {
        StatGroupInfo() {}
        StatGroupInfo(int32 group, int32 type, int32 stat) : statGroup(group), statType(type), statId(stat) {}
        StatGroupInfo(int32 group, int32 type, int32 stat, std::string n) : statGroup(group), statType(type), statId(stat), comment(n) {}
        int32 statGroup;
        int32 statType;
        int32 statId;
        std::string comment;
    };

	/**
	 * Not thread safe.
	 * Loads all possible names from the generator table into memory.
	 */
	void LoadNamesFromDB();

	struct NameInfo
	{
		NameInfo() {}
		NameInfo(int32 type, int32 sub, int32 iType, int32 arrid) : itemType(type), subclass(sub), inventoryType(iType), array_id(arrid) {}
		NameInfo(int32 type, int32 sub, int32 iType, int32 arrid, std::string n) : itemType(type), subclass(sub), inventoryType(iType), array_id(arrid), name(n) {}
		int32 itemType;
		int32 subclass;
        int32 inventoryType;
		int32 array_id;
		std::string name;
	};

    /**
     * Generate virtual level lookup array from ilevel 1 to 325.
     */

    void GenerateVirtualLevelLookupArray();
    int32 GetVirtualLevel(float ilevel) const;

    class VirtualLevelInfo
    {
        friend class VirtualItemMgr;

        VirtualLevelInfo() {}
        VirtualLevelInfo(float ilevel) : iLevel(ilevel) {}
        float iLevel;
    };

    /**
     * Not thread safe.
     * Loads all possible displays from the generator table into memory.
     */

    void LoadDisplaysFromDB();

    struct displayInfo
    {
        displayInfo(uint32 iQ, uint32 _class, uint32 subclass, uint32 inventoryType, uint32 dId) : quality(iQ), iClass(_class), isubClass(subclass), iInventoryType(inventoryType), displayId(dId) {}
        displayInfo(uint32 iQ, uint32 _class, uint32 subclass, uint32 inventoryType) : quality(iQ), iClass(_class), isubClass(subclass), iInventoryType(inventoryType) {}
        uint32 quality;
        uint32 iClass;
        uint32 isubClass;
        uint32 iInventoryType;
        uint32 displayId;
    };

    /**
      * Not thread safe.
      * Loads all possible spells from the generator table into memory.
      */

    void LoadSpellsFromDB();

    /**
     * Returns a randomly generated item display depending on item type, subclass and quality
     */
    uint32 GenerateItemDisplay(VirtualItemTemplate* item, VirtualModifier modifier) const;

    /**
      * Returns a randomly generated item spell depending on item type, subclass and quality
      */
    itemSpellInfo GenerateSpell(VirtualItemTemplate* item, VirtualModifier modifier);

    /**
     * Generates a randoml item name depending on item type, subclass and quality
     */
    void GenerateItemName(VirtualItemTemplate* item, VirtualModifier modifier, bool reRoll = false) const;

	/**
	 * Return a vector of available names for the specified subclass.
	 */
	std::vector<std::string> GetNamesForNameInfo(NameInfo* info) const;

    /**
     * Return a vector of available displays for the specified requirements.
     */
    std::list<uint32> GetDisplaysForDisplayInfo(VirtualItemTemplate* item, bool qualityOverride = false) const;

    /**
     * Return a vector of available spells for the specified requirements.
     */
    //std::list<itemSpellInfo> GetSpells(VirtualItemTemplate* item) const;

    /**
     * Creates all used generators and sets their entry ranges in addition to constructing the object itself.
     */
    VirtualItemMgr();

    /**
     * Deletes all stored virtual templates in addition to deleting the object itself.
     */
    ~VirtualItemMgr();

    /**
     * Fetches the VirtualItemTemplate assigned for the unique entry.
     * Returns VirtualItemTemplate if found, else returns null.
     */
    VirtualItemTemplate * GetVirtualTemplate(uint32 entry);

    /**
     * Uses passed base and modifier to generate a new VirtualItemTemplate.
     * Returns the newly created VirtualItemTemplate.
     */
    VirtualItemTemplate* GenerateVirtualTemplate(ItemTemplate const* base, VirtualModifier modifier = VirtualModifier());


    /**
     * Used to regenerate item info of virtual items.
     */
    void RegenerateItemInfo(VirtualItemTemplate* output, VirtualModifier modifier);

    /**
     * Uses passed modifier to generate stats and edits output to have the generated stats.
     */
    void GenerateStats(VirtualItemTemplate* output, VirtualModifier modifier = VirtualModifier(), bool reRoll = false) const;

    /**
     * Uses passed modifier to generate stats and edits output to have the generated stats.
     */
    void GenerateItemStats(VirtualItemTemplate* output, VirtualModifier modifier = VirtualModifier(), bool reRoll = false) const;


    /**
     * Converts a virtual player/group level into a usable item level.
     */
    float GenerateItemLevel(int32 virtualLevel) const;

    /**
     * Checks if the passed template is a valid virtual item template.
     * Returns true if it is, false if it is not.
     */
    static bool IsVirtualTemplate(ItemTemplate const* base);

    void GenerateSockets(VirtualItemTemplate* output, VirtualModifier modifier = VirtualModifier(), bool reRoll = false);
    void GenerateSpells(VirtualItemTemplate* output, VirtualModifier modifier = VirtualModifier(), bool reRoll = false);
    void GenerateQuality(VirtualItemTemplate* output, VirtualModifier modifier = VirtualModifier(), bool reRoll = false);
    void GenerateAdditonalStat(VirtualItemTemplate* output);
    void UpdateDisenchantId(VirtualItemTemplate* output);
    void InitSeedGen(VirtualModifier modifier = VirtualModifier());

private:

    class EntryGenerator
    {
        friend class VirtualItemMgr;
    public:
        /**
         * Not thread safe. Crashes if no entry found or entry not between assumed range.
         * Generates a new entry from the generator's range that is not used in passed container.
         * Returns unique unused entry.
         */
        uint32 GenerateEntry(Store const& store);

        /**
         * Not thread safe.
         * Checks the current entry that is to be used next.
         * Returns next unique unused entry.
         */
        uint32 PeekNext() const;

        /**
         * Generates a new EntryGenerator with entry range 0-1 by default.
         */
        EntryGenerator();

        /**
         * Crashes if min >= max.
         * Generates a new EntryGenerator with given range.
         */
        EntryGenerator(uint32 min, uint32 max);
        
    private:
        uint32 nextEntry;
        uint32 minEntry;
        uint32 maxEntry;
    };

    LockType lock;
    Store store;
    std::unordered_map<InventoryType, EntryGenerator> armorGenerator;
    std::unordered_map<ItemSubclassWeapon, EntryGenerator> weaponGenerator;
    std::vector<uint32> freed_entries;

    std::map<uint32, VirtualLevelInfo> virtual_level_info;
    std::vector<StatGroupInfo> stat_group_info;
	std::vector<NameInfo> availableNames;
    std::vector<displayInfo> availableDisplays;
    std::vector<itemSpellInfo> availableSpells;

    /**
     * Not thread safe.
     * Fetches the EntryGenerator used for items with same type as passed ItemTemplate.
     * Returns EntryGenerator* if it succeeds, null if it fails.
     */
    EntryGenerator* Generator(ItemTemplate* temp);

    /**
     * Not thread safe.
     * Used to insert virtual items on startup when loading database.
     * Returns true if it succeeds and generates a new entry if needed, false if it fails.
     */
    bool InsertEntry(VirtualItemTemplate* virtualItem);
};

#define sVirtualItemMgr VirtualItemMgr::instance()

#endif
