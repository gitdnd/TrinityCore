#include "Chat.h"
#include "GameObject.h"
#include "Language.h"
#include "Map.h"
#include "MapManager.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Position.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "WorldSession.h"

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
        };
        return tbsBullshitCommandTable;
    }

    static bool HandleCirclerLaserCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        char* radius_str = strtok((char*)args, " ");
        char* step_str = args ? strtok(nullptr, " ") : "8";
        float radius = atof(radius_str);
        uint8 step = atoi(step_str);
        if (Creature* master = player->SummonCreature(82001, player->GetPosition(), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10 * MINUTE * IN_MILLISECONDS))
        {
            if (Creature* slave = player->SummonCreature(82001, master->GetRandomNearPosition(5.0f), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10 * MINUTE * IN_MILLISECONDS))
            {
                master->CastSpell(slave, 82000, true);
                slave->GetMotionMaster()->MoveCirclePath(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), radius, roll_chance_f(50.f), step);
            }
        }
        return true;
    }

    static bool HandleClearInventory(ChatHandler* handler, char const* args)
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
        for (uint32 i = 0; i < numberOfClones; ++i)
        {
            if (Creature* clone = player->SummonCreature(82002, player->GetRandomNearPosition(5.f), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10 * MINUTE * IN_MILLISECONDS))
            {
                target->CastSpell(clone, 45204, true);
                clone->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, player->GetFollowAngle());
            }

        }
        return true;
    }
};

void AddSC_TbsBullshit_commandscript()
{
    new tbsbullshit_commandscript();
}
