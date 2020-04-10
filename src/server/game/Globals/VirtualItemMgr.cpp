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

void VirtualItemTemplate::UpdateDisplay()
{
    // Get the correct display ID
    if (ItemEntry const* dbcitem = sItemStore.LookupEntry(ItemId))
        DisplayInfoID = dbcitem->DisplayId;
}

VirtualModifier::VirtualModifier() : ilevel(0), quality(MAX_ITEM_QUALITY), statpool(-1), statgroup(STAT_GROUP_RANDOM), seed(0)
{
}

float VirtualModifier::GetSlotStatModifier(InventoryType invtype)
{
    switch (invtype)
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
        case INVTYPE_FINGER:
        case INVTYPE_HOLDABLE:
        case INVTYPE_SHIELD:
            return 9.0f / 16.0f;

        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
            return 27.0f / 64.0f;

        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN:
            return 81.0f / 256.0f;

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
            return 0.14f;
        case ITEM_MOD_ATTACK_POWER:
            return 0.5f;
        case ITEM_MOD_SPELL_HEALING_DONE:
            return 0.45f;
        case ITEM_MOD_MANA_REGENERATION:
            return 2.0f;
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
        default:
            return 1.0f;
    }
    return 1.0f;
}

float VirtualModifier::GetTypeSlotArmorModifier(ItemSubclassArmor subclass, InventoryType invtype)
{
    switch (subclass)
    {
    case ITEM_SUBCLASS_ARMOR_CLOTH:
        switch (invtype)
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
        switch (invtype)
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
        switch (invtype)
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
        switch (invtype)
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

char* VirtualItemMgr::ConvertSeed(uint32 seed) const
{
    char cseed[11] = "";
    sprintf(cseed, "%u", seed);

    return cseed;
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
		int32 array_id = fields[2].GetInt32();
		std::string name = fields[3].GetString();

		availableNames.push_back(NameInfo(itemType, subclass, array_id, name));
		++count;
	} while (result->NextRow());

	TC_LOG_INFO("server.loading", "Loaded %u available virtual item names in %u MS.", count, GetMSTimeDiffToNow(beginTime));
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

		names.push_back(name.name);
	}
	return names;
}

std::string VirtualItemMgr::GenerateItemName(uint32 type, uint32 subclass, uint32 quality, char* seed) const
{
    std::string fullName = "";


    // TODO: Add armor case if this is something we need/want
    if (type == ITEM_CLASS_WEAPON)
    {
        // Some subclasses use the same database name lists.
        // No need to have duplicate db entries, so switch item subclass.
        if (subclass == ITEM_SUBCLASS_WEAPON_SWORD2)
            subclass = ITEM_SUBCLASS_WEAPON_SWORD;
        else if (subclass == ITEM_SUBCLASS_WEAPON_MACE2)
            subclass = ITEM_SUBCLASS_WEAPON_MACE;
        else if (subclass == ITEM_SUBCLASS_WEAPON_AXE2)
            subclass = ITEM_SUBCLASS_WEAPON_AXE;
        else if (subclass == ITEM_SUBCLASS_WEAPON_CROSSBOW)
            subclass = ITEM_SUBCLASS_WEAPON_BOW;

        // Retrieve all the string lists
        std::map<uint32, std::vector<std::string>> nameLists;
        for (size_t i = 1; i <= 7; ++i)
        {
            NameInfo nameInfo(type, subclass, i);
            auto list = GetNamesForNameInfo(&nameInfo);

            // Make sure the current list is not empty. If it is, fall back to template item name.
            if (list.empty())
                return fullName;

            nameLists.insert(std::make_pair(i, list));
        }

        std::stringstream ss;
        // Concat the correct full item name for the item quality
        switch (quality)
        {
            case ITEM_QUALITY_NORMAL:
            {
                ss << nameLists[4][urand(0, nameLists[4].size() - 1, seed)];
                break;
            }
            case ITEM_QUALITY_UNCOMMON:
            {
                ss << nameLists[3][urand(0, nameLists[3].size() - 1)] << " " << nameLists[4][urand(0, nameLists[4].size() - 1, seed)];
                break;
            }
            case ITEM_QUALITY_RARE:
            {
                ss << nameLists[2][urand(0, nameLists[2].size() - 1)] << " " << nameLists[4][urand(0, nameLists[4].size() - 1, seed)];
                break;
            }
            case ITEM_QUALITY_EPIC:
            {
                ss << nameLists[1][urand(0, nameLists[1].size() - 1, seed)];
                break;
            }
            case ITEM_QUALITY_LEGENDARY:
            {
                ss << nameLists[7][urand(0, nameLists[7].size() - 1)] << ", " << nameLists[5][urand(0, nameLists[5].size() - 1)] << " " << nameLists[6][urand(0, nameLists[6].size() - 1, seed)];
                break;
            }
            default:
                return fullName;
        }
        fullName = ss.str();
    }

    return fullName;
}

std::list<uint32> VirtualItemMgr::GetDisplaysForDisplayInfo(displayInfo* info) const
{
    if (!info)
        return std::list<uint32>();

    std::list<uint32> displays;
    for (auto displaysitr : availableDisplays)
    {
        if (info->quality != displaysitr.quality)
            continue;

        if (info->iInventoryType != displaysitr.iInventoryType)
            continue;

        if (info->iClass != displaysitr.iClass)
            continue;

        if (info->isubClass != displaysitr.isubClass)
            continue;

        displays.push_back(displaysitr.displayId);
    }
    return displays;
}

uint32 VirtualItemMgr::GenerateItemDisplay(uint32 quality, uint32 _class, uint32 subclass, uint32 inventoryType, char* seed) const
{
    std::list<uint32> displayLists;
    displayInfo dInfo(quality, _class, subclass, inventoryType);
    displayLists = GetDisplaysForDisplayInfo(&dInfo);

    if (displayLists.empty())
        return 0;
    auto display = std::begin(displayLists);
    std::advance(display, urand(0, uint32(std::size(displayLists)) - 1, seed));
    return *display;
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

VirtualItemTemplate const* VirtualItemMgr::GetVirtualTemplate(uint32 entry)
{
    if (entry < minEntry || entry >= maxEntry)
        return nullptr;
    ReadGuard guard(lock);
    auto it = store.find(entry);
    if (it != store.end())
        return it->second;
    return nullptr;
}

VirtualItemTemplate* VirtualItemMgr::GenerateVirtualTemplate(ItemTemplate const* base, VirtualModifier modifier)
{
    if (!base)
        return nullptr;

    // If no seed supplied, generate a new seed.
    if (modifier.seed == 0)
    {
        SFMTRand sfmt;
        modifier.seed = sfmt.RandomUInt32();
    }

    char* seed = ConvertSeed(modifier.seed);

    VirtualItemTemplate* temp = new VirtualItemTemplate(base);
    GenerateStats(temp, modifier);

    WriteGuard guard(lock);
    EntryGenerator* generator = Generator(temp);
    if (!generator)
        return nullptr;

    uint32 entry = generator->GenerateEntry(store);
    temp->seed = modifier.seed;
    temp->ItemId = entry;
    uint32 display = GenerateItemDisplay(temp->Quality, temp->Class, temp->SubClass, temp->InventoryType, seed);
    if (display == 0)
        temp->UpdateDisplay();
    else
        temp->DisplayInfoID = display;

    delete store[entry];
    store[entry] = temp;

    if(sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
        temp->InitializeQueryData();

    return temp;
}

void VirtualItemMgr::GenerateStats(ItemTemplate* output, VirtualModifier modifier) const
{
    // decide quality
    uint32 quality = output->Quality;

    char* seed = ConvertSeed(modifier.seed);

    if (modifier.quality < MAX_ITEM_QUALITY)
        quality = modifier.quality;
    else
    {
        quality = output->Quality;

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
            uint32 rand = urand(1, sum, seed);
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
    }
    ASSERT(quality < MAX_ITEM_QUALITY);

    // always bind on pickup
    output->Bonding = 1;

    // decide stat amount
    uint32 statscount = quality;
    if (statscount < 0)
        statscount = 0;
    ASSERT(statscount <= MAX_ITEM_PROTO_STATS);

    // decide itemlevel
    uint32 ilevel = output->ItemLevel;
    if (modifier.ilevel)
        ilevel = modifier.ilevel;
    else
        ilevel = output->ItemLevel + ((int32(quality) - int32(output->Quality)) * 5);

    output->ItemLevel = ilevel;

    // decide armor, if item class is armor, armor should always be applied.
    uint32 armor = 0;
    if (output->Class == ITEM_CLASS_ARMOR)
    {
        // by default we assume 1 ilevel = 1 armor
        armor = 1*ilevel;

        // retrieve armor slot and type multiplier
        float typeslotmod = VirtualModifier::GetTypeSlotArmorModifier((ItemSubclassArmor)output->SubClass, (InventoryType)output->InventoryType);
        armor = armor * typeslotmod;

        // depending on quality, add multiplier to armor piece
        float qmulti = ((int32(quality) - int32(ITEM_QUALITY_NORMAL)) / 10.0f) + 1.0f;
        armor = armor * qmulti;

        // add a random 10% increase or decrease of stats
        float randmulti = (urand(90, 110, seed) / 100.0f);
        armor = armor * randmulti;
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
        pool = ilevel;
    else
        pool = modifier.statpool;
    ASSERT(pool >= 0 && pool < 0x7FFF);

    // modify stat pool size depending on item quality
    pool = (pool * quality) / 2;

    // select stat group
    StatGroup statgroupid = modifier.statgroup;
    if (modifier.statgroup == STAT_GROUP_RANDOM && output->Class == ITEM_CLASS_ARMOR)
    {
        std::vector<StatGroup> const& statgroups = modifier.premadeStatGroupData.GetArmorSubclassStatGroups((ItemSubclassArmor)output->SubClass);
        if (!statgroups.empty())
            statgroupid = statgroups[urand(0, statgroups.size() - 1, seed)];
    }
    if (statgroupid == STAT_GROUP_RANDOM)
        statgroupid = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, seed));
    ASSERT(statgroupid < STAT_GROUP_COUNT); // must not be random anymore
    std::vector<ItemModType> const& statgroup = modifier.premadeStatGroupData.GetStatGroupStats(statgroupid, seed);

    std::vector<ItemModType> selectedStats;
    std::vector<int16> distributedPool;
    if (statscount && !statgroup.empty())
    {
        // select stats from preselected stat group
        for (uint32 i = 0; i < statscount; ++i)
            selectedStats.push_back(statgroup[urand(0, statgroup.size() - 1, seed)]);

        // distribute pool to stats
        const float mineachpct = 0.5f / selectedStats.size();
        ASSERT(mineachpct <= 1.0f / selectedStats.size() && mineachpct >= 0.0);

        // calculate min amount and take that from the randomly distributed pool
        int16 min_amount = std::floor(pool*mineachpct);
        int16 workpool = pool - selectedStats.size()*min_amount;

        // pick random positions from the workpool and use them to divide it into N random size parts
        // then add those to distributedPool along with the minimum amounts
        std::vector<int16> fences;
        fences.push_back(0);
        for (int32 i = 1; i < int32(selectedStats.size()); ++i)
            fences.push_back(urand(0, workpool, seed));
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
                uint32 finalStatValue = std::floor(distributedPool[i] / VirtualModifier::GetStatRate(selectedStats[i]) * VirtualModifier::GetSlotStatModifier((InventoryType)output->InventoryType));
                output->ItemStat[j].ItemStatType = selectedStats[i];
                output->ItemStat[j].ItemStatValue += finalStatValue;
                setStats = std::max(setStats, uint32(j + 1));
                break;
            }
        }
    }
    statscount = setStats;

    // set socket colors
    std::vector<SocketColor> const& socketcolors = modifier.premadeStatGroupData.GetStatGroupSockets(statgroupid, seed);
    if (!socketcolors.empty())
    {
        for (int32 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            if (!output->Socket[i].Color)
                continue;
            if (output->Socket[i].Color != SOCKET_COLOR_RED &&
                output->Socket[i].Color != SOCKET_COLOR_BLUE &&
                output->Socket[i].Color != SOCKET_COLOR_YELLOW)
                continue;
            output->Socket[i].Color = socketcolors[urand(0, socketcolors.size() - 1, seed)];
        }
    }

    // Hardcode disenchant ID
    switch (quality) {
    case ITEM_QUALITY_LEGENDARY:
        output->DisenchantID = 60004;
        break;
    case ITEM_QUALITY_EPIC:
        output->DisenchantID = 60003;
        break;
    case ITEM_QUALITY_RARE:
        output->DisenchantID = 60002;
        break;
    case ITEM_QUALITY_UNCOMMON:
        output->DisenchantID = 60001;
        break;
    case ITEM_QUALITY_NORMAL:
        output->DisenchantID = 60000;
        break;
    }

    // Generate random item name
    std::string name = GenerateItemName(output->Class, output->SubClass, quality, seed);
    if (!name.empty())
        output->Name1 = name;

    // apply other item data
    output->Quality = quality;
    output->StatsCount = statscount; // remember to modify in stat generation if two same stats are picked
    output->ItemLevel = ilevel;
    output->Armor = armor;
    output->ItemSet = 0; // Temporary default to set 0, ie. no set. Need to add set handler based on stat groups.
}

bool VirtualItemMgr::IsVirtualTemplate(ItemTemplate const * base)
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

VirtualModifier::StatGroupData::StatGroupData()
{
    stat_group_stats[STAT_GROUP_HEALING] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_SPIRIT,
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_MANA_REGENERATION,
        ITEM_MOD_SPELL_POWER
    };
    stat_group_stats[STAT_GROUP_INT_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_INTELLECT,
        ITEM_MOD_HIT_SPELL_RATING,
        ITEM_MOD_HASTE_SPELL_RATING,
        ITEM_MOD_CRIT_SPELL_RATING,
        ITEM_MOD_MANA_REGENERATION,
        ITEM_MOD_SPELL_POWER,
        ITEM_MOD_SPELL_PENETRATION
    };
    stat_group_stats[STAT_GROUP_STR_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_STRENGTH,
        ITEM_MOD_HIT_MELEE_RATING,
        ITEM_MOD_CRIT_MELEE_RATING,
        ITEM_MOD_HASTE_MELEE_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_stats[STAT_GROUP_STR_TANK] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_STRENGTH,
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_RATING
    };
    stat_group_stats[STAT_GROUP_AGI_DPS] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_HIT_MELEE_RATING,
        ITEM_MOD_CRIT_MELEE_RATING,
        ITEM_MOD_HASTE_MELEE_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };
    stat_group_stats[STAT_GROUP_AGI_TANK] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_DEFENSE_SKILL_RATING,
        ITEM_MOD_DODGE_RATING,
        ITEM_MOD_PARRY_RATING,
        ITEM_MOD_HIT_RATING
    };
    stat_group_stats[STAT_GROUP_AGI_RANGED] = {
        ITEM_MOD_STAMINA,
        ITEM_MOD_AGILITY,
        ITEM_MOD_HIT_RANGED_RATING,
        ITEM_MOD_CRIT_RANGED_RATING,
        ITEM_MOD_HASTE_RANGED_RATING,
        ITEM_MOD_EXPERTISE_RATING,
        ITEM_MOD_RANGED_ATTACK_POWER,
        ITEM_MOD_ARMOR_PENETRATION_RATING
    };

    // socket groups
    stat_group_sockets[STAT_GROUP_HEALING] = {
        SOCKET_COLOR_BLUE
    };

    stat_group_sockets[STAT_GROUP_INT_DPS] = {
        SOCKET_COLOR_BLUE
    };

    stat_group_sockets[STAT_GROUP_STR_DPS] = {
        SOCKET_COLOR_YELLOW
    };

    stat_group_sockets[STAT_GROUP_STR_TANK] = {
        SOCKET_COLOR_RED
    };

    stat_group_sockets[STAT_GROUP_AGI_DPS] = {
        SOCKET_COLOR_YELLOW
    };

    stat_group_sockets[STAT_GROUP_AGI_TANK] = {
        SOCKET_COLOR_RED
    };

    stat_group_sockets[STAT_GROUP_AGI_RANGED] = {
        SOCKET_COLOR_YELLOW
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

std::vector<ItemModType> const & VirtualModifier::StatGroupData::GetStatGroupStats(StatGroup group, char* seed) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, seed));
    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_stats[group];
}

std::vector<SocketColor> const & VirtualModifier::StatGroupData::GetStatGroupSockets(StatGroup group, char* seed) const
{
    if (group == STAT_GROUP_RANDOM)
        group = static_cast<StatGroup>(urand(0, STAT_GROUP_COUNT - 1, seed));
    ASSERT(group < STAT_GROUP_COUNT);

    return stat_group_sockets[group];
}

std::vector<StatGroup> const & VirtualModifier::StatGroupData::GetArmorSubclassStatGroups(ItemSubclassArmor subclass) const
{
    return armor_type_stat_groups[subclass];
}
