#include "VirtualItemMgr.h"
#include "Containers.h"
#include "Errors.h" // ASSERT macro
#include "SharedDefines.h" // item quality enum
#include "World.h" // config values
#include <algorithm>
#include <cstdlib>
#include "Random.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "SFMTRand.h"

#define SelectSkip(a,b) if( a >= 0 && a != b) continue
#define SelectMinMaxSkip(min,max,value) if((min >= 0 && min > value) || (max >= 0 && max < value )) continue

bool SelectSkipDebug(int32 a, int32 b, std::string label)
{
    if (a >= 0)
    {
        if (a == b)
            return false;
    }
    else
        return false;
    return true;
}

VirtualItemMgr& VirtualItemMgr::instance()
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
        uint32 min = minEntry + (blockCounter * block);
        uint32 max = min + block - 1;
        armorGenerator[InventoryType] = EntryGenerator(min, min + block - 1);
        ++blockCounter;
        TC_LOG_INFO("server.loading", "ItemGenerator Inventory[%d] %d - %d", static_cast<int>(InventoryType), min, max);
    }

    for (auto subclass : weaponSubclasses)
    {
        uint32 min = minEntry + (blockCounter * block);
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

    do {
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
        uint32 minQuality = fields[1].GetUInt32();
        uint32 maxQuality = fields[2].GetUInt32();
        int32 itemClass = fields[3].GetInt32();
        int32 subClass = fields[4].GetInt32();
        int32 inventoryType = fields[5].GetInt32();
        int8 statGroup = fields[6].GetInt8();
        int32 minItemLevel = fields[7].GetInt32();
        int32 maxItemLevel = fields[8].GetInt32();
        uint32 SpellTrigger = fields[9].GetUInt32();
        int32  SpellCharges = fields[10].GetInt32();
        float  SpellPPMRate = fields[11].GetFloat();
        int32  SpellCooldown = fields[12].GetInt32();
        uint32 SpellCategory = fields[13].GetUInt32();
        int32  SpellCategoryCooldown = fields[14].GetInt32();

        availableSpells.push_back(itemSpellInfo(spellId, minQuality, maxQuality, itemClass, subClass, inventoryType, statGroup, minItemLevel, maxItemLevel, SpellTrigger, SpellCharges, SpellPPMRate, SpellCooldown, SpellCategory, SpellCategoryCooldown));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item spells in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

void VirtualItemMgr::LoadSetsFromDB()
{
    WriteGuard guard(lock);

    uint32 count = 0;
    uint32 beginTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_itemsets`");

    if (!result)
    {
        TC_LOG_INFO("server.loading", "Loaded 0 available virtual item sets, table item_generator_itemsets is empty.");
        return;
    }

    do {
        Field* fields = result->Fetch();
        uint32 setId = fields[0].GetUInt32();
        int32 itemClass = fields[1].GetInt32();
        int32 subClass = fields[2].GetInt32();
        int32 inventoryType = fields[3].GetInt32();
        int8 statGroup = fields[4].GetInt8();
        int32 minItemLevel = fields[5].GetInt32();
        int32 maxItemLevel = fields[6].GetInt32();
        int32 minQuality = fields[7].GetInt32();
        int32 maxQuality = fields[8].GetInt32();
        uint32 displayOverride = fields[9].GetUInt32();
        std::string flavorOverride = fields[10].GetString();

        availableItemSets.push_back(itemSetInfo(setId, itemClass, subClass, inventoryType, statGroup, minItemLevel, maxItemLevel, minQuality, maxQuality, displayOverride, flavorOverride));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item sets in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

// Generators

void VirtualItemMgr::RegenerateItemInfo(VirtualItemTemplate* output, VirtualModifier modifier)
{
    GenerateQuality(output, modifier);
    GenerateStatGroup(output, modifier);
    GenerateLegendaryItemEffect(output, modifier);
    GenerateBaseStats(output, modifier);
    GenerateItemName(output, modifier);
    //UpdateDisenchantId(output);
    UpdateDisenchantIdNew(output);
    GenerateSockets(output, modifier);
    //GenerateSpells(output, modifier);
    GenerateItemStats(output, modifier);
    //GenerateItemSet(output, modifier); should we ever regenerate selected sets?
    GenerateItemDisplay(output, modifier);
}

void initSeed(uint32& val, std::mt19937 generator)
{
    if (!val)
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
    initSeed(modifier.statGroupSeed, generator);
    initSeed(modifier.setSeed, generator);
    initSeed(modifier.legendarySeed, generator);
}

VirtualItemTemplate* VirtualItemMgr::GenerateVirtualTemplate(ItemTemplate const* base, VirtualModifier& modifier)
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
    output->qualitySeed = modifier.qualitySeed;
    output->displaySeed = modifier.displaySeed;
    output->nameSeed = modifier.nameSeed;
    output->socketSeed = modifier.socketSeed;
    output->spellSeed = modifier.spellSeed;
    output->statSeed = modifier.statSeed;
    output->statValueSeed = modifier.statValueSeed;
    output->statGroupSeed = modifier.statGroupSeed;
    output->setSeed = modifier.setSeed;
    output->legendarySeed = modifier.legendarySeed;

    GenerateQuality(output, modifier);

    // Select a stat group for the item
    GenerateStatGroup(output, modifier);

    // Generate Legendary (if item is legendary)
    GenerateLegendaryItemEffect(output, modifier);

    // Generate base stats for the item.
    GenerateBaseStats(output, modifier);

    // Generate an item name based on type and quality
    GenerateItemName(output, modifier);

    // Set the correct disenchant ID based on ilevel and quality
    //UpdateDisenchantId(output);
    UpdateDisenchantIdNew(output);

    // Generate the items sockets based on type and quality
    GenerateSockets(output, modifier);

    // Add spells to items like trinkets
    GenerateSpells(output, modifier);

    // Generate primary and secondary stats.
    GenerateItemStats(output, modifier);

    // Generate item set
    GenerateItemSet(output, modifier);

    // Generate an entry based on item type
    WriteGuard guard(lock);
    EntryGenerator* entryGenerator = Generator(output);
    if (!entryGenerator)
        return nullptr;

    uint32 entry = entryGenerator->GenerateEntry(store);
    output->ItemId = entry;

    // Select a display ID for the item based on type
    GenerateItemDisplay(output, modifier);

    delete store[entry];
    store[entry] = output;

    if (sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
        output->InitializeQueryData();

    return output;
}

void VirtualItemMgr::GenerateStatGroup(VirtualItemTemplate* output, VirtualModifier& modifier) const
{
    std::mt19937 generator;
    generator.seed(modifier.statGroupSeed);
    StatGroup statgroupid;
    StatGroup statgroupbiasid;

    // select a random stat group
    statgroupid = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 2, generator));

    // if the player has a loot preference, roll for bias statgroup
    if (modifier.lootPreference > 0 && modifier.lootPreference < MAX_PREF)
    {
        // grab available loot preference stat groups
        std::vector<StatGroup> const& substatgroups = premadeStatGroupData.GetPlayerLootPreference(modifier.lootPreference);
        statgroupbiasid = substatgroups[urand(0, substatgroups.size() - 1, generator)];

        // 25% chance of bias, might want to make this into a config later on
        uint32 chance = 25;
        if (urand(1, 100, generator) <= chance)
            statgroupid = statgroupbiasid;
    }

    // override stat group if the item is flagged as a specific type
    if ((output->FlagsCu & ITEM_FLAGS_CU_VIRT_CLASS_WARDEN) != 0)
        statgroupid = STAT_GROUP_STR_TANK;
    else if ((output->FlagsCu & ITEM_FLAGS_CU_VIRT_CLASS_HISTORIAN) != 0)
        statgroupid = STAT_GROUP_HEALING;
    else if ((output->FlagsCu & ITEM_FLAGS_CU_VIRT_CLASS_WEAVER) != 0)
        statgroupid = STAT_GROUP_INT_DPS;
    else if ((output->FlagsCu & ITEM_FLAGS_CU_VIRT_CLASS_WATCHER) != 0)
        statgroupid = STAT_GROUP_STR_DPS;
    else if ((output->FlagsCu & ITEM_FLAGS_CU_VIRT_CLASS_RANGER) != 0)
        statgroupid = STAT_GROUP_AGI_DPS;

    // if the modifier is not set to random (default value), override selected stat group
    if (modifier.statgroup != STAT_GROUP_RANDOM)
        statgroupid = modifier.statgroup;

    ASSERT(statgroupid < STAT_GROUP_COUNT); // must not be random anymore

    output->statGroup = statgroupid;
}

void VirtualItemMgr::GenerateBaseStats(VirtualItemTemplate* output, VirtualModifier& modifier) const
{
    std::mt19937 generator;
    generator.seed(modifier.statSeed);

    // always bind on pickup
    output->Bonding = BIND_WHEN_PICKED_UP;

    // if item is a legendary or higher, flag as BoA
    if(output->Quality >= ITEM_QUALITY_LEGENDARY)
        output->Flags += ITEM_FLAG_IS_BOUND_TO_ACCOUNT;

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
    vLevel += irand(-1, 5, generator);
    // Add all vLvl mods before generating a new ilevel
    vLevel += modifier.vLvlMod;

    // Get the new item level based on above modifier virtual level
    ilevel = round(GenerateItemLevel(vLevel));

    // Modify the returned, newly generated iLevel based on quality
    ilevel += (int32(output->Quality) * 3);

    // One last mod to the ilevel to try to smooth out any ilevel groups and spikes
    ilevel += irand(-3, 3, generator);

    // If ilevel modifier is set, override all ilevel generation
    if (modifier.ilevel)
        ilevel = modifier.ilevel;

    // Hard cap of 325 across all items FIXME
    if (ilevel > sWorld->getIntConfig(CONFIG_MAX_ITEM_LEVEL))
        ilevel = sWorld->getIntConfig(CONFIG_MAX_ITEM_LEVEL);

    // decide armor, if item class is armor and not of type misc, armor should always be applied.
    if (output->Class == ITEM_CLASS_ARMOR && output->SubClass != ITEM_SUBCLASS_ARMOR_MISC)
    {
        // by default we assume 1 ilevel = 1 armor
        float armorValue = (float)ilevel;

        // retrieve armor slot and type multiplier
        armorValue *= VirtualModifier::GetTypeSlotArmorModifier(output);;

        // depending on quality, add multiplier to armor piece
        // we use the same modifier as the stat quality modifier
        // reconsider this if we ever need to change stat quality modifiers
        armorValue *= VirtualModifier::GetQualityStatModifier(output);

        // add a random 10% increase or decrease of stats
        armorValue = (float)urand((uint32)(armorValue * 0.9f), (uint32)(armorValue * 1.1f), generator);

        float honePct = modifier.statPoolPctModifier;
        if (honePct > 0)
        {
            honePct = modifier.statPoolPctModifier / 100;
            armorValue += armorValue * honePct;
        }

        // apply armor value to template
        output->Armor = (uint32)armorValue;
    }

    // Apply block rating to shields
    if (output->Class == ITEM_CLASS_ARMOR && output->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        output->Block = uint32(0.93f * float(ilevel));

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
            {
                output->Delay = (urand(15, 27, generator) * 100);
                output->Damage[0].DamageMin = ((0.83f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((0.83f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;
                break;
            }
            case ITEM_SUBCLASS_WEAPON_FIST:
            {
                output->Delay = (urand(15, 27, generator) * 100);
                output->Damage[0].DamageMin = ((0.83f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((0.83f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageType = 0;

                // fist weapons need to either be main or offhand, special case for this
                output->InventoryType = urand(0, 1, generator) > 0 ? INVTYPE_WEAPONMAINHAND : INVTYPE_WEAPONOFFHAND;
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
                output->Damage[0].DamageMin = ((1.35f * float(ilevel)) * 0.85f) * (float(output->Delay) / 1000.0f);
                output->Damage[0].DamageMax = ((1.35f * float(ilevel)) * 1.15f) * (float(output->Delay) / 1000.0f);
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
        if ((output->statGroup == STAT_GROUP_HEALING || output->statGroup == STAT_GROUP_INT_DPS) && output->InventoryType != INVTYPE_RANGED)
        {
            output->Damage[0].DamageMin /= 2.0f;
            output->Damage[0].DamageMax /= 2.0f;
        }

        // Apply a damage bonus based on item quality
        float damageBonus = (((output->Quality - 2.0f) / 10.0f) / 2.0f) + 1.0f;
        output->Damage[0].DamageMin *= damageBonus;
        output->Damage[0].DamageMax *= damageBonus;
        float honePct = modifier.statPoolPctModifier;
        if (honePct > 0)
        {
            honePct = modifier.statPoolPctModifier / 100;
            output->Damage[0].DamageMin += output->Damage[0].DamageMin * honePct;
            output->Damage[0].DamageMax += output->Damage[0].DamageMax * honePct;
        }

    }

    // TODO: add custom descriptions to legendaries possibly?
    // currently used to clean description of base template for crafting etc.
    output->Description = "";

    // apply other item data
    output->ItemLevel = ilevel;
     //output->MaxDurability = 0; // Disable any form of durability for now
    output->MaxDurability = round(float((output->ItemLevel * (output->Quality / 10.f)) + 25));
}

void VirtualItemMgr::GenerateItemStats(VirtualItemTemplate* output, VirtualModifier& modifier) const
{
    std::mt19937 generator;
    generator.seed(modifier.statValueSeed);

    // get statgroup id
    StatGroup statgroupid = output->statGroup;

    // create key-value vector for selected stats
    std::vector<std::pair<ItemModType, float>> selectedStats;

    // get stats for primary and secondary stat groups
    std::vector<ItemModType> primarystatgroup = premadeStatGroupData.GetStatGroupPrimaryStats(statgroupid, generator);
    std::vector<ItemModType> secondarystatgroup = premadeStatGroupData.GetStatGroupSecondaryStats(statgroupid, generator);

    // override to add MagicFind to rings/trinkets only
    if (output->Class == ITEM_CLASS_ARMOR && (output->InventoryType == INVTYPE_NECK || output->InventoryType == INVTYPE_FINGER))
        secondarystatgroup.push_back(ITEM_MOD_MAGICFIND);

    // shuffle vectors so we can pick top values without using any rand calls
    std::shuffle(std::begin(primarystatgroup), std::end(primarystatgroup), generator);
    std::shuffle(std::begin(secondarystatgroup), std::end(secondarystatgroup), generator);

    // select stat pool
    float pool = 0;
    if (modifier.statpool == -1)
        pool = (float)output->ItemLevel;
    else
        pool = (float)modifier.statpool;

    // get stat pool amount
    uint32 primaryStatSlots = VirtualModifier::GetPrimaryStatSlots(output);
    uint32 secondaryStatSlots = VirtualModifier::GetSecondaryStatSlots(output);

    // if this is a trinket, randomly select which slot to generate a stat for
    if (output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET)
    {
        // set both primary and secondary stats to none
        primaryStatSlots = 0;
        secondaryStatSlots = 0;

        // randomly select primary or secondary stat
        bool isPrimary = (urand(0, 1, generator) != 0);
        if (isPrimary)
            primaryStatSlots = 1;
        else
            secondaryStatSlots = 1;
    }

    if (legendaryItemInfo const* leg = GetLegendaryItemInfo(output->legendaryId))
    {
        // Primary stat count modifier can both add and subtract
        if (leg->primaryStatCountMod && leg->primaryStatCountMod != 0)
            primaryStatSlots = leg->primaryStatCountMod;

        if (leg->secondaryStatCountMod && leg->secondaryStatCountMod != 0)
            secondaryStatSlots = leg->secondaryStatCountMod;
    }

    // multiply the pool size by the base amounts of stat slots
    pool *= (float)(primaryStatSlots + secondaryStatSlots);

    // randomly select between -1 and +1 additional stat slots
    // this does not apply to trinkets
    if (output->Class != ITEM_CLASS_ARMOR && output->InventoryType != INVTYPE_TRINKET)
    {
        std::uniform_int_distribution<int> dist(-1, 1);
        int primarySlotMod = dist(generator);
        int secondarySlotMod = dist(generator);

        // check whether or not the amount of stats exceeds the size of our stat group
        if ((primaryStatSlots + primarySlotMod) > primarystatgroup.size())
            primaryStatSlots = primarystatgroup.size();
        else if ((primaryStatSlots + primarySlotMod) < 1) // never generate 0 primary stats
            primaryStatSlots = 1;
        else
            primaryStatSlots += primarySlotMod;

        if ((secondaryStatSlots + secondarySlotMod) > secondarystatgroup.size())
            secondaryStatSlots = secondarystatgroup.size();
        else
            secondaryStatSlots += secondarySlotMod;
    }

    //Random hone percent in future?
    //output->honePct = modifier.statPoolPctModifier;
    float honePct = modifier.statPoolPctModifier;
    if (honePct > 0)
        honePct = modifier.statPoolPctModifier / 100;

    // if we still have any slots to generate stats for, continue
    if (primaryStatSlots + secondaryStatSlots > 0)
    {
        // divide the pool by the modified amount of stat slots
        pool /= (float)(primaryStatSlots + secondaryStatSlots);

        // divide per-stat pool by predefined blizzlike value
        pool *= sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_POOLMOD);

        // for armor specifically we want to modify the pool size depending on the selected stat group and armor type
        // does not apply to rings, trinkets, cloaks and shields
        if (output->Class == ITEM_CLASS_ARMOR && (output->InventoryType != INVTYPE_SHIELD || output->InventoryType != INVTYPE_TRINKET || output->InventoryType != INVTYPE_FINGER || output->InventoryType != INVTYPE_NECK || output->InventoryType != INVTYPE_CLOAK))
            pool *= VirtualModifier::GetArmorTypeStatGroupModifier(output);

        // generate primary stat values  for all stats in group
        for (uint32 i = 0; i < primarystatgroup.size(); ++i)
        {
            float primaryStatMod = 1.f;

            if (legendaryItemInfo const* leg = GetLegendaryItemInfo(output->legendaryId))
                primaryStatMod = leg->primaryStatModifier;

            primaryStatMod += honePct;

            // select random pool size value based on upper and lower bounds
            float statPoints = (float)urand((uint32)(pool * sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_LOWBOUND)), (uint32)(pool * sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_HIGHBOUND)), generator);

            statPoints *= primaryStatMod;
            // mod stat points based on stat weight
            statPoints *= VirtualModifier::GetStatRate(primarystatgroup[i]);

            // mod stat points based on item quality
            statPoints *= VirtualModifier::GetQualityStatModifier(output);

            // mod stat points based on item slot
            statPoints *= VirtualModifier::GetSlotStatModifier(output);

            // mod stat points based on stat tier
            statPoints *= sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_PRIMARY_MOD);

            // hard coded overrides for primary stats
            if (primarystatgroup[i] == ITEM_MOD_STAMINA)
            {
                switch (statgroupid)
                {
                    case STAT_GROUP_STR_DPS:
                        statPoints *= 1.5f;
                        break;
                    case STAT_GROUP_STR_TANK:
                        statPoints *= 1.73f;
                        break;
                    case STAT_GROUP_AGI_DPS:
                    case STAT_GROUP_AGI_TANK:
                        statPoints *= 1.32f;
                        break;
                    default:
                        break;
                }
            }
            else if (primarystatgroup[i] == ITEM_MOD_STRENGTH)
            {
                switch (statgroupid)
                {
                    case STAT_GROUP_STR_DPS:
                    case STAT_GROUP_STR_TANK:
                        statPoints *= 1.32f;
                        break;
                    default:
                        break;
                }
            }
            else if (primarystatgroup[i] == ITEM_MOD_AGILITY)
            {
                switch (statgroupid)
                {
                    case STAT_GROUP_AGI_DPS:
                    case STAT_GROUP_AGI_TANK:
                        statPoints *= 1.32f;
                        break;
                    default:
                        break;
                }
            }

            if (i < primaryStatSlots && primaryStatSlots > 0)
            {
                selectedStats.push_back(std::pair(primarystatgroup[i], statPoints));
            }
        }

        // generate secondary stat values
        for (uint32 i = 0; i < secondarystatgroup.size(); ++i)
        {
            float secondayStatMod = 1.f;

            if (legendaryItemInfo const* leg = GetLegendaryItemInfo(output->legendaryId))
                secondayStatMod = leg->secondaryStatModifier;

            secondayStatMod += honePct;

            // select random pool size value based on upper and lower bounds
            float statPoints = (float)urand((uint32)(pool * sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_LOWBOUND)), (uint32)(pool * sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_HIGHBOUND)), generator);

            statPoints *= secondayStatMod;

            // mod stat points based on stat weight
            statPoints *= VirtualModifier::GetStatRate(secondarystatgroup[i]);

            // mod stat points based on item quality
            statPoints *= VirtualModifier::GetQualityStatModifier(output);

            // mod stat points based on item slot
            statPoints *= VirtualModifier::GetSlotStatModifier(output);

            // mod stat points based on stat tier
            statPoints *= sWorld->getFloatConfig(CONFIG_ITEMGEN_STATGEN_SECONDARY_MOD);

            // hard coded behavior for weapons with spell power.
            if (secondarystatgroup[i] == ITEM_MOD_SPELL_POWER)
            {
                switch (output->InventoryType)
                {
                    case INVTYPE_2HWEAPON:
                    case INVTYPE_WEAPON:
                    case INVTYPE_WEAPONMAINHAND:
                    case INVTYPE_WEAPONOFFHAND:
                        statPoints *= 4.0f;
                        break;
                    default:
                        break;
                }
            }

            if (i < secondaryStatSlots && secondaryStatSlots > 0)
            {
                // if the stat selected is magic find, we want to apply it directly and not push it to the selected stat array
                if (secondarystatgroup[i] == ITEM_MOD_MAGICFIND)
                {
                    uint32 magicFindId = VirtualModifier::GetMagicFindId(statPoints);
                    if (magicFindId > 0)
                    {
                        uint32 slot = 0;
                        output->Spells[slot].SpellId = magicFindId;
                        output->Spells[slot].SpellTrigger = ITEM_SPELLTRIGGER_ON_EQUIP; // onEquip
                        // are these needed?
                        /* output->Spells[slot].SpellCharges = 0;
                        output->Spells[slot].SpellPPMRate = 0;
                        output->Spells[slot].SpellCooldown = 0;
                        output->Spells[slot].SpellCategory = 0;
                        output->Spells[slot].SpellCategoryCooldown = 0; */
                    }
                    continue;
                }

                // push stat back to the selected stat vector
                selectedStats.push_back(std::pair(secondarystatgroup[i], statPoints));
            }
        }
    }

    // iterate selected stats and apply to item
    uint32 setStats = 0;
    for (uint32 i = 0; i < selectedStats.size(); ++i)
    {
        uint32 type = (uint32)selectedStats[i].first;
        float value = selectedStats[i].second;

        output->ItemStat[i].ItemStatType = type;
        output->ItemStat[i].ItemStatValue = value;

        setStats = std::max(setStats, uint32(i + 1));
    }
    if (setStats > MAX_ITEM_PROTO_STATS)
        setStats = MAX_ITEM_PROTO_STATS;
    output->StatsCount = setStats;
}

void VirtualItemMgr::GenerateVirtualLevelLookupArray()
{
    WriteGuard guard(lock);
    float iLevel = 0.0f;

    // Generate lookup table for Virtual Levels.
    for (int i = 0; iLevel < sWorld->getIntConfig(CONFIG_MAX_ITEM_LEVEL); ++i)
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
    float ilevel = ((pow((x + 0.0555f), 2) - 1.0f) / pow((x + 0.0555f), 2)) + float(sWorld->getIntConfig(CONFIG_MAX_ITEM_LEVEL));

    return ilevel;
}

void VirtualItemMgr::GenerateItemName(VirtualItemTemplate* output, VirtualModifier& modifier) const
{
    if (!modifier.nameOverride.empty())
    {
        output->Name1 = modifier.nameOverride;
        return;
    }

    std::mt19937 generator;
    generator.seed(modifier.nameSeed);

    std::string fullName = "";
    std::stringstream ss;

    // For Weapons we always use inventoryType 0
    uint32 inventoryType = 0;
    if (output->Class == ITEM_CLASS_ARMOR)
        inventoryType = output->InventoryType;

    // Retrieve all the string lists
    std::map<uint32, std::vector<std::string>> nameLists;
    std::map<uint32, std::string> selectedWords;
    for (size_t i = 1; i <= 8; ++i)
    {
        NameInfo nameInfo(output->Class, output->SubClass, inventoryType, i);
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

    // the temporary name is the same as the template name, we can assume lists were missing. skip generating names.
    if (fullName != output->Name1)
    {
        // List 1: Unique names, like Malice, Mangler, Mercy etc.
        // List 2: Prefixes, like Arcane, Arched, Bloodied etc.
        // List 3: Material names, like Bone, Copper, Diamond etc.
        // List 4: Basic type name, like Blade, Razor, Maul etc.
        // List 5: Exotic type name, like Blade, Carver etc. Contains more exotic, but also the basic, type names.
        // List 6: Suffixes, like "of Agony", "of Bloodlust" etc.

        // Shields have their names generated like weapons.
        if (output->Class == ITEM_CLASS_ARMOR && output->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
        {
            switch (output->Quality)
            {
                case ITEM_QUALITY_NORMAL:
                    ss << selectedWords[8] << " " << selectedWords[5];
                    break;
                case ITEM_QUALITY_UNCOMMON:
                    ss << selectedWords[4] << " " << selectedWords[5];
                    break;
                case ITEM_QUALITY_RARE:
                    ss << selectedWords[4] << " " << selectedWords[5] << " of " << selectedWords[2];
                    break;
                case ITEM_QUALITY_EPIC:
                    ss << selectedWords[3] << " " << selectedWords[4] << " " << selectedWords[5];
                    break;
                case ITEM_QUALITY_LEGENDARY:
                    ss << selectedWords[1] << ", " << selectedWords[3] << " " << selectedWords[5] << " of " << selectedWords[7];
                    break;
                default:
                    ss << fullName;
                    break;
            }
        }

        // Shields have their names generated like weapons.
        if (output->Class == ITEM_CLASS_WEAPON || (output->Class == ITEM_CLASS_ARMOR && output->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD))
        {
            switch (output->Quality)
            {
                case ITEM_QUALITY_NORMAL:
                    ss << selectedWords[8] << " " << selectedWords[4];
                    break;
                case ITEM_QUALITY_UNCOMMON:
                    ss << selectedWords[3] << " " << selectedWords[4];
                    break;
                case ITEM_QUALITY_RARE:
                    ss << selectedWords[2] << " " << selectedWords[4];
                    break;
                case ITEM_QUALITY_EPIC:
                    ss << selectedWords[1];
                    break;
                case ITEM_QUALITY_LEGENDARY:
                    ss << selectedWords[1] << ", " << selectedWords[5] << " " << selectedWords[6];
                    break;
                default:
                    ss << fullName;
                    break;
            }
        }
        output->Name1 = ss.str();
    }
}

void VirtualItemMgr::GenerateItemDisplay(VirtualItemTemplate* output, VirtualModifier& modifier)
{
    std::mt19937 generator;
    generator.seed(modifier.displaySeed);
    uint32 display;

    std::vector<displayInfo> displays;
    for (displayInfo const &someDisplays : availableDisplays)
    {
        if (output->Quality == ITEM_QUALITY_LEGENDARY)
        {
            if (someDisplays.quality < ITEM_QUALITY_EPIC)
                continue;
            if (someDisplays.quality >= ITEM_QUALITY_ARTIFACT)
                continue;
        }
        else
        {
            if (output->Quality != someDisplays.quality)
                continue;
        }

        if (output->InventoryType != someDisplays.iInventoryType)
            continue;

        if (output->Class != someDisplays.iClass)
            continue;

        if (output->SubClass != someDisplays.isubClass)
            continue;

        displays.push_back(someDisplays);
    }

    // if there are no found displays, default to DBC display. Do the same for trinkets and rings.
    // else shuffle list and pick first
    if (displays.empty())
    {
        display = output->GetDBCDisplay();
    } 
    else
    {
        std::shuffle(std::begin(displays), std::end(displays), generator);
        display = displays.front().displayId;
    }

    // if a display override has been set earlier by set, legendary etc, apply this display
    if (modifier.displayId)
        display = modifier.displayId;

    // randomize sheath position if the item is a 1h sword, axe or dagger
    if (output->Class == ITEM_CLASS_WEAPON && (output->SubClass == ITEM_SUBCLASS_WEAPON_AXE || output->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || output->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER))
    {
        // 50% chance for back sheath
        if (urand(0, 1, generator) < 1)
            output->Sheath = 1;
    }

    // If an item is flagged as static display, use the static display
    else if (output->HasFlag(VIRTUAL_ITEM_FLAG_DISPLAY_STATIC))
        display = output->DisplayInfoID;

    output->DisplayInfoID = display;
}

itemSpellInfo VirtualItemMgr::GenerateSpell(VirtualItemTemplate* output, VirtualModifier& modifier)
{
    std::mt19937 generator;
    generator.seed(modifier.spellSeed);

    std::list<itemSpellInfo> spells;
    bool hasExistingOnUse = false;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (output->Spells[i].SpellId > 0 && output->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE) // Don't stack two on use effects.
        {
            hasExistingOnUse = true;
            break;
        }
    }

    for (itemSpellInfo const &someSpells : availableSpells)
    {
        SelectMinMaxSkip(someSpells.minQuality, someSpells.maxQuality, output->Quality);
        SelectMinMaxSkip(someSpells.minItemLevel, someSpells.maxItemLevel, output->ItemLevel);
        SelectSkip(someSpells.itemClass, output->Class);
        SelectSkip(someSpells.subClass, output->SubClass);
        SelectSkip(someSpells.inventoryType, output->InventoryType);
        SelectSkip(someSpells.statGroup, output->statGroup);

        if(someSpells.itemClass <= -2)
        {
            if (output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET)
                continue;
        }

        if (hasExistingOnUse && someSpells.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
            continue;

        spells.push_back(someSpells);
    }

    if (spells.empty())
        return itemSpellInfo();

    auto selectedSpell = std::begin(spells);
    std::advance(selectedSpell, urand(0, uint32(std::size(spells)) - 1, generator));

    return *selectedSpell;
}

itemSetInfo VirtualItemMgr::GenerateSet(VirtualItemTemplate* output, VirtualModifier& modifier)
{
    std::mt19937 generator;
    generator.seed(modifier.setSeed);

    std::list<itemSetInfo> sets;
    for (itemSetInfo const& someSets : availableItemSets)
    {
        SelectMinMaxSkip(someSets.minQuality, someSets.maxQuality, (int32)output->Quality);
        SelectMinMaxSkip(someSets.minItemLevel, someSets.maxItemLevel, (int32)output->ItemLevel);
        SelectSkip(someSets.itemClass, (int32)output->Class);
        SelectSkip(someSets.subClass, (int32)output->SubClass);
        SelectSkip(someSets.inventoryType, (int32)output->InventoryType);
        SelectSkip(someSets.statGroup, output->statGroup);

        sets.push_back(someSets);
    }

    if (sets.empty())
        return itemSetInfo();

    auto selectedSet = std::begin(sets);
    std::advance(selectedSet, urand(0, uint32(std::size(sets)) - 1, generator));

    return *selectedSet;
}

void VirtualItemMgr::GenerateSpells(VirtualItemTemplate* output, VirtualModifier& modifier)
{

    uint8 numOfSpell = 0;

    // Only Guarantee spells on trinkets
    if (output->Class == ITEM_CLASS_ARMOR && output->InventoryType == INVTYPE_TRINKET)
        numOfSpell = 1;
    else
    {
        std::mt19937 generator;
        generator.seed(modifier.spellSeed);
        float randChance = frand(0.f, 100.f, generator);
        if (randChance >= sWorld->getFloatConfig(WorldFloatConfigs(CONFIG_ITEMGEN_SPELL_CHANCE_POOR + output->Quality - 1)))
            numOfSpell = 1;
    }

    if (!numOfSpell)
        return;

    itemSpellInfo spell = GenerateSpell(output, modifier);

    if (spell.spellId == 0)
        return;

    output->Spells[0].SpellId = spell.spellId;
    output->Spells[0].SpellTrigger = spell.SpellTrigger;
    output->Spells[0].SpellCharges = spell.SpellCharges;
    output->Spells[0].SpellPPMRate = spell.SpellPPMRate;
    output->Spells[0].SpellCooldown = spell.SpellCooldown;
    output->Spells[0].SpellCategory = spell.SpellCategory;
    output->Spells[0].SpellCategoryCooldown = spell.SpellCategoryCooldown;
}

void VirtualItemMgr::GenerateSockets(VirtualItemTemplate* output, VirtualModifier& modifier, bool reRoll)
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
    if (output->Class == ITEM_CLASS_ARMOR && output->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
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
                break;
        }
    }

    if (output->Class == ITEM_CLASS_WEAPON)
    {
        switch (output->InventoryType)
        {
        case INVTYPE_2HWEAPON:
            if (output->Quality >= ITEM_QUALITY_RARE && socketCount < 2)
                socketCount = 2;
            break;
        }
    }

    if (legendaryItemInfo const* leg = GetLegendaryItemInfo(output->legendaryId))
    {
        if (leg->socketMod && leg->socketMod != 0)
            socketCount += leg->socketMod;

        if (leg->generatePrismatic && socketCount >= 3)
            socketCount = 2;
    }

    // set socket colors
    std::vector<SocketColor> const& socketcolors = premadeStatGroupData.GetStatGroupSockets(reRoll ? STAT_GROUP_ALL : output->statGroup, generator);
    if (!socketcolors.empty())
    {
        for (int32 i = 0; i < socketCount; ++i)
        {
            if (output->Socket[i].Color != 0 && !reRoll)
            {
                //Dummy call to urand to keep consistency.
                urand(0, socketcolors.size() - 1, generator);
                continue;
            }
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

void VirtualItemMgr::GenerateQuality(VirtualItemTemplate* output, VirtualModifier& modifier, bool /*reRoll*/)
{
    std::mt19937 generator;
    generator.seed(modifier.qualitySeed);

    // get initial quality from the template item
    uint32 quality = output->Quality;

    uint32 magicFind = output->generatedMagicFind != 0 ? output->generatedMagicFind : modifier.magicFind;

    output->generatedMagicFind = magicFind;
    
    // these are not percentage chances. They represent areas of a number line made from their sum
    static const uint32 chances[MAX_ITEM_QUALITY] = {
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_POOR),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_COMMON) - magicFind,
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_UNCOMMON),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_RARE),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_EPIC),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_LEGENDARY),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_ARTIFACT),
        sWorld->getIntConfig(CONFIG_ITEMGEN_QUALITY_HEIRLOOM),
    };

    float sum = 0;
    for (auto u : chances)
    {
        sum += u;
    }

    if (sum >= 1)
    {
        float rand = urand(1.f, sum, generator);
        sum = 0;
        for (size_t i = 0; i < MAX_ITEM_QUALITY; ++i)
        {
            if (chances[i] <= 0)
                continue;

            sum += chances[i];
            if (sum < rand)
                continue;

            quality = i;
            break;
        }
    }

    quality = std::max(output->Quality, quality); // dont generate quality below original

    // if the minQuality modifier is greater than the determined quality, then set the quality to the minQuality
    if (modifier.minQuality > quality && modifier.minQuality < MAX_ITEM_QUALITY)
        quality = modifier.minQuality;

    // If the quality modifier is set, discard generated quality and force specific quality
    if (modifier.quality < MAX_ITEM_QUALITY)
        quality = modifier.quality;

    output->Quality = quality;
}

void VirtualItemMgr::GenerateAdditonalStat(VirtualItemTemplate* /*output*/)
{
    uint8 stats[21] = {
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

void VirtualItemMgr::GenerateItemSet(VirtualItemTemplate* output, VirtualModifier& modifier)
{
    std::mt19937 generator;
    generator.seed(modifier.setSeed);

    itemSetInfo set = GenerateSet(output, modifier);

    uint32 qualityChance = VirtualModifier::GetSetChance(output);

    ASSERT(qualityChance <= 100);

    uint32 chanceRng = urand(1, 100, generator);

    if (set.setId > 0 && (chanceRng <= qualityChance || modifier.generateSet == true))
    {
        output->ItemSet = set.setId;

        if (set.displayOverride)
            modifier.displayId = set.displayOverride;

        if (!set.flavorOverride.empty())
            output->Description = set.flavorOverride;
    }
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

    for (auto i = virtual_level_info.begin(); i != virtual_level_info.end(); i++)
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

std::vector<std::string> VirtualItemMgr::GetNamesForNameInfo(NameInfo* info) const
{
    if (!info)
        return std::vector<std::string>();

    std::vector<std::string> names;
    for (auto name : availableNames)
    {
        SelectSkip(name.array_id, info->array_id);
        SelectSkip(name.itemType, info->itemType);
        SelectSkip(name.subclass, info->subclass);
        SelectSkip(name.inventoryType, info->inventoryType);

        names.push_back(name.name);
    }
    return names;
}

std::vector<ItemModType> const& VirtualItemMgr::StatGroupData::GetStatGroupPrimaryStats(StatGroup group, std::mt19937& generator) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));

    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_primary_stats[group];
}

std::vector<ItemModType> const& VirtualItemMgr::StatGroupData::GetStatGroupSecondaryStats(StatGroup group, std::mt19937& generator) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));

    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_secondary_stats[group];
}

std::vector<SocketColor> const& VirtualItemMgr::StatGroupData::GetStatGroupSockets(StatGroup group, std::mt19937& generator) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, generator));

    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_sockets[group];
}

std::vector<StatGroup> const& VirtualItemMgr::StatGroupData::GetPlayerLootPreference(uint8 preference) const
{
    return preference_stat_groups[preference];
}

uint32 VirtualModifier::GetPrimaryStatSlots(VirtualItemTemplate* output)
{
    switch (output->Quality)
    {
        case ITEM_QUALITY_NORMAL:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_PRIMARY_COMMON);
        case ITEM_QUALITY_UNCOMMON:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_PRIMARY_UNCOMMON);
        case ITEM_QUALITY_RARE:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_PRIMARY_RARE);
        case ITEM_QUALITY_EPIC:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_PRIMARY_EPIC);
        case ITEM_QUALITY_LEGENDARY:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_PRIMARY_LEGENDARY);
        default:
            return 1;
    }

    return 1;
}

uint32 VirtualModifier::GetSecondaryStatSlots(VirtualItemTemplate* output)
{
    switch (output->Quality)
    {
        case ITEM_QUALITY_NORMAL:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_SECONDARY_COMMON);
        case ITEM_QUALITY_UNCOMMON:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_SECONDARY_UNCOMMON);
        case ITEM_QUALITY_RARE:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_SECONDARY_RARE);
        case ITEM_QUALITY_EPIC:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_SECONDARY_EPIC);
        case ITEM_QUALITY_LEGENDARY:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_STATSLOT_SECONDARY_LEGENDARY);
        default:
            return 1;
    }

    return 1;
}

float VirtualModifier::GetQualityStatModifier(VirtualItemTemplate* output)
{
    switch (output->Quality)
    {
        case ITEM_QUALITY_NORMAL:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_QUALITYMOD_COMMON);
        case ITEM_QUALITY_UNCOMMON:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_QUALITYMOD_UNCOMMON);
        case ITEM_QUALITY_RARE:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_QUALITYMOD_RARE);
        case ITEM_QUALITY_EPIC:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_QUALITYMOD_EPIC);
        case ITEM_QUALITY_LEGENDARY:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_QUALITYMOD_LEGENDARY);
        default:
            return 1.0f;
    }
    return 1.0f;
}

float VirtualModifier::GetSlotStatModifier(VirtualItemTemplate* output)
{
    switch (output->InventoryType)
    {
        case INVTYPE_HEAD:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_HEAD);
        case INVTYPE_CHEST:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_CHEST);
        case INVTYPE_ROBE:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_ROBE);
        case INVTYPE_LEGS:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_LEGS);
        case INVTYPE_2HWEAPON:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_2HWEAPON);
        case INVTYPE_SHOULDERS:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_SHOULDERS);
        case INVTYPE_HANDS:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_HANDS);
        case INVTYPE_WAIST:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_WAIST);
        case INVTYPE_FEET:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_FEET);
        case INVTYPE_WRISTS:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_WRISTS);
        case INVTYPE_NECK:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_NECK);
        case INVTYPE_CLOAK:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_CLOAK);
        case INVTYPE_HOLDABLE:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_HOLDABLE);
        case INVTYPE_SHIELD:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_SHIELD);
        case INVTYPE_WEAPON:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_WEAPON);
        case INVTYPE_WEAPONMAINHAND:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_WEAPONMAINHAND);
        case INVTYPE_WEAPONOFFHAND:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_WEAPONOFFHAND);
        case INVTYPE_FINGER:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_FINGER);
        case INVTYPE_TRINKET:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_TRINKET);
        case INVTYPE_RANGED:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_RANGED);
        case INVTYPE_RANGEDRIGHT:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_RANGEDRIGHT);
        case INVTYPE_THROWN:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_SLOTWEIGHT_THROWN);
        default:
            return 1.0f;
    }
    return 1.0f;
}

float VirtualModifier::GetStatRate(ItemModType stat)
{
    switch (stat)
    {
        /* Primary stats */
        case ITEM_MOD_STAMINA:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_STAMINA);
        case ITEM_MOD_AGILITY:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_AGILITY);
        case ITEM_MOD_INTELLECT:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_INTELLECT);
        case ITEM_MOD_STRENGTH:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_STRENGTH);
        case ITEM_MOD_SPIRIT:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_SPIRIT);
        /* Defensive stats */
        case ITEM_MOD_DEFENSE_SKILL_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_DEFENSE_SKILL_RATING);
        case ITEM_MOD_DODGE_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_DODGE_RATING);
        case ITEM_MOD_PARRY_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_PARRY_RATING);
        case ITEM_MOD_BLOCK_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_BLOCK_RATING);
        case ITEM_MOD_BLOCK_VALUE:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_BLOCK_VALUE);
        /* Spell stats */
        case ITEM_MOD_MANA_REGENERATION:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_MANA_REGENERATION);
        case ITEM_MOD_SPELL_POWER:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_SPELL_POWER);
        case ITEM_MOD_SPELL_PENETRATION:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_SPELL_PENETRATION);
        /* Melee stats */
        case ITEM_MOD_EXPERTISE_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_EXPERTISE_RATING);
        case ITEM_MOD_ARMOR_PENETRATION_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_ARMOR_PENETRATION_RATING);
        case ITEM_MOD_ATTACK_POWER:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_ATTACK_POWER);
        /* Global stats */
        case ITEM_MOD_HIT_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_HIT_RATING);
        case ITEM_MOD_HASTE_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_HASTE_RATING);
        case ITEM_MOD_CRIT_RATING:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_CRIT_RATING);
        /* Other / Unused */
        case ITEM_MOD_HEALTH_REGEN:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_HEALTH_REGEN);
        case ITEM_MOD_MAGICFIND:
            return sWorld->getFloatConfig(CONFIG_ITEMGEN_STATWEIGHT_MAGICFIND);
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

float VirtualModifier::GetArmorTypeStatGroupModifier(VirtualItemTemplate* output)
{
    switch (output->statGroup)
    {
        case STAT_GROUP_STR_TANK:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 0.7f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 0.8f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                    return 0.9f;
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 1.0f;
                default:
                    return 1.0f;
            }
        case STAT_GROUP_AGI_TANK:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 0.7f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 1.0f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                    return 0.8f;
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 0.9f;
                default:
                    return 1.0f;
            }
        case STAT_GROUP_HEALING:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 0.9f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 1.0f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 0.7f;
                default:
                    return 1.0f;
            }
        case STAT_GROUP_INT_DPS:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 1.0f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 0.9f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 0.7f;
                default:
                    return 1.0f;
            }
        case STAT_GROUP_STR_DPS:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 0.7f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 0.9f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                    return 1.0f;
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 0.8f;
                default:
                    return 1.0f;
            }
        case STAT_GROUP_AGI_DPS:
            switch (output->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH:
                    return 0.7f;
                case ITEM_SUBCLASS_ARMOR_LEATHER:
                    return 1.0f;
                case ITEM_SUBCLASS_ARMOR_MAIL:
                    return 0.9f;
                case ITEM_SUBCLASS_ARMOR_PLATE:
                    return 0.8f;
                default:
                    return 1.0f;
            }
        default:
            return 1.0f;
    }
    return 1.0f;
}


uint32 VirtualModifier::GetSetChance(VirtualItemTemplate* output)
{
    switch (output->Quality)
    {
        case ITEM_QUALITY_NORMAL:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_SETCHANCE_COMMON);
        case ITEM_QUALITY_UNCOMMON:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_SETCHANCE_UNCOMMON);
        case ITEM_QUALITY_RARE:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_SETCHANCE_RARE);
        case ITEM_QUALITY_EPIC:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_SETCHANCE_EPIC);
        case ITEM_QUALITY_LEGENDARY:
            return sWorld->getIntConfig(CONFIG_ITEMGEN_SETCHANCE_LEGENDARY);
        default:
            return 0;
    }
    return 0;
}

uint32 VirtualModifier::GetMagicFindId(uint32 statPoints)
{
    struct {
        uint32 value;
        uint32 id;
    } list[] = {
        {1, 450100},
        {10, 450101},
        {15, 450102},
        {20, 450103},
        {25, 450104},
        {30, 450105},
        {35, 450106},
        {40, 450107},
        {45, 450108},
        {50, 450109},
        {55, 450110},
        {60, 450111},
        {65, 450112},
        {70, 450113},
        {75, 450114},
        {80, 450115},
        {85, 450116},
        {90, 450117},
        {95, 450118},
        {100, 450119},
    };

    for (int i = (sizeof(list) / sizeof(list[0])) - 1; i != -1; i--)
        if (statPoints >= list[i].value)
            return list[i].id;

    return 0;
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

void VirtualItemMgr::UpdateDisenchantIdNew(VirtualItemTemplate* output)
{
    uint32 quality = output->Quality;
    struct {
        uint32 quality;
        uint32 id;
    } list[] = {
        {1, 60030},
        {2, 60031},
        {3, 60032},
        {4, 60033},
        {5, 60034},
    };

    for (int i = (sizeof(list) / sizeof(list[0])) - 1; i != -1; i--)
        if (quality >= list[i].quality)
        {
            output->DisenchantID = list[i].id;
            break;
        }
}

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
            if (base->SubClass == (uint32)subclass)
                return true;
    }
    return false;
}

void VirtualItemMgr::LoadLegendaryTemplate()
{
    WriteGuard guard(lock);

    uint32 count = 0;
    uint32 beginTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `item_generator_legendary_template`");

    if (!result)
    {
        TC_LOG_INFO("server.loading", "Loaded 0 available virtual item legendary templates, table item_generator_legendary_template is empty.");
        return;
    }

    _LegendaryTemplateStore.reserve(result->GetRowCount());

    do {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        legendaryItemInfo& legTemp = _LegendaryTemplateStore[entry];

        legTemp.legendaryId = entry;
        legTemp.minItemLevel = fields[1].GetInt32();
        legTemp.maxItemLevel = fields[2].GetInt32();
        legTemp.itemClass = fields[3].GetInt8();
        legTemp.itemSubClass = fields[4].GetInt8();
        legTemp.itemInventoryType = fields[5].GetInt8();
        legTemp.itemStatGroup = fields[6].GetInt8();
        legTemp.primaryStatModifier = fields[7].GetFloat();
        legTemp.secondaryStatModifier = fields[8].GetFloat();
        legTemp.socketMod = fields[9].GetInt8();
        legTemp.generatePrismatic = fields[10].GetBool();
        legTemp.primaryStatCountMod = fields[11].GetInt8();
        legTemp.secondaryStatCountMod = fields[12].GetInt8();
        legTemp.statGroupOverride = fields[13].GetInt8();

        if (legTemp.socketMod > 3)
            legTemp.socketMod = 3;

        if (legTemp.socketMod == 3 && legTemp.generatePrismatic)
            legTemp.socketMod = 2;


        for (uint8 i = 0; i < MAX_LEGENDARY_SPELLS; ++i)
        {
            _Spell spell = _Spell();
            spell.SpellId = 0;
            spell.SpellTrigger = 0;
            spell.SpellCharges = -1;
            spell.SpellPPMRate = 0.f;
            spell.SpellCooldown = -1;
            spell.SpellCategory = 0;
            spell.SpellCategoryCooldown = -1;
            if (QueryResult spellEntry = WorldDatabase.PQuery("SELECT SpellId, SpellTrigger, SpellCharges, SpellPPMRate, SpellCooldown, SpellCategory, SpellCategoryCooldown"
                " FROM item_generator_legendary_spell_entry WHERE legendaryIndex = %u AND spellIndex = %u", legTemp.legendaryId, i))
            {
                Field* spellFields = spellEntry->Fetch();
                spell.SpellId = spellFields[0].GetUInt32();
                spell.SpellTrigger = spellFields[1].GetUInt32();
                spell.SpellCharges = spellFields[2].GetInt32();
                spell.SpellPPMRate = spellFields[3].GetFloat();
                spell.SpellCooldown = spellFields[4].GetInt32();
                spell.SpellCategory = spellFields[5].GetUInt32();
                spell.SpellCategoryCooldown = spellFields[6].GetInt32();
            }
            legTemp.legendarySpells[i] = spell;
        }
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded %u available virtual item legendary templates in %u MS.", count, GetMSTimeDiffToNow(beginTime));
}

legendaryItemInfo const* VirtualItemMgr::GetLegendaryItemInfo(uint32 id) const
{
    return Trinity::Containers::MapGetValuePtr(_LegendaryTemplateStore, id);
}

void VirtualItemMgr::GenerateLegendaryItemEffect(VirtualItemTemplate* output, VirtualModifier& modifier)
{
    if (output->Quality != ITEM_QUALITY_LEGENDARY)
        return;

    std::mt19937 generator;
    generator.seed(modifier.legendarySeed);
    std::list<legendaryItemInfo> legList;
    bool hasExistingOnUse = false;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (output->Spells[i].SpellId > 0 && output->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE) // Don't stack two on use effects.
        {
            hasExistingOnUse = true;
            break;
        }
    }
    for (auto const& itr : _LegendaryTemplateStore)
    {
        SelectMinMaxSkip(itr.second.minItemLevel, itr.second.maxItemLevel, output->ItemLevel);
        if (SelectSkipDebug(itr.second.itemClass, output->Class, "[Class]"))
            continue;
        if (SelectSkipDebug(itr.second.itemSubClass, output->SubClass, "[SubClass]"))
            continue;
        if (SelectSkipDebug(itr.second.itemInventoryType, output->InventoryType, "[InventoryType]"))
            continue;
        if (SelectSkipDebug(itr.second.itemStatGroup, output->statGroup, "[StatGroup]"))
            continue;
        bool skip = false;
        for (uint8 i = 0; i < MAX_LEGENDARY_SPELLS; ++i)
        {
            if (itr.second.legendarySpells[i].SpellId > 0 && itr.second.legendarySpells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE && hasExistingOnUse)
                skip = true;
        }
        if (skip)
            continue;
        legList.push_back(itr.second);
    }

    if (legList.empty())
        return;

    auto selectedLegendary = std::begin(legList);
    std::advance(selectedLegendary, urand(0, uint32(std::size(legList)) - 1, generator));

    output->legendaryId = selectedLegendary->legendaryId;

    if (selectedLegendary->statGroupOverride != -1)
        output->statGroup = static_cast<StatGroup>(selectedLegendary->statGroupOverride);

    for (uint8 i = 0; i < MAX_LEGENDARY_SPELLS; ++i)
    {
        if (selectedLegendary->legendarySpells[i].SpellId != 0)
        {
            output->Spells[i + MAX_GENERATED_SPELLS] = selectedLegendary->legendarySpells[i];
        }
    }
}

void VirtualItemMgr::UpdateHoneDisplaySpell(VirtualItemTemplate* output)
{
    output->Spells[HONED_SPELL_SLOT].SpellId = 410000 + output->honePct;
    output->Spells[HONED_SPELL_SLOT].SpellTrigger = ITEM_SPELLTRIGGER_ON_NO_DELAY_USE;
    output->Spells[HONED_SPELL_SLOT].SpellCharges = -1;
    output->Spells[HONED_SPELL_SLOT].SpellPPMRate = 0.f;
    output->Spells[HONED_SPELL_SLOT].SpellCooldown = -1;
    output->Spells[HONED_SPELL_SLOT].SpellCategory = 0;
    output->Spells[HONED_SPELL_SLOT].SpellCategoryCooldown = -1;
}

uint32 VirtualItemTemplate::GetDBCDisplay()
{
    // Get the correct display ID
    if (ItemEntry const* dbcitem = sItemStore.LookupEntry(ItemId))
        return dbcitem->DisplayId;

    return 0;
}

VirtualItemMgr::StatGroupData::StatGroupData()
{
    // Healing Data
    stat_group_primary_stats[STAT_GROUP_HEALING] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_SPIRIT
    };
    stat_group_secondary_stats[STAT_GROUP_HEALING] = {
        ITEM_MOD_HASTE_RATING,
        ITEM_MOD_CRIT_RATING,
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
        ITEM_MOD_HIT_RATING,
        ITEM_MOD_HASTE_RATING,
        ITEM_MOD_CRIT_RATING,
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
        ITEM_MOD_HIT_RATING,
        ITEM_MOD_CRIT_RATING,
        ITEM_MOD_HASTE_RATING,
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
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_BLOCK_RATING,
        ITEM_MOD_BLOCK_VALUE
    };
    stat_group_sockets[STAT_GROUP_STR_TANK] = {
        SOCKET_COLOR_RED
    };
    // Agi DPS Data
    stat_group_primary_stats[STAT_GROUP_AGI_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY
    };
    stat_group_secondary_stats[STAT_GROUP_AGI_DPS] = {
        ITEM_MOD_HIT_RATING,
        ITEM_MOD_CRIT_RATING,
        ITEM_MOD_HASTE_RATING,
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
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_BLOCK_RATING,
        ITEM_MOD_BLOCK_VALUE
    };
    stat_group_sockets[STAT_GROUP_AGI_TANK] = {
        SOCKET_COLOR_YELLOW
    };
    // Stat group for all stats
    stat_group_primary_stats[STAT_GROUP_ALL] = {
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
        ITEM_MOD_SPELL_PENETRATION,
        ITEM_MOD_HIT_RANGED_RATING,
        ITEM_MOD_CRIT_RANGED_RATING,
        ITEM_MOD_HASTE_RANGED_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_RANGED_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_secondary_stats[STAT_GROUP_ALL] = {
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
        ITEM_MOD_SPELL_PENETRATION,
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

    //preference stat groups
    preference_stat_groups[PREF_TANK] = {
        STAT_GROUP_AGI_TANK,
        STAT_GROUP_STR_TANK
    };
    preference_stat_groups[PREF_HEALER] = {
        STAT_GROUP_HEALING
    };
    preference_stat_groups[PREF_DPS_INT] = {
        STAT_GROUP_INT_DPS
    };
    preference_stat_groups[PREF_DPS_STR] = {
        STAT_GROUP_STR_DPS
    };
    preference_stat_groups[PREF_DPS_AGI] = {
        STAT_GROUP_AGI_DPS
    };
}

VirtualItemMgr::StatGroupData const VirtualItemMgr::premadeStatGroupData;

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
