#include "VirtualItemMgr.h"
#include "Errors.h" // ASSERT macro
#include "SharedDefines.h" // item quality enum
#include "World.h" // config values
#include <algorithm>
#include <cstdlib>
#include "Random.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "SFMTRand.h"

VirtualModifier::VirtualModifier() : ilevel(0), quality(MAX_ITEM_QUALITY), statpool(-1), statgroup(STAT_GROUP_RANDOM), seed(0), plrAvgLvl(0), vLvlMod(0),
socketSeed(0), qualitySeed(0), statSeed(0), nameSeed(0), displaySeed(0), spellSeed(0), statValueSeed(0)
{
}

VirtualItemMgr & VirtualItemMgr::instance()
{
    static VirtualItemMgr obj;
    return obj;
}

VirtualItemMgr::VirtualItemMgr()
{
    WriteGuard guard(lock);

    // Some basic code to divide the given entry range between all item types
    const uint32 block = std::floor((double)(maxEntry - minEntry) / (weaponSubclasses.size() + armorInventoryType.size()));
    size_t blockCounter = 0;

    for (auto InventoryType : armorInventoryType)
    {
        uint32 min = minEntry + (blockCounter*block);
        uint32 max = min + block - 1;
        armorGenerator[InventoryType] = EntryGenerator(min, min + block - 1);
        ++blockCounter;
        TC_LOG_INFO("server.loading", "ItemGenerator Inventory[%d] %d - %d", static_cast<int>(InventoryType), min, max);
    }

    for (auto subclass : weaponSubclasses)
    {
        uint32 min = minEntry + (blockCounter*block);
        uint32 max = min + block - 1;
        weaponGenerator[subclass] = EntryGenerator(min, min + block - 1);
        ++blockCounter;
        TC_LOG_INFO("server.loading", "ItemGenerator Weapon[%d] %d - %d", subclass, min, max);
    }
}

VirtualItemMgr::~VirtualItemMgr()
{
    WriteGuard guard(lock);
    for (auto it : store)
    {
        delete it.second;
    }
    store.clear();
}

// Database loading

void VirtualItemMgr::LoadStatGroupInfoFromDB()
{
    WriteGuard guard(lock);

    uint32 count = 0;
    uint32 beginTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_stat_group_info`");

    if (!result)
    {
        TC_LOG_INFO("server.loading", "Loaded 0 available virtual item stat group info entries, table item_generator_stat_group_info is empty.");
        return;
    }

    do {
        Field* fields = result->Fetch();
        int32 statGroup = fields[0].GetInt32();
        int32 statType = fields[1].GetInt32();
        int32 statId = fields[2].GetInt32();
        std::string comment = fields[3].GetString();

        stat_group_info.push_back(StatGroupInfo(statGroup, statType, statId, comment));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item stat group entries in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

void VirtualItemMgr::LoadNamesFromDB()
{
	WriteGuard guard(lock);

	uint32 count = 0;
	uint32 beginTime = getMSTime();

	QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_names`");

	if (!result)
	{
		TC_LOG_INFO("server.loading", "Loaded 0 available virtual item names, table item_generator_names is empty.");
		return;
	}

	do{
		Field* fields = result->Fetch();
		int32 itemType = fields[0].GetInt32();
		int32 subclass = fields[1].GetInt32();
        int32 inventoryType = fields[2].GetInt32();
		int32 array_id = fields[3].GetInt32();
		std::string name = fields[4].GetString();

		availableNames.push_back(NameInfo(itemType, subclass, inventoryType, array_id, name));
		++count;
	} while (result->NextRow());

	TC_LOG_INFO("server.loading", "Loaded %u available virtual item names in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

void VirtualItemMgr::LoadDisplaysFromDB()
{
    WriteGuard guard(lock);

    uint32 count = 0;
    uint32 beginTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_displays`");

    if (!result)
    {
        TC_LOG_INFO("server.loading", "Loaded 0 available virtual item displays, table item_generator_displays is empty.");
        return;
    }

    do {
        Field* fields = result->Fetch();
        uint32 quality = fields[0].GetUInt32();
        uint32 classType = fields[1].GetUInt32();
        uint32 subclassType = fields[2].GetUInt32();
        uint32 inventoryType = fields[3].GetUInt32();
        uint32 displayId = fields[4].GetUInt32();

        availableDisplays.push_back(displayInfo(quality, classType, subclassType, inventoryType, displayId));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item displays in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

void VirtualItemMgr::LoadSpellsFromDB()
{
    WriteGuard guard(lock);

    uint32 count = 0;
    uint32 beginTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_spells`");

    if (!result)
    {
        TC_LOG_INFO("server.loading", "Loaded 0 available virtual item spells, table item_generator_spells is empty.");
        return;
    }

    do {
        Field* fields = result->Fetch();
        uint32 spellId = fields[0].GetUInt32();
        uint32 quality = fields[1].GetUInt32();
        int32 itemClass = fields[2].GetInt32();
        int32 subClass = fields[3].GetInt32();
        int32 inventoryType = fields[4].GetInt32();
        int8 statGroup = fields[5].GetInt8();
        int8 minItemLevel = fields[6].GetInt32();
        int8 maxItemLevel = fields[7].GetInt32();
        uint32 SpellTrigger = fields[8].GetUInt32();
        int32  SpellCharges = fields[9].GetInt32();
        float  SpellPPMRate = fields[10].GetFloat();
        int32  SpellCooldown = fields[11].GetInt32();
        uint32 SpellCategory = fields[12].GetUInt32();
        int32  SpellCategoryCooldown = fields[13].GetInt32();

        availableSpells.push_back(itemSpellInfo(spellId, quality, itemClass, subClass, inventoryType, statGroup, minItemLevel, maxItemLevel, SpellTrigger, SpellCharges, SpellPPMRate, SpellCooldown, SpellCategory, SpellCategoryCooldown));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item spells in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

// Generators

void VirtualItemMgr::RegenerateItemInfo(VirtualItemTemplate* output, VirtualModifier modifier)
{
    GenerateQuality(output, modifier);
    GenerateStats(output, modifier);
    GenerateItemName(output, modifier);
    UpdateDisenchantId(output);
    GenerateSockets(output, modifier);
    GenerateSpells(output, modifier);
    GenerateItemStats(output, modifier);
    bool isTrinket = output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET;
    bool isRing = output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_FINGER;
    uint32 display = isTrinket || isRing ? 0 : GenerateItemDisplay(output, modifier);
    if (display == 0)
        output->UpdateDisplay();
    else
        output->DisplayInfoID = display;
}

void initSeed(uint32& val, std::mt19937 generator)
{
    if(!val)
        val = urand(std::numeric_limits<uint32>::min(), std::numeric_limits<uint32>::max(), generator);
}

void VirtualItemMgr::InitSeedGen(VirtualModifier& modifier)
{
    // instantiate RNG
    std::mt19937 generator;
    generator.seed(modifier.seed);
    initSeed(modifier.socketSeed, generator);
    initSeed(modifier.qualitySeed, generator);
    initSeed(modifier.statSeed, generator);
    initSeed(modifier.nameSeed, generator);
    initSeed(modifier.displaySeed, generator);
    initSeed(modifier.spellSeed, generator);
    initSeed(modifier.statValueSeed, generator);
}

VirtualItemTemplate* VirtualItemMgr::GenerateVirtualTemplate(ItemTemplate const* base, VirtualModifier modifier)
{
    if (!base)
        return nullptr;

    // Generate a new virtual item template based on the base items template
    VirtualItemTemplate* output = new VirtualItemTemplate(base);

    // If no seed supplied, generate a new seed.
    // If seed supplied, skip and assign seed to template.
    if (modifier.seed == 0)
    {
        SFMTRand sfmt;
        output->seed = sfmt.RandomUInt32();
        modifier.seed = output->seed;
    }
    else
        output->seed = modifier.seed;

    InitSeedGen(modifier);
    output->seed = modifier.seed;
    output->displaySeed = modifier.displaySeed;
    output->nameSeed = modifier.nameSeed;
    output->socketSeed = modifier.socketSeed;
    output->spellSeed = modifier.spellSeed;
    output->statSeed = modifier.statSeed;
    output->statValueSeed = modifier.statValueSeed;

    GenerateQuality(output, modifier);

    // Generate base stats for the item.
    // Important that this is the first part to be generated after tempalte creation,
    // as some of the next function calls require information set in this function ie. quality, ilevel etc.
    GenerateStats(output, modifier);

    // Generate an item name based on type and quality
    GenerateItemName(output, modifier);

    // Set the correct disenchant ID based on ilevel and quality
    UpdateDisenchantId(output);

    // Generate the items sockets based on type and quality
    GenerateSockets(output, modifier);

    // Add spells to items like trinkets and legendaries(todo)
    GenerateSpells(output, modifier);

    // Generate primary and secondary stats.
    // Always do this last, as it has variable rand calls based on quality.
    GenerateItemStats(output, modifier);

    // Generate an entry based on item type
    WriteGuard guard(lock);
    EntryGenerator* entryGenerator = Generator(output);
    if (!entryGenerator)
        return nullptr;
    uint32 entry = entryGenerator->GenerateEntry(store);
    output->ItemId = entry;

    // Select a display ID for the item based on type, special case for trinkets and rings
    bool isTrinket = output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET;
    bool isRing = output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_FINGER;
    uint32 display = isTrinket || isRing ? 0 : GenerateItemDisplay(output, modifier);
    /*std::stringstream ss;
    ss << "Generated item with display " << display;
    sWorld->SendGlobalText(ss.str().c_str(), nullptr);*/
    if (display == 0)
        output->UpdateDisplay();
    else
        output->DisplayInfoID = display;

    delete store[entry];
    store[entry] = output;

    if(sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
        output->InitializeQueryData();

    return output;
}

void VirtualItemMgr::GenerateStats(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll) const
{
    std::mt19937 generator;
    generator.seed(modifier.statSeed);

    // always bind on pickup
    output->Bonding = BIND_WHEN_PICKED_UP;

    // decide itemlevel
    // if the modifier for ilevel is manually set (regenerating item as an example) then statically use this item level
    // if ilevel is not set, use the players average item level +/- 5 item levels.
    uint32 ilevel = output->ItemLevel;

    // If for whatever reason the players' average item level is less than 20, make sure to set it to 20.
    // To prevent too small of a stat pool on early items.
    if (modifier.plrAvgLvl < 20)
        modifier.plrAvgLvl = 20;

    // Get the average ilevels virtual level group
    int32 vLevel = GetVirtualLevel(float(modifier.plrAvgLvl));

    // Mod the virtual item level to allow higher or lower virtual item levels 
    vLevel = vLevel + irand(-1, 5, generator);
    // Add all vLvl mods before generating a new ilevel
    vLevel = vLevel + modifier.vLvlMod;

    // Get the new item level based on above modifier virtual level
    ilevel = round(GenerateItemLevel(vLevel));

    // Modify the returned, newly generated iLevel based on quality
    ilevel = ilevel + (int32(output->Quality) * 3);

    // One last mod to the ilevel to try to smooth out any ilevel groups and spikes
    ilevel = ilevel + irand(-3, 3, generator);

    // If not regenerating a item and item level has been set in the DB, cap ilevel at this amount
    if (modifier.isCrafted && !modifier.ilevel && ilevel >= output->ItemLevel)
    {
        ilevel = output->ItemLevel;
    }

    // If ilevel modifier is set, override all ilevel generation
    if (modifier.ilevel)
        ilevel = modifier.ilevel;

    // Hard cap of 325 across all items FIXME
    if (ilevel > 325)
    {
        ilevel = 325;
    }

    // decide armor, if item class is armor and not of type misc, armor should always be applied.
    if (output->Class == ITEM_CLASS_ARMOR && output->SubClass != ITEM_SUBCLASS_ARMOR_MISC)
    {
        // by default we assume 1 ilevel = 1 armor
        output->Armor = 1 * ilevel;

        // retrieve armor slot and type multiplier
        float typeslotmod = VirtualModifier::GetTypeSlotArmorModifier(output);
        output->Armor = output->Armor * typeslotmod;

        // depending on quality, add multiplier to armor piece
        float qmulti = ((int32(output->Quality) - int32(ITEM_QUALITY_NORMAL)) / 10.0f) + 1.0f;
        output->Armor = output->Armor * qmulti;

        // add a random 10% increase or decrease of stats
        float randmulti = (urand(90, 110, generator) / 100.0f);
        output->Armor = output->Armor * randmulti;
    }

    // Apply block rating to shields
    if (output->Class == ITEM_CLASS_ARMOR && output->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
    {
        output->Block = uint32(0.93f * float(ilevel));
    }

    // select stat group if the item is an armor piece or manually set as a modifier
    StatGroup statgroupid = modifier.statgroup;
    bool isCloak = output->Class == ITEM_CLASS_ARMOR &&
        output->SubClass == ITEM_SUBCLASS_ARMOR_CLOTH &&
        output->InventoryType == INVTYPE_CLOAK;
    StatGroup randone = StatGroup();
    StatGroup randtwo = StatGroup();
    if (output->Class == ITEM_CLASS_ARMOR && !isCloak)
    {
        std::vector<StatGroup> const& statgroups = modifier.premadeStatGroupData.GetArmorSubclassStatGroups(output);
        if (!statgroups.empty())
            randone = statgroups[urand(0, statgroups.size() - 1, generator)];
    }

    if (modifier.statgroup == STAT_GROUP_RANDOM)
        statgroupid = randone;

     randtwo = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 2, generator));
    // If the stat group is still random, select a random stat group.
     if (statgroupid == STAT_GROUP_RANDOM)
         statgroupid = randtwo;

    ASSERT(statgroupid < STAT_GROUP_COUNT); // must not be random anymore

    output->statGroup = statgroupid;

    // If item is a weapon, then generate bot and top damage + speed
    if (output->Class == ITEM_CLASS_WEAPON)
    {
        switch (output->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_SWORD2:
            case ITEM_SUBCLASS_WEAPON_AXE2:
            case ITEM_SUBCLASS_WEAPON_MACE2:
            {
                output->Delay = (urand(33, 37, generator) * 100);
                output->Damage[0].DamageMin = ((1.08f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.08f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_POLEARM:
            {
                output->Delay = (urand(31, 36, generator) * 100);
                output->Damage[0].DamageMin = ((1.08f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.08f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_STAFF:
            {
                output->Delay = (urand(20, 31, generator) * 100);
                output->Damage[0].DamageMin = ((1.08f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.08f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_AXE:
            case ITEM_SUBCLASS_WEAPON_MACE:
            case ITEM_SUBCLASS_WEAPON_SWORD:
            case ITEM_SUBCLASS_WEAPON_FIST:
            {
                output->Delay = (urand(15, 27, generator) * 100);
                output->Damage[0].DamageMin = ((0.83f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((0.83f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_DAGGER:
            {
                output->Delay = (urand(14, 19, generator) * 100);
                output->Damage[0].DamageMin = ((0.83f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((0.83f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_GUN:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            {
                output->Delay = (urand(27, 30, generator) * 100);
                output->Damage[0].DamageMin = ((1.2f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.2f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_WAND:
            {
                output->Delay = (urand(18, 22, generator) * 100);
                output->Damage[0].DamageMin = ((1.2f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.2f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = urand(SPELL_SCHOOL_FIRE, SPELL_SCHOOL_ARCANE, generator);
                break;
            }
            default:
                break;
        }

        // If weapon is a caster weapon, divide damage by 2, unless it's a wand!
        if ((statgroupid == STAT_GROUP_HEALING || statgroupid == STAT_GROUP_INT_DPS) && output->SubClass != ITEM_SUBCLASS_WEAPON_WAND)
        {
            output->Damage[0].DamageMin = output->Damage[0].DamageMin / 2.0f;
            output->Damage[0].DamageMax = output->Damage[0].DamageMax / 2.0f;
        }

        // Apply a damage bonus based on item quality
        float damageBonus = (((output->Quality - 2.0f) / 10.0f) / 2.0f) + 1.0f;
        output->Damage[0].DamageMin = output->Damage[0].DamageMin * damageBonus;
        output->Damage[0].DamageMax = output->Damage[0].DamageMax * damageBonus;
    }

    // TODO: add custom descriptions to legendaries possibly?
    // currently used to clean description of base template for crafting etc.
    output->Description = "";

    // apply other item data
    output->ItemLevel = ilevel;
    output->ItemSet = 0; // Temporary default to set 0, ie. no set. Need to add set handler based on stat groups.
    output->MaxDurability = 0; // Disable any form of durability for now
}

void VirtualItemMgr::GenerateItemStats(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll) const
{
    std::mt19937 generator;
    generator.seed(modifier.statValueSeed);

    // get statgroup id
    StatGroup statgroupid = output->statGroup;

    // decide stat amount
    uint32 statscount = output->Quality;
    if (statscount < 0)
        statscount = 0;

    ASSERT(statscount <= MAX_ITEM_PROTO_STATS);

    // add up to two extra stats per item
    uint32 statCountMod = urand(0, 2, generator);
    statscount = statscount + statCountMod;

    // Only a single stat on trinkets
    if (output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET)
    {
        statscount = 1;
    }

    // clear old stats
    for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
    {
        output->ItemStat[i].ItemStatType = 0;
        output->ItemStat[i].ItemStatValue = 0;
    }

    // select stat pool
    int16 pool = 0;
    if (modifier.statpool == -1)
        pool = output->ItemLevel;
    else
        pool = modifier.statpool;

    ASSERT(pool >= 0 && pool < 0x7FFF);

    // modify stat pool size depending on item quality
    pool = (pool * output->Quality) / 2;

    // since our stats are based on the ilevel of the item, regenerating with a new ilevel causes problems.
    // instead, we distribute the pool based on a static pool size, and use that as a percentage value
    // when distributing the actual stat values.
    int16 percentile_pool = 100;

    std::vector<ItemModType> const& primarystatgroup = modifier.premadeStatGroupData.GetStatGroupPrimaryStats(statgroupid, generator, modifier);
    std::vector<ItemModType> const& secondarystatgroup = modifier.premadeStatGroupData.GetStatGroupSecondaryStats(statgroupid, generator, modifier);

    std::vector<ItemModType> selectedStats;
    std::vector<int16> distributedPool;

    if (statscount && !primarystatgroup.empty() && !secondarystatgroup.empty())
    {
        // select stats from preselected stat group
        for (uint32 i = 0; i < statscount; ++i)
        {
            // make sure primary stats are always selected before secondary stats
            // trinkets should also only have secondary stats, not primary
            if (i < 2 && !(output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET))
                selectedStats.push_back(primarystatgroup[urand(0, primarystatgroup.size() - 1, generator)]);
            else
                selectedStats.push_back(secondarystatgroup[urand(0, secondarystatgroup.size() - 1, generator)]);
        };

        // distribute pool to stats
        const float mineachpct = 0.5f / selectedStats.size();
        ASSERT(mineachpct <= 1.0f / selectedStats.size() && mineachpct >= 0.0);

        // calculate min amount and take that from the randomly distributed pool
        int16 min_amount = std::floor(percentile_pool * mineachpct);
        int16 workpool = percentile_pool - selectedStats.size() * min_amount;

        // pick random positions from the workpool and use them to divide it into N random size parts
        // then add those to distributedPool along with the minimum amounts
        std::vector<int16> fences;
        fences.push_back(0);
        for (int32 i = 1; i < int32(selectedStats.size()); ++i)
            fences.push_back(urand(0, workpool, generator));
        fences.push_back(workpool);
        std::sort(fences.begin(), fences.end());
        for (int32 i = 1; i < int32(fences.size()); ++i)
            distributedPool.push_back(min_amount + fences[i] - fences[i - 1]);
    }

    // apply new stats
    distributedPool.resize(selectedStats.size()); // ensure counts match
    uint32 setStats = 0;
    for (size_t i = 0; i < std::min(selectedStats.size(), size_t(MAX_ITEM_PROTO_STATS)); ++i)
    {
        for (uint32 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
        {
            // if we are at a free stat slot or we are at a stat slot that has the same stat type
            if (j >= setStats || output->ItemStat[j].ItemStatType == selectedStats[i])
            {
                uint32 finalStatValue = std::floor((float(pool) * (float(distributedPool[i]) / 100.0f)) / VirtualModifier::GetStatRate(selectedStats[i]) * VirtualModifier::GetSlotStatModifier(output));
                output->ItemStat[j].ItemStatType = selectedStats[i];
                output->ItemStat[j].ItemStatValue += finalStatValue;
                setStats = std::max(setStats, uint32(j + 1));
                break;
            }
        }
    }

    output->StatsCount = setStats;
}

void VirtualItemMgr::GenerateVirtualLevelLookupArray()
{
    WriteGuard guard(lock);
    int32 i = 1;
    float iLevel = 0.0f;

    // Generate lookup table for Virtual Levels.
    for (int i = 0; iLevel < 325.0f; ++i)
    {
        iLevel = GenerateItemLevel(i);
        virtual_level_info.insert(std::make_pair(i, VirtualLevelInfo(iLevel)));
    }
}

float VirtualItemMgr::GenerateItemLevel(int32 virtualLevel) const
{
    // We need our x variable to be much lower than a managable number, so divide it
    float x = float(virtualLevel) / 5000.0f;

    // Generate a logarithmic value to be used as the correct ilevel for the provided vlevel
    float ilevel = ((pow((x + 0.0555f), 2) - 1.0f) / pow((x + 0.0555f), 2)) + 325.0f;

    return ilevel;
}

void VirtualItemMgr::GenerateItemName(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll) const
{
    std::mt19937 generator;
    generator.seed(modifier.nameSeed);

    std::string fullName = "";

    if (output->Class == ITEM_CLASS_ARMOR)
    {
        // Retrieve all the string lists
        // For Weapons we always use inventoryType 0
        std::map<uint32, std::vector<std::string>> nameLists;
        std::map<uint32, std::string> selectedWords;
        for (size_t i = 1; i <= 8; ++i)
        {
            NameInfo nameInfo(output->Class, output->SubClass, output->InventoryType, i);
            auto list = GetNamesForNameInfo(&nameInfo);

            // Make sure the current list is not empty. If it is, fall back to template item name.
            if (list.empty())
            {
                fullName = output->Name1;
                break;
            }

            nameLists.insert(std::make_pair(i, list));

            // Select a word from each list, as they are to be used across quality for regeneration.
            selectedWords.insert(std::make_pair(i, nameLists[i][urand(0, nameLists[i].size() - 1, generator)]));
        }

        // List 1: Unique names, like Malice, Mangler, Mercy etc.
        // List 2: Prefixes, like Arcane, Arched, Bloodied etc.
        // List 3: Material names, like Bone, Copper, Diamond etc.
        // List 4: Basic type name, like Blade, Razor, Maul etc.
        // List 5: Exotic type name, like Blade, Carver etc. Contains more exotic, but also the basic, type names.
        // List 6: Suffixes, like "of Agony", "of Bloodlust" etc.

        if (fullName != output->Name1)
        {
            // Concat the correct full item name for the item quality
            std::stringstream ss;

            // Shields have their names generated like weapons.
            if (output->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
            {
                switch (output->Quality)
                {
                case ITEM_QUALITY_NORMAL:
                {
                    ss << selectedWords[8] << " " << selectedWords[4];
                    break;
                }
                case ITEM_QUALITY_UNCOMMON:
                {
                    ss << selectedWords[3] << " " << selectedWords[4];
                    break;
                }
                case ITEM_QUALITY_RARE:
                {
                    ss << selectedWords[2] << " " << selectedWords[4];
                    break;
                }
                case ITEM_QUALITY_EPIC:
                {
                    ss << selectedWords[1];
                    break;
                }
                case ITEM_QUALITY_LEGENDARY:
                {
                    ss << selectedWords[1] << ", " << selectedWords[5] << " " << selectedWords[6];
                    break;
                }
                default:
                    ss << fullName;
                }
            }
            else
            {
                switch (output->Quality)
                {
                case ITEM_QUALITY_NORMAL:
                {
                    ss << selectedWords[8] << " " << selectedWords[5];
                    break;
                }
                case ITEM_QUALITY_UNCOMMON:
                {
                    ss << selectedWords[4] << " " << selectedWords[5];
                    break;
                }
                case ITEM_QUALITY_RARE:
                {
                    ss << selectedWords[5] << " of " << selectedWords[2];
                    break;
                }
                case ITEM_QUALITY_EPIC:
                {
                    ss << selectedWords[3] << " " << selectedWords[5];
                    break;
                }
                case ITEM_QUALITY_LEGENDARY:
                {
                    ss << selectedWords[1] << ", " << selectedWords[3] << " " << selectedWords[5] << " of " << selectedWords[7];
                    break;
                }
                default:
                    ss << fullName;
                }
            }
            fullName = ss.str();
        }
    }

    if (output->Class == ITEM_CLASS_WEAPON)
    {
        // Retrieve all the string lists
        // For Weapons we always use inventoryType 0
        std::map<uint32, std::vector<std::string>> nameLists;
        std::map<uint32, std::string> selectedWords;
        for (size_t i = 1; i <= 8; ++i)
        {
            NameInfo nameInfo(output->Class, output->SubClass, 0, i);
            auto list = GetNamesForNameInfo(&nameInfo);

            // Make sure the current list is not empty. If it is, fall back to template item name.
            if (list.empty())
            {
                fullName = output->Name1;
                break;
            }

            nameLists.insert(std::make_pair(i, list));

            // Select a word from each list, as they are to be used across quality for regeneration.
            selectedWords.insert(std::make_pair(i, nameLists[i][urand(0, nameLists[i].size() - 1, generator)]));
        }

        std::stringstream ss;
        // Concat the correct full item name for the item quality

        // List 1: Unique names, like Malice, Mangler, Mercy etc.
        // List 2: Prefixes, like Arcane, Arched, Bloodied etc.
        // List 3: Material names, like Bone, Copper, Diamond etc.
        // List 4: Basic type name, like Blade, Razor, Maul etc.
        // List 5: Exotic type name, like Blade, Carver etc. Contains more exotic, but also the basic, type names.
        // List 6: Suffixes, like "of Agony", "of Bloodlust" etc.
        if (fullName != output->Name1)
        {
            switch (output->Quality)
            {
            case ITEM_QUALITY_NORMAL:
            {
                ss << selectedWords[8] << " " << selectedWords[4];
                break;
            }
            case ITEM_QUALITY_UNCOMMON:
            {
                ss << selectedWords[3] << " " << selectedWords[4];
                break;
            }
            case ITEM_QUALITY_RARE:
            {
                ss << selectedWords[2] << " " << selectedWords[4];
                break;
            }
            case ITEM_QUALITY_EPIC:
            {
                ss << selectedWords[1];
                break;
            }
            case ITEM_QUALITY_LEGENDARY:
            {
                ss << selectedWords[1] << ", " << selectedWords[5] << " " << selectedWords[6];
                break;
            }
            default:
                ss << fullName;
            }
            fullName = ss.str();
        }
    }

    output->Name1 = fullName;
}

uint32 VirtualItemMgr::GenerateItemDisplay(VirtualItemTemplate* output, VirtualModifier modifier) const
{
    std::mt19937 generator;
    generator.seed(modifier.displaySeed);
    std::list<uint32> displayLists;
    displayLists = GetDisplaysForDisplayInfo(output);
    // Attempt to find display at a quality up if none found
    if (displayLists.empty())
    {
        displayLists = GetDisplaysForDisplayInfo(output, true);
    }
    // If still empty, output an error
    if (displayLists.empty())
    {
        std::ostringstream stream;
        stream << "ERROR: Found no display id for item quality [" << output->Quality << "] class [";
        stream << output->Class << "] subclass [" << output->SubClass << "] inventoryType [" << output->InventoryType << "]";
        stream << ". Please report this to developers.";
        sWorld->SendGlobalText(stream.str().c_str(), nullptr);
        return 0;
    }
    auto display = std::begin(displayLists);
    std::advance(display, urand(0, uint32(std::size(displayLists)) - 1, generator));
    return *display;
}

itemSpellInfo VirtualItemMgr::GenerateSpell(VirtualItemTemplate* output, VirtualModifier modifier)
{
    std::mt19937 generator;
    generator.seed(modifier.spellSeed);

#define IFSKIP(spellinfo, requirement) if (spellinfo != -1 && spellinfo != requirement) continue
    std::list<itemSpellInfo> spells;
    for (auto const &someSpells : availableSpells)
    {
        if (output->Quality != someSpells.quality)
            continue;
        IFSKIP(someSpells.itemClass, output->Class);
        IFSKIP(someSpells.subClass, output->SubClass);
        IFSKIP(someSpells.inventoryType, output->InventoryType);
        IFSKIP(someSpells.statGroup, output->statGroup);
        if (someSpells.maxItemLevel != -1 && output->ItemLevel > uint32(someSpells.maxItemLevel))
            continue;
        if (someSpells.minItemLevel != -1 && output->ItemLevel < uint32(someSpells.minItemLevel))
            continue;
        spells.push_back(someSpells);
    }
    if (spells.empty())
        return itemSpellInfo();
    auto selectedSpell = std::begin(spells);
    std::advance(selectedSpell, urand(0, uint32(std::size(spells)) - 1, generator));
#undef IFSKIP
    return *selectedSpell;
}

void VirtualItemMgr::GenerateSpells(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll)
{
    std::mt19937 generator;
    generator.seed(modifier.spellSeed);

    //@todo Finalize these numbers, add more then 1 spell to generate.
    bool isTrinket = output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET;
    uint8 numSpellsToGenerate = isTrinket ? 1 : 0;

    // FIXME twinkets should always generate a single spell, and the rest are from stat pool
    // This will need to be refactored to support other items
    /*switch (output->Quality)
    {
    case ITEM_QUALITY_UNCOMMON:
        {
            float chance = isTrinket ? 60.f : 10.f;
            if (roll_chance_f(chance))
                numSpellsToGenerate += 1;
        }break;
    case ITEM_QUALITY_RARE:
    {
        float chance = isTrinket ? 70.f : 15.f;
        if (roll_chance_f(chance))
            numSpellsToGenerate += 2;
        else
            numSpellsToGenerate += 1;
    }break;
    case ITEM_QUALITY_EPIC:
    {
        float chance = isTrinket ? 80.f : 20.f;
        if (roll_chance_f(chance))
            numSpellsToGenerate += 3;
        else
            numSpellsToGenerate += 2;
    }break;
    case ITEM_QUALITY_LEGENDARY:
    {
        float chance = isTrinket ? 90.f : 25.f;
        if (roll_chance_f(chance))
            numSpellsToGenerate += 4;
        else
            numSpellsToGenerate += 3;
    }break;
    default:
        break;
    }*/

    //Prevent crash incase something goes dumb.
    if (numSpellsToGenerate > MAX_ITEM_PROTO_SPELLS)
        numSpellsToGenerate = MAX_ITEM_PROTO_SPELLS;

    std::vector<uint32_t> spellsToUse;
    for (uint8 i = 0; i < numSpellsToGenerate; ++i)
    {
        itemSpellInfo spell = GenerateSpell(output, modifier);
        if (spell.spellId == 0)
            continue;
        // Skip spell if we have already used this one. Try a few times to fetch a unique spell
        int tries = 0;
        while (tries < 3 && std::find(spellsToUse.begin(), spellsToUse.end(), spell.spellId) != spellsToUse.end())
        {
            spell = GenerateSpell(output, modifier);
            ++tries;
        }
        // If still a duplicate, skip
        if (std::find(spellsToUse.begin(), spellsToUse.end(), spell.spellId) != spellsToUse.end())
            continue;
        spellsToUse.push_back(spell.spellId);
        output->Spells[i].SpellId = spell.spellId;
        output->Spells[i].SpellTrigger = spell.SpellTrigger;
        output->Spells[i].SpellCharges = spell.SpellCharges;
        output->Spells[i].SpellPPMRate = spell.SpellPPMRate;
        output->Spells[i].SpellCooldown = spell.SpellCooldown;
        output->Spells[i].SpellCategory = spell.SpellCategory;
        output->Spells[i].SpellCategoryCooldown = spell.SpellCategoryCooldown;
    }
}

void VirtualItemMgr::GenerateSockets(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll)
{
    std::mt19937 generator;
    generator.seed(modifier.socketSeed);

    // set amount of sockets on the items depending on the quality
    int32 socketCount = 0;
    uint32 socketMod = urand(0, 1, generator);

    switch (output->Quality) {
    case ITEM_QUALITY_LEGENDARY:
        socketCount = 2 + socketMod;
        break;
    case ITEM_QUALITY_EPIC:
        socketCount = 2;
        break;
    case ITEM_QUALITY_RARE:
        socketCount = 1 + socketMod;
        break;
    default:
        socketCount = 1;
        break;
    }

    // reduce max amount of sockets depending on type
    if (output->Class == ITEM_CLASS_ARMOR)
    {
        switch (output->InventoryType)
        {
        case INVTYPE_LEGS:
        case INVTYPE_CHEST:
            break;
        case INVTYPE_HEAD:
        case INVTYPE_SHOULDERS:
            if (socketCount > 2)
                socketCount = 2;
            break;
        case INVTYPE_WAIST:
        case INVTYPE_CLOAK:
        case INVTYPE_FEET:
        case INVTYPE_WRISTS:
        case INVTYPE_HANDS:
            if (socketCount > 1)
                socketCount = 1;
            break;
        default:
            socketCount = 0;
        }
    }

    // set socket colors
    std::vector<SocketColor> const& socketcolors = VirtualModifier().premadeStatGroupData.GetStatGroupSockets(reRoll ? STAT_GROUP_ALL : output->statGroup, generator, VirtualModifier());
    if (!socketcolors.empty())
    {
        for (int32 i = 0; i < socketCount; ++i)
        {
            if (output->Socket[i].Color != 0 && !reRoll)
                continue;

            /*float chance = output->Quality == ITEM_QUALITY_LEGENDARY ? 10.f : 5.f;
            if (roll_chance_f(chance))
            {
                output->Socket[i].Color = SOCKET_COLOR_PRISMATIC;
                continue;
            }*/

            // FIXME - Regeneration:
            // We should urand the socket color, but with variable amounts of sockets, we have variable amounts of urand calls.
            // This causes regeneration/upgrading to fail.
            // Should be fixed with specific mods instead of full item regeneration.

            // output->Socket[i].Color = socketcolors[urand(0, socketcolors.size() - 1, generator)];
            output->Socket[i].Color = socketcolors[urand(0, socketcolors.size() - 1, generator)];
        }
    }
}

void VirtualItemMgr::GenerateQuality(VirtualItemTemplate* output, VirtualModifier modifier, bool reRoll)
{
    std::mt19937 generator;
    generator.seed(modifier.qualitySeed);

    // decide quality
    uint32 quality = output->Quality;

    // these are not percentage chances. They represent areas of a number line made from their sum
    static const uint32 chances[MAX_ITEM_QUALITY] = {
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_POOR),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_COMMON),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_UNCOMMON),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_RARE),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_EPIC),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_LEGENDARY),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_ARTIFACT),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_HEIRLOOM),
    };

    uint32 sum = 0;
    for (auto u : chances)
        sum += u;

    if (sum >= 1)
    {
        uint32 rand = urand(1, sum, generator);
        sum = 0;
        for (size_t i = 0; i < MAX_ITEM_QUALITY; ++i)
        {
            sum += chances[i];
            if (sum < rand)
                continue;

            quality = i;
            break;
        }
    }

    quality = std::max(output->Quality, quality); // dont generate quality below original

    // If the quality modifier is set, discard generated quality and force specific quality
    if (modifier.quality < MAX_ITEM_QUALITY)
        quality = modifier.quality;

    output->Quality = quality;
}

void VirtualItemMgr::GenerateAdditonalStat(VirtualItemTemplate* output)
{
    uint8 stats [21] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_SPIRIT,
        ITEM_MOD_STRENGTH,
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_SPELL_RATING,
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_MANA_REGENERATION,
        ITEM_MOD_SPELL_POWER,
        //ITEM_MOD_SPELL_PENETRATION,
        ITEM_MOD_HIT_RANGED_RATING,
        ITEM_MOD_CRIT_RANGED_RATING,
        ITEM_MOD_HASTE_RANGED_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_RANGED_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
}


uint32 VirtualItemMgr::EntryGenerator::GenerateEntry(VirtualItemMgr::Store const& store)
{
    uint32 entry = nextEntry;
    for (uint32 i = nextEntry + 1; true; ++i)
    {
        if (i >= maxEntry)
            i = minEntry;
        if (i == nextEntry)
            break;
        if (store.find(i) == store.end())
        {
            nextEntry = i;
            break;
        }
    }

    ASSERT(entry != nextEntry);
    ASSERT(entry >= VirtualItemMgr::minEntry);
    ASSERT(entry <= VirtualItemMgr::maxEntry);
    return entry;
}

uint32 VirtualItemMgr::EntryGenerator::PeekNext() const
{
    return nextEntry;
}

VirtualItemMgr::EntryGenerator::EntryGenerator() : minEntry(0), maxEntry(1)
{
    ASSERT(minEntry < maxEntry);
    nextEntry = minEntry;
}

VirtualItemMgr::EntryGenerator::EntryGenerator(uint32 min, uint32 max) : minEntry(min), maxEntry(max)
{
    ASSERT(minEntry < maxEntry);
    nextEntry = minEntry;
}

VirtualItemMgr::EntryGenerator* VirtualItemMgr::Generator(ItemTemplate* temp)
{
    if (temp->Class == ITEM_CLASS_ARMOR)
    {
        auto it = armorGenerator.find((InventoryType)temp->InventoryType);
        if (it != armorGenerator.end())
            return &it->second;
    }
    if (temp->Class == ITEM_CLASS_WEAPON)
    {
        auto it = weaponGenerator.find((ItemSubclassWeapon)temp->SubClass);
        if (it != weaponGenerator.end())
            return &it->second;
    }
    return nullptr;
}

bool VirtualItemMgr::InsertEntry(VirtualItemTemplate* virtualItem)
{
    if (!virtualItem)
        return false;
    EntryGenerator* generator = Generator(virtualItem);
    if (!generator)
        return false;

    uint32 entry = virtualItem->ItemId;
    if (entry < minEntry || entry >= maxEntry || store.find(entry) != store.end())
        return false;
    store[entry] = virtualItem;

    if (entry == generator->PeekNext())
        generator->GenerateEntry(store);
    return true;
}

// Getters

int32 VirtualItemMgr::GetVirtualLevel(float ilevel) const
{
    int32 vLevel = 1;

    for (auto i=virtual_level_info.begin(); i!=virtual_level_info.end(); i++)
    {
        float diff = ilevel / i->second.iLevel;
        if (diff < 1.0f)
        {
            vLevel = i->first;
            break;
        }
    }

    return vLevel;
}

std::list<uint32> VirtualItemMgr::GetDisplaysForDisplayInfo(VirtualItemTemplate* output, bool qualityOverride) const
{
    std::list<uint32> displays;

    uint32 quality = output->Quality;
    if (qualityOverride)
        quality = output->Quality + 1;

    if (quality == ITEM_QUALITY_LEGENDARY)
        quality = quality - 1;

    for (auto displaysitr : availableDisplays)
    {
        if (quality != displaysitr.quality)
            continue;

        if (output->InventoryType != displaysitr.iInventoryType)
            continue;

        if (output->Class != displaysitr.iClass)
            continue;

        if (output->SubClass != displaysitr.isubClass)
            continue;

        displays.push_back(displaysitr.displayId);
    }
    return displays;
}

std::vector<std::string> VirtualItemMgr::GetNamesForNameInfo(NameInfo* info) const
{
    if (!info)
        return std::vector<std::string>();

    std::vector<std::string> names;
    for (auto name : availableNames)
    {
        if (info->array_id != name.array_id && name.array_id != -1)
            continue;

        if (info->itemType != name.itemType && name.itemType != -1)
            continue;

        if (info->subclass != name.subclass && name.subclass != -1)
            continue;

        if (info->inventoryType != name.inventoryType && name.inventoryType != -1)
            continue;

        names.push_back(name.name);
    }
    return names;
}

std::vector<ItemModType> const & VirtualModifier::StatGroupData::GetStatGroupPrimaryStats(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));
    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_primary_stats[group];
}

std::vector<ItemModType> const& VirtualModifier::StatGroupData::GetStatGroupSecondaryStats(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));
    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_secondary_stats[group];
}

std::vector<SocketColor> const & VirtualModifier::StatGroupData::GetStatGroupSockets(StatGroup group, std::mt19937& generator, VirtualModifier modifier) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));
    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_sockets[group];
}

std::vector<StatGroup> const & VirtualModifier::StatGroupData::GetArmorSubclassStatGroups(VirtualItemTemplate* output) const
{
    return armor_type_stat_groups[output->SubClass];
}

float VirtualModifier::GetSlotStatModifier(VirtualItemTemplate* output)
{
    switch (output->InventoryType)
    {
    case INVTYPE_HEAD:
    case INVTYPE_CHEST:
    case INVTYPE_ROBE:
    case INVTYPE_LEGS:
    case INVTYPE_2HWEAPON:
        return 1.0f;

    case INVTYPE_SHOULDERS:
    case INVTYPE_HANDS:
    case INVTYPE_WAIST:
    case INVTYPE_FEET:
        return 0.75f;

    case INVTYPE_WRISTS:
    case INVTYPE_NECK:
    case INVTYPE_CLOAK:
    case INVTYPE_HOLDABLE:
    case INVTYPE_SHIELD:
        return 0.56f;

    // Trinkets are extremely nerfed since they roll a single stat with a special effect
    case INVTYPE_TRINKET:
        return 0.2f;

    case INVTYPE_WEAPON:
    case INVTYPE_WEAPONMAINHAND:
    case INVTYPE_WEAPONOFFHAND:
    case INVTYPE_FINGER:
        return 0.42f;

    case INVTYPE_RANGED:
    case INVTYPE_RANGEDRIGHT:
    case INVTYPE_THROWN:
        return 0.31f;

    default:
        return 1.0f;
    }
    return 1.0f;
}

float VirtualModifier::GetStatRate(ItemModType stat)
{
    switch (stat)
    {
    case ITEM_MOD_RANGED_ATTACK_POWER:
        return 0.4f;
    case ITEM_MOD_ARMOR_PENETRATION_RATING:
        return 0.4f;
    case ITEM_MOD_ATTACK_POWER:
        return 0.5f;
    case ITEM_MOD_SPELL_HEALING_DONE:
        return 0.45f;
    case ITEM_MOD_MANA_REGENERATION:
        return 2.5f;
    case ITEM_MOD_HEALTH_REGEN:
        return 2.5f;
    case ITEM_MOD_SPELL_PENETRATION:
        return 0.8f;
    case ITEM_MOD_BLOCK_VALUE:
        return 0.65f;
    case ITEM_MOD_SPELL_POWER:
        return 0.86f;
    case ITEM_MOD_DEFENSE_SKILL_RATING:
        return 1.2f;
    case ITEM_MOD_BLOCK_RATING:
        return 1.2f;
    case ITEM_MOD_INTELLECT:
        return 1.9f;
    case ITEM_MOD_SPIRIT:
        return 1.65f;
    default:
        return 1.0f;
    }
    return 1.0f;
}

float VirtualModifier::GetTypeSlotArmorModifier(VirtualItemTemplate* output)
{
    switch (output->SubClass)
    {
    case ITEM_SUBCLASS_ARMOR_CLOTH:
        switch (output->InventoryType)
        {
        case INVTYPE_HEAD:
            return 1.05f;
        case INVTYPE_SHOULDERS:
            return 0.97f;
        case INVTYPE_CHEST:
            return 1.28f;
        case INVTYPE_WAIST:
            return 0.73f;
        case INVTYPE_LEGS:
            return 1.13f;
        case INVTYPE_FEET:
            return 0.9f;
        case INVTYPE_WRISTS:
            return 0.56f;
        case INVTYPE_HANDS:
            return 0.80f;
        case INVTYPE_CLOAK:
            return 0.66f;
        default:
            return 1.0f;
        }
    case ITEM_SUBCLASS_ARMOR_LEATHER:
        switch (output->InventoryType)
        {
        case INVTYPE_HEAD:
            return 2.13f;
        case INVTYPE_SHOULDERS:
            return 2.0f;
        case INVTYPE_CHEST:
            return 2.63f;
        case INVTYPE_WAIST:
            return 1.95f;
        case INVTYPE_LEGS:
            return 2.32f;
        case INVTYPE_FEET:
            return 1.82f;
        case INVTYPE_WRISTS:
            return 1.17f;
        case INVTYPE_HANDS:
            return 1.66f;
        default:
            return 1.0f;
        }
    case ITEM_SUBCLASS_ARMOR_MAIL:
        switch (output->InventoryType)
        {
        case INVTYPE_HEAD:
            return 4.35f;
        case INVTYPE_SHOULDERS:
            return 4.0f;
        case INVTYPE_CHEST:
            return 5.34f;
        case INVTYPE_WAIST:
            return 3.0f;
        case INVTYPE_LEGS:
            return 4.68f;
        case INVTYPE_FEET:
            return 3.69f;
        case INVTYPE_WRISTS:
            return 2.35f;
        case INVTYPE_HANDS:
            return 3.34f;
        default:
            return 1.0f;
        }
    case ITEM_SUBCLASS_ARMOR_PLATE:
        switch (output->InventoryType)
        {
        case INVTYPE_HEAD:
            return 5.38f;
        case INVTYPE_SHOULDERS:
            return 5.28f;
        case INVTYPE_CHEST:
            return 8.48f;
        case INVTYPE_WAIST:
            return 3.73f;
        case INVTYPE_LEGS:
            return 6.58f;
        case INVTYPE_FEET:
            return 4.55f;
        case INVTYPE_WRISTS:
            return 2.9f;
        case INVTYPE_HANDS:
            return 4.15f;
        default:
            return 1.0f;
        }
    case ITEM_SUBCLASS_ARMOR_SHIELD:
        return 18.2f;
    default:
        return 1.0f;
    }
    return 1.0f;
}

VirtualItemTemplate* VirtualItemMgr::GetVirtualTemplate(uint32 entry)
{
    if (entry < minEntry || entry >= maxEntry)
        return nullptr;
    ReadGuard guard(lock);
    auto it = store.find(entry);
    if (it != store.end())
        return it->second;
    return nullptr;
}

// Others

void VirtualItemMgr::UpdateDisenchantId(VirtualItemTemplate* output)
{
    uint32 ilevel = output->ItemLevel;
    uint32 quality = output->Quality;
    // Different disenchant loot pools depending on ilevel and quality
    // Range 60000-60029
    if (ilevel <= 50)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60000;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60001;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60002;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60003;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60004;
            break;
        }
    }
    else if (ilevel > 50 && ilevel <= 100)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60005;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60006;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60007;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60008;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60009;
            break;
        }
    }
    else if (ilevel > 100 && ilevel <= 150)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60010;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60011;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60012;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60013;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60014;
            break;
        }
    }
    else if (ilevel > 150 && ilevel <= 200)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60015;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60016;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60017;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60018;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60019;
            break;
        }
    }
    else if (ilevel > 200 && ilevel <= 250)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60020;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60021;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60022;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60023;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60024;
            break;
        }
    }
    else if (ilevel > 250)
    {
        switch (quality) {
        case ITEM_QUALITY_LEGENDARY:
            output->DisenchantID = 60025;
            break;
        case ITEM_QUALITY_EPIC:
            output->DisenchantID = 60026;
            break;
        case ITEM_QUALITY_RARE:
            output->DisenchantID = 60027;
            break;
        case ITEM_QUALITY_UNCOMMON:
            output->DisenchantID = 60028;
            break;
        case ITEM_QUALITY_NORMAL:
            output->DisenchantID = 60029;
            break;
        }
    }
}

bool VirtualItemMgr::IsVirtualTemplate(ItemTemplate const* base)
{
    if (!((base->FlagsCu & ITEM_FLAGS_CU_VIRTUAL_ITEM_BASE) != 0 &&
        (base->Class == ITEM_CLASS_WEAPON || base->Class == ITEM_CLASS_ARMOR) &&
        base->RandomProperty == 0 && base->RandomSuffix == 0 &&
        base->ScalingStatDistribution == 0 && base->ScalingStatValue == 0))
        return false;

    if (base->Class == ITEM_CLASS_ARMOR)
    {
        for (auto InventoryType : armorInventoryType)
            if (base->InventoryType == InventoryType)
                return true;
    }

    if (base->Class == ITEM_CLASS_WEAPON)
    {
        for (auto subclass : weaponSubclasses)
            if (base->SubClass == subclass)
                return true;
    }
    return false;
}

void VirtualItemTemplate::UpdateDisplay()
{
    // Get the correct display ID
    if (ItemEntry const* dbcitem = sItemStore.LookupEntry(ItemId))
        DisplayInfoID = dbcitem->DisplayId;
}

// Data

VirtualModifier::StatGroupData::StatGroupData()
{
    // Healing Data
    stat_group_primary_stats[STAT_GROUP_HEALING] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_SPIRIT
    };
    stat_group_secondary_stats[STAT_GROUP_HEALING] = {
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_MANA_REGENERATION,
        ITEM_MOD_SPELL_POWER
    };
    stat_group_sockets[STAT_GROUP_HEALING] = {
        SOCKET_COLOR_BLUE
    };

    // Int DPS Data
    stat_group_primary_stats[STAT_GROUP_INT_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_INTELLECT
    };
    stat_group_secondary_stats[STAT_GROUP_INT_DPS] = {
        ITEM_MOD_HIT_SPELL_RATING,
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_SPELL_POWER
        //ITEM_MOD_SPELL_PENETRATION
    };
    stat_group_sockets[STAT_GROUP_INT_DPS] = {
        SOCKET_COLOR_BLUE
    };

    // Str DPS Data
    stat_group_primary_stats[STAT_GROUP_STR_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_STRENGTH
    };
    stat_group_secondary_stats[STAT_GROUP_STR_DPS] = {
        ITEM_MOD_HIT_MELEE_RATING,
        ITEM_MOD_CRIT_MELEE_RATING,
        ITEM_MOD_HASTE_MELEE_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_sockets[STAT_GROUP_STR_DPS] = {
        SOCKET_COLOR_RED
    };

    // Str Tank Data
    stat_group_primary_stats[STAT_GROUP_STR_TANK] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_STRENGTH
    };
    stat_group_secondary_stats[STAT_GROUP_STR_TANK] = {
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_RATING,
        ITEM_MOD_BLOCK_RATING,
        ITEM_MOD_BLOCK_VALUE
    };
    stat_group_sockets[STAT_GROUP_STR_TANK] = {
        SOCKET_COLOR_RED
    };

    // Agi Melee DPS Data
    stat_group_primary_stats[STAT_GROUP_AGI_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY
    };
    stat_group_secondary_stats[STAT_GROUP_AGI_DPS] = {
        ITEM_MOD_HIT_MELEE_RATING,
        ITEM_MOD_CRIT_MELEE_RATING,
        ITEM_MOD_HASTE_MELEE_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_sockets[STAT_GROUP_AGI_DPS] = {
        SOCKET_COLOR_YELLOW
    };

    // Agi Tank Data
    stat_group_primary_stats[STAT_GROUP_AGI_TANK] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY
    };
    stat_group_secondary_stats[STAT_GROUP_AGI_TANK] = {
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_RATING,
        ITEM_MOD_BLOCK_RATING,
        ITEM_MOD_BLOCK_VALUE
    };
    stat_group_sockets[STAT_GROUP_AGI_TANK] = {
        SOCKET_COLOR_YELLOW
    };

    // Agi Ranged DPS Data
    stat_group_primary_stats[STAT_GROUP_AGI_RANGED] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_INTELLECT
    };
    stat_group_secondary_stats[STAT_GROUP_AGI_RANGED] = {
        ITEM_MOD_HIT_RANGED_RATING,
        ITEM_MOD_CRIT_RANGED_RATING,
        ITEM_MOD_HASTE_RANGED_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_RANGED_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_sockets[STAT_GROUP_AGI_RANGED] = {
        SOCKET_COLOR_YELLOW
    };

    // Stat group for all stats
    stat_group_primary_stats[STAT_GROUP_ALL] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_SPIRIT,
        ITEM_MOD_STRENGTH
    };
    stat_group_secondary_stats[STAT_GROUP_ALL] = {
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_SPELL_RATING,
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_MANA_REGENERATION,
        ITEM_MOD_SPELL_POWER,
        //ITEM_MOD_SPELL_PENETRATION,
        ITEM_MOD_HIT_RANGED_RATING,
        ITEM_MOD_CRIT_RANGED_RATING,
        ITEM_MOD_HASTE_RANGED_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_RANGED_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_sockets[STAT_GROUP_ALL] = {
        SOCKET_COLOR_YELLOW,
        SOCKET_COLOR_RED,
        SOCKET_COLOR_BLUE
    };

    // type stat groups
    armor_type_stat_groups[ITEM_SUBCLASS_ARMOR_CLOTH] = {
        STAT_GROUP_HEALING,
        STAT_GROUP_INT_DPS
    };

    armor_type_stat_groups[ITEM_SUBCLASS_ARMOR_LEATHER] = {
        STAT_GROUP_HEALING,
        STAT_GROUP_INT_DPS,
        STAT_GROUP_AGI_DPS,
        STAT_GROUP_AGI_TANK,
        STAT_GROUP_AGI_RANGED
    };

    armor_type_stat_groups[ITEM_SUBCLASS_ARMOR_MAIL] = {
        STAT_GROUP_HEALING,
        STAT_GROUP_STR_DPS,
        STAT_GROUP_STR_TANK,
        STAT_GROUP_AGI_DPS,
        STAT_GROUP_AGI_RANGED
    };

    armor_type_stat_groups[ITEM_SUBCLASS_ARMOR_PLATE] = {
        STAT_GROUP_HEALING,
        STAT_GROUP_INT_DPS,
        STAT_GROUP_STR_DPS,
        STAT_GROUP_STR_TANK
    };
}

VirtualModifier::StatGroupData const VirtualModifier::premadeStatGroupData;

const std::vector<InventoryType> VirtualItemMgr::armorInventoryType = {
    //INVTYPE_NON_EQUIP,
    INVTYPE_HEAD,
    INVTYPE_NECK,
    INVTYPE_SHOULDERS,
    //INVTYPE_BODY,
    INVTYPE_CHEST,
    INVTYPE_WAIST,
    INVTYPE_LEGS,
    INVTYPE_FEET,
    INVTYPE_WRISTS,
    INVTYPE_HANDS,
    INVTYPE_FINGER,
    INVTYPE_TRINKET,
    //INVTYPE_WEAPON,
    INVTYPE_SHIELD,
    //INVTYPE_RANGED,
    INVTYPE_CLOAK,
    //INVTYPE_2HWEAPON,
    //INVTYPE_BAG,
    //INVTYPE_TABARD,
    //INVTYPE_ROBE,
    //INVTYPE_WEAPONMAINHAND,
    //INVTYPE_WEAPONOFFHAND,
    //INVTYPE_HOLDABLE,
    //INVTYPE_AMMO,
    //INVTYPE_THROWN,
    //INVTYPE_RANGEDRIGHT,
    //INVTYPE_QUIVER,
    //INVTYPE_RELIC
};
const std::vector<ItemSubclassWeapon> VirtualItemMgr::weaponSubclasses = {
    ITEM_SUBCLASS_WEAPON_AXE,
    ITEM_SUBCLASS_WEAPON_AXE2,
    ITEM_SUBCLASS_WEAPON_BOW,
    ITEM_SUBCLASS_WEAPON_GUN,
    ITEM_SUBCLASS_WEAPON_MACE,
    ITEM_SUBCLASS_WEAPON_MACE2,
    ITEM_SUBCLASS_WEAPON_POLEARM,
    ITEM_SUBCLASS_WEAPON_SWORD,
    ITEM_SUBCLASS_WEAPON_SWORD2,
    //ITEM_SUBCLASS_WEAPON_obsolete,
    ITEM_SUBCLASS_WEAPON_STAFF,
    //ITEM_SUBCLASS_WEAPON_EXOTIC,
    //ITEM_SUBCLASS_WEAPON_EXOTIC2,
    ITEM_SUBCLASS_WEAPON_FIST,
    //ITEM_SUBCLASS_WEAPON_MISC,
    ITEM_SUBCLASS_WEAPON_DAGGER,
    //ITEM_SUBCLASS_WEAPON_THROWN,
    //ITEM_SUBCLASS_WEAPON_SPEAR,
    ITEM_SUBCLASS_WEAPON_CROSSBOW,
    ITEM_SUBCLASS_WEAPON_WAND,
    //ITEM_SUBCLASS_WEAPON_FISHING_POLE,
};
