/*
* Copyright (C) 2010 - 2016 Eluna Lua Engine <http://emudevs.com/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GROUPMETHODS_H
#define GROUPMETHODS_H

#include "AffixMgr.h"
#include "lua.h"

/***
 * Inherits all methods from: none
 */
namespace LuaGroup
{
    /**
     * Returns 'true' if the [Player] is the [Group] leader
     *
     * @param uint64 guid : guid of a possible leader
     * @return bool isLeader
     */
    int IsLeader(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        Eluna::Push(E->L, group->IsLeader(ObjectGuid(guid)));
        return 1;
    }

    /**
     * Returns 'true' if the [Group] is full
     *
     * @return bool isFull
     */
    int IsFull(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->IsFull());
        return 1;
    }

    /**
     * Returns 'true' if the [Group] is a raid [Group]
     *
     * @return bool isRaid
     */
    int IsRaidGroup(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->isRaidGroup());
        return 1;
    }

    /**
     * Returns 'true' if the [Group] is a battleground [Group]
     *
     * @return bool isBG
     */
    int IsBGGroup(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->isBGGroup());
        return 1;
    }

    /**
     * Returns 'true' if the [Player] is a member of this [Group]
     *
     * @param uint64 guid : guid of a player
     * @return bool isMember
     */
    int IsMember(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        Eluna::Push(E->L, group->IsMember(ObjectGuid(guid)));
        return 1;
    }

    /**
     * Returns 'true' if the [Player] is an assistant of this [Group]
     *
     * @param uint64 guid : guid of a player
     * @return bool isAssistant
     */
    int IsAssistant(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        Eluna::Push(E->L, group->IsAssistant(ObjectGuid(guid)));
        return 1;
    }

    /**
     * Returns 'true' if the [Player]s are in the same subgroup in this [Group]
     *
     * @param [Player] player1 : first [Player] to check
     * @param [Player] player2 : second [Player] to check
     * @return bool sameSubGroup
     */
    int SameSubGroup(Eluna* E, Group* group)
    {
        Player* player1 = Eluna::CHECKOBJ<Player>(E->L, 2);
        Player* player2 = Eluna::CHECKOBJ<Player>(E->L, 3);
        Eluna::Push(E->L, group->SameSubGroup(player1, player2));
        return 1;
    }

    /**
     * Returns 'true' if the subgroup has free slots in this [Group]
     *
     * @param uint8 subGroup : subGroup ID to check
     * @return bool hasFreeSlot
     */
    int HasFreeSlotSubGroup(Eluna* E, Group* group)
    {
        uint8 subGroup = Eluna::CHECKVAL<uint8>(E->L, 2);

        if (subGroup >= MAX_RAID_SUBGROUPS)
        {
            luaL_argerror(E->L, 2, "valid subGroup ID expected");
            return 0;
        }

        Eluna::Push(E->L, group->HasFreeSlotSubGroup(subGroup));
        return 1;
    }

    /**
     * Adds a new member to the [Group]
     *
     * @param [Player] player : [Player] to add to the group
     * @return bool added : true if member was added
     */
    int AddMember(Eluna* E, Group* group)
    {
        Player* player = Eluna::CHECKOBJ<Player>(E->L, 2);

        if (player->GetGroup() || !group->IsCreated() || group->IsFull())
        {
            Eluna::Push(E->L, false);
            return 1;
        }

        if (player->GetGroupInvite())
            player->UninviteFromGroup();

#if defined TRINITY || AZEROTHCORE
        bool success = group->AddMember(player);
        if (success)
            group->BroadcastGroupUpdate();
#else
        bool success = group->AddMember(player->GetObjectGuid(), player->GetName());
#endif

        Eluna::Push(E->L, success);
        return 1;
    }

    /*int IsLFGGroup(Eluna* E, Group* group) // TODO: Implementation
    {
        Eluna::Push(E->L, group->isLFGGroup());
        return 1;
    }*/

    /*int IsBFGroup(Eluna* E, Group* group) // TODO: Implementation
    {
        Eluna::Push(E->L, group->isBFGroup());
        return 1;
    }*/

    /**
     * Returns a table with the [Player]s in this [Group]
     *
     * @return table groupPlayers : table of [Player]s
     */
    int GetMembers(Eluna* E, Group* group)
    {
        lua_newtable(E->L);
        int tbl = lua_gettop(E->L);
        uint32 i = 0;

        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
#if defined TRINITY || AZEROTHCORE
            Player* member = itr->GetSource();
#else
            Player* member = itr->getSource();
#endif

            if (!member || !member->GetSession())
                continue;

            Eluna::Push(E->L, member);
            lua_rawseti(E->L, tbl, ++i);
        }

        lua_settop(E->L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns [Group] leader GUID
     *
     * @return uint64 leaderGUID
     */
    int GetLeaderGUID(Eluna* E, Group* group)
    {
#if defined TRINITY || AZEROTHCORE
        Eluna::Push(E->L, group->GetLeaderGUID());
#else
        Eluna::Push(E->L, group->GetLeaderGuid());
#endif
        return 1;
    }

    /**
     * Returns the [Group]'s GUID
     *
     * @return uint64 groupGUID
     */
    int GetGUID(Eluna* E, Group* group)
    {
#ifdef CLASSIC
        Eluna::Push(E->L, group->GetId());
#else
        Eluna::Push(E->L, group->GET_GUID());
#endif
        return 1;
    }

    /**
     * Returns a [Group] member's GUID by their name
     *
     * @param string name : the [Player]'s name
     * @return uint64 memberGUID
     */
    int GetMemberGUID(Eluna* E, Group* group)
    {
        const char* name = Eluna::CHECKVAL<const char*>(E->L, 2);
#if defined TRINITY || AZEROTHCORE
        Eluna::Push(E->L, group->GetMemberGUID(name));
#else
        Eluna::Push(E->L, group->GetMemberGuid(name));
#endif
        return 1;
    }

    /**
     * Returns the member count of this [Group]
     *
     * @return uint32 memberCount
     */
    int GetMembersCount(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->GetMembersCount());
        return 1;
    }

    /**
     * Returns the [Player]'s subgroup ID of this [Group]
     *
     * @param uint64 guid : guid of the player
     * @return uint8 subGroupID : a valid subgroup ID or MAX_RAID_SUBGROUPS+1
     */
    int GetMemberGroup(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        Eluna::Push(E->L, group->GetMemberGroup(ObjectGuid(guid)));
        return 1;
    }

    /**
     * Sets the leader of this [Group]
     *
     * @param uint64 guid : guid of the new leader
     */
    int SetLeader(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        group->ChangeLeader(ObjectGuid(guid));
        group->SendUpdate();
        return 0;
    }

    /**
     * Sends a specified [WorldPacket] to this [Group]
     *
     * @param [WorldPacket] packet : the [WorldPacket] to send
     * @param bool ignorePlayersInBg : ignores [Player]s in a battleground
     * @param uint64 ignore : ignore a [Player] by their GUID
     */
    int SendPacket(Eluna* E, Group* group)
    {
        WorldPacket* data = Eluna::CHECKOBJ<WorldPacket>(E->L, 2);
        bool ignorePlayersInBg = Eluna::CHECKVAL<bool>(E->L, 3);
        uint64 ignore = Eluna::CHECKVAL<uint64>(E->L, 4);

#ifdef CMANGOS
        group->BroadcastPacket(*data, ignorePlayersInBg, -1, ObjectGuid(ignore));
#else
        group->BroadcastPacket(data, ignorePlayersInBg, -1, ObjectGuid(ignore));
#endif
        return 0;
    }

    /**
     * Removes a [Player] from this [Group] and returns 'true' if successful
     *
     * <pre>
     * enum RemoveMethod
     * {
     *     GROUP_REMOVEMETHOD_DEFAULT  = 0,
     *     GROUP_REMOVEMETHOD_KICK     = 1,
     *     GROUP_REMOVEMETHOD_LEAVE    = 2,
     *     GROUP_REMOVEMETHOD_KICK_LFG = 3
     * };
     * </pre>
     *
     * @param uint64 guid : guid of the player to remove
     * @param [RemoveMethod] method : method used to remove the player
     * @return bool removed
     */
    int RemoveMember(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        uint32 method = Eluna::CHECKVAL<uint32>(E->L, 3, 0);

#if defined TRINITY || AZEROTHCORE
        Eluna::Push(E->L, group->RemoveMember(ObjectGuid(guid), (RemoveMethod)method));
#else
        Eluna::Push(E->L, group->RemoveMember(ObjectGuid(guid), method));
#endif
        return 1;
    }

    /**
     * Disbands this [Group]
     *
     */
    int Disband(Eluna* /*E*/, Group* group)
    {
        group->Disband();
        return 0;
    }

    /**
     * Converts this [Group] to a raid [Group]
     *
     */
    int ConvertToRaid(Eluna* /*E*/, Group* group)
    {
        group->ConvertToRaid();
        return 0;
    }

    /**
     * Sets the member's subGroup
     *
     * @param uint64 guid : guid of the player to move
     * @param uint8 groupID : the subGroup's ID
     */
    int SetMembersGroup(Eluna* E, Group* group)
    {
        uint64 guid = Eluna::CHECKVAL<uint64>(E->L, 2);
        uint8 subGroup = Eluna::CHECKVAL<uint8>(E->L, 3);

        if (subGroup >= MAX_RAID_SUBGROUPS)
        {
            luaL_argerror(E->L, 3, "valid subGroup ID expected");
            return 0;
        }

        if (!group->HasFreeSlotSubGroup(subGroup))
            return 0;

        group->ChangeMembersGroup(ObjectGuid(guid), subGroup);
        return 0;
    }

    /**
     * Sets the target icon of an object for the [Group]
     *
     * @param uint8 icon : the icon (Skull, Square, etc)
     * @param uint64 target : GUID of the icon target, 0 is to clear the icon
     * @param uint64 setter : GUID of the icon setter
     */
    int SetTargetIcon(Eluna* E, Group* group)
    {
        uint8 icon = Eluna::CHECKVAL<uint8>(E->L, 2);
        uint64 target = Eluna::CHECKVAL<uint64>(E->L, 3);
        uint64 setter = Eluna::CHECKVAL<uint64>(E->L, 4, 0);

        if (icon >= TARGETICONCOUNT)
            return luaL_argerror(E->L, 2, "valid target icon expected");

#if (defined(CLASSIC) || defined(TBC))
        group->SetTargetIcon(icon, ObjectGuid(target));
#else
        group->SetTargetIcon(icon, ObjectGuid(setter), ObjectGuid(target));
#endif
        return 0;
    }

    /*int ConvertToLFG(Eluna* E, Group* group) // TODO: Implementation
    {
        group->ConvertToLFG();
        return 0;
    }*/

    int GetDungeonLevel(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->GetDungeonLevel());
        return 1;
    }

    int GetCappedDungeonLevel(Eluna* E, Group* group)
    {
        Eluna::Push(E->L, group->GetCappedDungeonLevel());
        return 1;
    }

    int GetAffixGroup(Eluna* E, Group* group)
    {
        AffixGroup affixGroup = sAffixMgr->GetAffixGroup(group);
        int numSlots = 4;

        lua_createtable(E->L, numSlots, 0);
        int tbl = lua_gettop(E->L);
        for (int i = 1; i <= numSlots; ++i)
        {
            lua_createtable(E->L, 3, 0); // 3 being the number of values in the inner table
            int subtable = lua_gettop(E->L);

            lua_pushnumber(E->L, affixGroup.GetAffixId(i));
            lua_rawseti(E->L, subtable, 1);

            lua_pushnumber(E->L, affixGroup.GetNumRolls(i));
            lua_rawseti(E->L, subtable, 2);

            lua_pushnumber(E->L, affixGroup.GetRank(i));
            lua_rawseti(E->L, subtable, 3);

            lua_rawseti(E->L, tbl, i);
        }
        lua_settop(E->L, tbl);
        return 1;
    }

    int SetAffixSlot(Eluna* E, Group* group)
    {
        uint8 slot = Eluna::CHECKVAL<uint8>(E->L, 2);
        uint32 affixId = Eluna::CHECKVAL<uint32>(E->L, 3);
        int rolls = Eluna::CHECKVAL<int>(E->L, 4);
        uint8 rank = Eluna::CHECKVAL<uint8>(E->L, 5);
        sAffixMgr->GetAffixGroup(group).SetSlot(slot, affixId, rolls, rank);
        return 0;
    }
};

#endif
