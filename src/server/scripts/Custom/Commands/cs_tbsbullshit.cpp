#include "Chat.h"
#include "GameObject.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Language.h"
#include "LootMgr.h"
#include "Map.h"
#include "MapManager.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Position.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
#include "TemporarySummon.h"
#include "World.h"
#include "VirtualItemMgr.h"
#include "Item.h"
#include "SmartEnum.h"

using namespace Trinity::ChatCommands;

class tbsbullshit_commandscript : public CommandScript
{
public:
    tbsbullshit_commandscript() : CommandScript("tbsbullshit_commandscript")
    {
    }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> tbsBullshitCommandTable =
        {
            { "circlerlaser", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleCirclerLaserCommand, "" },
            { "clone", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleClonePlayerCommand, "" },
            { "clearinventory", rbac::RBAC_PERM_COMMAND_ADDITEM, false, &HandleClearInventory, "" },
            { "knockback", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleKnockbackCommand, "" },
            { "cheatspells", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleToggleCheatSpells, "" },
            { "debugstats", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleDebugStatPrint, "" },
            { "settalentloadout", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleDebugSetTalentLoadout, "" },
            { "learncustomtalent", rbac::RBAC_PERM_COMMAND_DEV, false, &HandleDebugLearnTalent, "" },
            { "addvitem", rbac::RBAC_PERM_COMMAND_ADDITEM, false, &HandleAddVirtualItem, "" },

        };
        return tbsBullshitCommandTable;
    }

    static bool HandleCirclerLaserCommand(ChatHandler* handler, float radius, uint8 step)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (Creature* master = player->SummonCreature(82001, player->GetPosition(), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10*60s))
        {
            if (Creature* slave = player->SummonCreature(82001, master->GetRandomNearPosition(5.0f), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10*60s))
            {
                master->CastSpell(slave, 82000, true);
                slave->GetMotionMaster()->MoveCirclePath(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), radius, roll_chance_f(50.f), step);
            }
        }
        return true;
    }

    static bool HandleClearInventory(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        player->ClearInventory();
        handler->PSendSysMessage("Inventory cleared.");
        return true;
    }

    static bool HandleClonePlayerCommand(ChatHandler* handler, char const* args)
    {
        uint32 numberOfClones = 1;
        if (*args)
            numberOfClones = atoi(args);
        Player* target = handler->getSelectedPlayerOrSelf();
        Player* player = handler->GetSession()->GetPlayer();
        uint32 targetDisplay = target->GetDisplayId();
        uint32 faction = player->GetFaction();
        uint32 iLvl = player->GetAverageItemLevel();

        if (!sWorld->getBoolConfig(CONFIG_ALLOW_DEVELOPMENT) && numberOfClones > 10)
            numberOfClones = 10;
        //@todo check current amount of summoned clones and limit.

        for (uint32 i = 0; i < numberOfClones; ++i)
        {
            if (TempSummon* clone = player->GetMap()->SummonCreature(82002, player->GetRandomNearPosition(5.f), sSummonPropertiesStore.LookupEntry(1021), 10 * MINUTE * IN_MILLISECONDS, player, 0, 0, iLvl))
            {
                clone->SetDisplayId(targetDisplay);
                clone->SetFaction(faction);
                target->CastSpell(clone, 45204, true);
                ((Minion*)clone)->SetFollowAngle(player->GetAbsoluteAngle(clone));
            }

        }
        return true;
    }

    static bool HandleKnockbackCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* pSpeedX = strtok((char*)args, " ");
        if (!pSpeedX)
            return false;

        char* pSpeedY = strtok(nullptr, " ");
        if (!pSpeedY)
            return false;

        float speedX = atof(pSpeedX);
        float speedY = atof(pSpeedY);

        handler->getSelectedUnit()->KnockbackFrom(handler->GetSession()->GetPlayer()->GetPositionX(), handler->GetSession()->GetPlayer()->GetPositionY(), speedX, speedY);
        return true;
    }

    static bool HandleToggleCheatSpells(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if(handler->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR)
            player = handler->getSelectedPlayerOrSelf();
        player->ToggleFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHEAT_SPELLS);
        handler->PSendSysMessage("Cheat spells %s on %s.", player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHEAT_SPELLS) ? "enabled" : "disabled", player->GetName().c_str());
        return true;
    }

    static bool HandleDebugStatPrint(ChatHandler* handler)
    {
        Player* player = handler->getSelectedPlayerOrSelf();
        handler->PSendSysMessage("Magic Find %u", player->GetMagicFind());
        for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        {
            handler->PSendSysMessage("Bonus Spell School Damage Pct %u, %f", i, player->GetBonusSchoolModifierPct(SpellSchools(i)));
        }
        return true;
    }

    static bool HandleDebugSetTalentLoadout(ChatHandler* handler, char const* args)
    {
        uint32 loadout = 1;
        if (*args)
            loadout = atoi(args);

        if (loadout >= MAX_CUSTOM_TALENT_LOADOUTS)
            loadout = MAX_CUSTOM_TALENT_LOADOUTS - 1;

        Player * p = handler->getSelectedPlayerOrSelf();
        p->SetTalentLoadout(loadout);
        return true;
    }

    static bool HandleDebugLearnTalent(ChatHandler* handler, char const* args)
    {
        if (!args)
            return false;

        Player* p = handler->getSelectedPlayerOrSelf();

        if (std::string((char*)args) == "all")
        {
            p = handler->GetSession()->GetPlayer(); // we are not supporting learn all on remote players.
            for (auto const& itr : sObjectMgr->GetTalentNodeStore())
            {
                p->LearnCustomTalent(itr.first);
                handler->PSendSysMessage("Learned node %u", itr.first);
            }

        }
        else
        {
            uint32 nodeEntry = atoi(args);
            if (sObjectMgr->GetTalentNode(nodeEntry))
            {
                p->LearnCustomTalent(nodeEntry);
                handler->PSendSysMessage("Learned node %u", nodeEntry);
            }
            else
                handler->PSendSysMessage("Invalid node %u", nodeEntry);
        }
        return true;
    }

    static bool HandleAddVirtualItem(ChatHandler* handler, uint32 itemEntry, Optional<uint8> quality, Optional<uint32> itemLevel, Optional<uint32> seed, Optional<int8> statGroup, Optional<bool> isCrafted, Optional<bool> generateSet)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        if (!sVirtualItemMgr.IsVirtualTemplate(sObjectMgr->GetItemTemplate(itemEntry)))
        {
            handler->PSendSysMessage("%u isn't a valid virtual item.", itemEntry);
            return true;
        }
        VirtualModifier mod;

        if (quality)
        {
            mod.quality = *quality;
            handler->PSendSysMessage(" Quality = %u", mod.quality);
        }

        if (itemLevel)
        {
            mod.ilevel = *itemLevel;
            handler->PSendSysMessage(" ItemLevel = %u", mod.ilevel);
        }

        if (seed)
        {
            mod.seed = *seed;
            handler->PSendSysMessage(" Seed = %u", mod.seed);
        }

        if (statGroup && statGroup < STAT_GROUP_COUNT)
        {
            mod.statgroup = StatGroup(*statGroup);
            handler->PSendSysMessage(" StatGroup = %u", mod.statgroup);
        }

        if (generateSet)
        {
            mod.generateSet = *generateSet;
            handler->PSendSysMessage(" GenerateSet = %u", mod.generateSet);
        }

        if (isCrafted)
        {
            mod.isCrafted = *isCrafted;
            mod.lowYield = *isCrafted;
            handler->PSendSysMessage(" isCrafted = true");
        }

        uint8 itemCount = 1;
        uint32 noSpaceForCount = 0;
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemEntry, itemCount, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)
            itemCount -= noSpaceForCount;

        if (itemCount == 0 || dest.empty())
            return 1;

        Item* item = player->StoreNewItem3(dest, itemEntry, true, GenerateItemRandomPropertyId(itemEntry), GuidSet(), mod);
        if (item)
        {
            item->SetGuidValue(ITEM_FIELD_CREATOR, ObjectGuid(HighGuid::Player, uint32(2)));

            player->SendNewItem(item, itemCount, true, false);

            handler->PSendSysMessage("Added virtual item");
        }
        else
            handler->PSendSysMessage("Error adding item %s", EnumUtils::ToString<InventoryResult>(msg).Constant);

        return true;
    }
};

void AddSC_TbsBullshit_commandscript()
{
    new tbsbullshit_commandscript();
}
