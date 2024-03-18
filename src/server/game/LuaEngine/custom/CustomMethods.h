#ifndef CUSTOMMETHODS_H
#define CUSTOMMETHODS_H

#include "BindingMap.h"
#include "lua.h"
#include "ElunaInstanceAI.h"
#include "ElunaTemplate.h"
#include "ElunaIncludes.h"

#include "AchievementMgr.h"
#include "LFGMgr.h"
#include "MiscPackets.h"
#include "LFG.h"
#include "TransportMgr.h"
#include "Transport.h"
#include "LootMgr.h"

namespace LuaCustom
{

    /**
    * Sets the [Creature]'s ReactState to `state`.
    *
    * @param [ReactState] state
    */
    int SetReactState(Eluna* E, Creature* creature)
    {
        uint32 state = E->CHECKVAL<uint32>(2);

        creature->SetReactState((ReactStates)state);
        return 0;
    }

    /**
     * Sets the [Creature]s waypoint path
     *
     * @param uint32 pathid : entry of a path
     */
    int SetWaypoint(Eluna* E, Creature* creature)
    {
        uint32 pathid = E->CHECKVAL<uint32>(2);
        if (creature->GetEntry() == 1)
            return 1;

        //creature->LoadPath(pathid);
        //creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        creature->GetMotionMaster()->MovePath(pathid, true);
        //creature->GetMotionMaster()->Initialize();

        return 0;
    }
 
    /**
     * Sets whether the [Creature] can regenerate health or not
     *
     * @param bool disable = true : `true` to enable regenerate health, `false` to disable regenerate health
     */
    int SetRegenerateHealth(Eluna* E, Creature* creature)
    {
        bool disable = E->CHECKVAL<bool>(2);

        creature->SetRegenerateHealth(disable);

        return 0;
    }
 
    /**
     * Adds the given quest to the [Creature].
     *
     * @param uint32 entry : The quest ID to add
     */
    int AddQuest(Eluna* E, Creature* creature)
    {
        uint32 q_entry = E->CHECKVAL<uint32>(2);

        QuestRelations* CreatureQuestMap = sObjectMgr->GetCreatureQuestRelationMapHACK();
        CreatureQuestMap->insert(QuestRelations::value_type(creature->GetEntry(), q_entry));
        return 0;
    }

    /**
     * Removes the given quest on the [Creature].
     *
     * @param uint32 entry : The quest ID to remove
     */
    int RemoveQuest(Eluna* E, Creature* creature)
    {
        uint32 q_entry = E->CHECKVAL<uint32>(2);

        QuestRelations* CreatureQuestMap = sObjectMgr->GetCreatureQuestRelationMapHACK();

        // Remove the pair(id, quest) from the multimap
        QuestRelations::iterator qitr = CreatureQuestMap->find(creature->GetEntry());
        QuestRelations::iterator lastElement = CreatureQuestMap->upper_bound(creature->GetEntry());
        for (; qitr != lastElement; ++qitr)
        {
            if (qitr->second == q_entry)
            {
                CreatureQuestMap->erase(qitr);          // iterator is now no more valid
                break;                                  // but we can exit loop since the element is found
            }
        }

        return 0;
    }

    int MoveCircle(Eluna* E, Creature* creature)
    {
        float x = E->CHECKVAL<float>(2);
        float y = E->CHECKVAL<float>(3);
        float z = E->CHECKVAL<float>(4);
        float radius = E->CHECKVAL<float>(5);
        bool clockwise = E->CHECKVAL<bool>(6, true);
        uint8 stepCount = E->CHECKVAL<uint8>(7, 8);

        creature->GetMotionMaster()->MoveCirclePath(x, y, z, radius, clockwise, stepCount);

        return 0;
    }

    int ClearLoot(Eluna* /*E*/, Creature* creature)
    {
        creature->loot.clear();
        return 0;
    }

    int AnimateAndSetFlyMode(Eluna* E, Creature* creature)
    {
        float zOffset = E->CHECKVAL<float>(2, 17.0f);

        creature->SetCanFly(true);
        creature->SetDisableGravity(true);
        creature->SetAnimTier(AnimTier::Fly);
        creature->SetFacingTo(creature->GetOrientation() + float(M_PI));
        creature->AttackStop();
        Position pos;
        pos.Relocate(creature);
        pos.m_positionZ += zOffset;
        creature->GetMotionMaster()->MoveTakeoff(0, pos);
        return 0;
    }

    int AnimateAndSetLandMode(Eluna* E, Creature* creature)
    {
        float x = E->CHECKVAL<float>(2);
        float y = E->CHECKVAL<float>(3);
        float z = E->CHECKVAL<float>(4);

        creature->SetCanFly(false);
        creature->SetDisableGravity(false);

        creature->SetAnimTier(AnimTier::Ground);

        creature->GetMotionMaster()->MoveLand(0, Position(x, y, z, 0));
        return 0;
    }

    int SendMirrorToPlayer(Eluna* E, Creature* creature)
    {
        uint64 guid = E->CHECKVAL<uint64>(2);
        uint32 displayid = E->CHECKVAL<uint32>(3);
        uint8 race = E->CHECKVAL<uint32>(4);
        uint8 gender = E->CHECKVAL<uint32>(5);
        uint8 _class = E->CHECKVAL<uint32>(6);
        uint8 skin = E->CHECKVAL<uint32>(7);
        uint8 face = E->CHECKVAL<uint32>(8);
        uint8 hairstyle = E->CHECKVAL<uint32>(9);
        uint8 haircolor = E->CHECKVAL<uint32>(10);
        uint8 facialhair = E->CHECKVAL<uint32>(11);
        uint32 head = E->CHECKVAL<uint32>(12);
        uint32 shoulder = E->CHECKVAL<uint32>(13);
        uint32 body = E->CHECKVAL<uint32>(14);
        uint32 chest = E->CHECKVAL<uint32>(15);
        uint32 waist = E->CHECKVAL<uint32>(16);
        uint32 legs = E->CHECKVAL<uint32>(17);
        uint32 feet = E->CHECKVAL<uint32>(18);
        uint32 wrist = E->CHECKVAL<uint32>(19);
        uint32 hand = E->CHECKVAL<uint32>(20);
        uint32 back = E->CHECKVAL<uint32>(21);
        uint32 tabard = E->CHECKVAL<uint32>(22);
        Player* player = E->CHECKOBJ<Player>(23);
        creature->blockMirror = true;
        WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);
        data << uint64(guid);
        data << uint32(displayid);
        data << uint8(race);
        data << uint8(gender);
        data << uint8(_class);
        data << uint8(skin);
        data << uint8(face);
        data << uint8(hairstyle);
        data << uint8(haircolor);
        data << uint8(facialhair);
        data << uint32(0); // guild id probably will never be used
        data << uint32(head);
        data << uint32(shoulder);
        data << uint32(body);
        data << uint32(chest);
        data << uint32(waist);
        data << uint32(legs);
        data << uint32(feet);
        data << uint32(wrist);
        data << uint32(hand);
        data << uint32(back);
        data << uint32(tabard);
        player->GetSession()->SendPacket(&data);
        return 0;
    }

    int GetCustomTalentStorage(Eluna* E)
    {
        lua_State* L = E->L;
        lua_createtable(L, eObjectMgr->GetTalentNodeStore().size(), 0);

        int maintable = lua_gettop(L);

        for (auto const& itr : eObjectMgr->GetTalentNodeStore())
        {
            lua_createtable(L, 7, 0); // 8 being the number of values in the inner table
            int subtable = lua_gettop(L);

            // Push each value with correct method
            // and set them to table with rawseti to correct index
            lua_pushnumber(L, itr.second.spellId);
            lua_rawseti(L, subtable, 1);

            lua_pushnumber(L, itr.second.xOffset);
            lua_rawseti(L, subtable, 2);

            lua_pushnumber(L, itr.second.yOffset);
            lua_rawseti(L, subtable, 3);

            // Create links subtable
            lua_createtable(L, itr.second.child_links.size(), 0);
            int linktable = lua_gettop(L);
            uint32 i = 0;
            for (auto const& itrr : itr.second.child_links)
            {
                lua_pushnumber(L, itrr);
                lua_rawseti(L, linktable, ++i);
            }
            lua_rawseti(L, subtable, 4);

            lua_pushnumber(L, itr.second.Mutex);
            lua_rawseti(L, subtable, 5);

            lua_pushnumber(L, itr.second.buttonType);
            lua_rawseti(L, subtable, 6);

            lua_pushnumber(L, itr.second.flagMask);
            lua_rawseti(L, subtable, 7);

            // Push the table itself to maintable
            lua_rawseti(L, maintable, itr.second.Index);
        }

        lua_settop(L, maintable); // make the maintable to be the top of the stack
        // We could also just push it here to the stack as the last step
        return 1;
    }

    int GetCustomTalent(Eluna* E)
    {
        lua_State* L = E->L;
        uint32 entry = E->CHECKVAL<uint32>(1);
        TalentNodeInfo const* nodeInfo = eObjectMgr->GetTalentNode(entry);
        if (!nodeInfo)
            return luaL_argerror(L, 1, "valid talent node index expected");

        E->Push(nodeInfo->Index);
        E->Push(nodeInfo->spellId);
        E->Push(nodeInfo->xOffset);
        E->Push(nodeInfo->yOffset);
        E->Push(nodeInfo->Mutex);
        E->Push(nodeInfo->buttonType);
        E->Push(nodeInfo->flagMask);
        lua_createtable(L, nodeInfo->child_links.size(), 0);
        int tbl = lua_gettop(L);
        uint32 i = 0;
        for (auto const& itr : nodeInfo->child_links)
        {
            E->Push(itr);
            lua_rawseti(L, tbl, ++i);
        }
        lua_settop(L, tbl);
        return 1;
    }

    int LoadCustomTalentNode(Eluna* E)
    {
        uint32 entry = E->CHECKVAL<uint32>(1);
        eObjectMgr->LoadTalentNodeEntry(entry);
        return 0;
    }

    int DeleteTalentNodeBecauseFoeisAMadMan(Eluna* E)
    {
        uint32 entry = E->CHECKVAL<uint32>(1);
        eObjectMgr->DeleteTalentNodeEntry(entry);
         return 0;
     }

    int GetElunaMapId(Eluna* E)
    {
        E->Push(E->GetBoundMapId());
        return 1;
    }

    int GetDungeonLevel(Eluna* E, Group* group)
    {
        E->Push(group->GetDungeonLevel());
        return 1;
    }

    int GetCappedDungeonLevel(Eluna* E, Group* group)
    {
        E->Push(group->GetCappedDungeonLevel());
        return 1;
    }

    int GetAffixGroup(Eluna* E, Group* group)
    {
        AffixGroup affixGroup = sAffixMgr->GetAffixGroup(group);
        int numSlots = 4;

        lua_State* L = E->L;
        lua_createtable(L, numSlots, 0);
        int tbl = lua_gettop(L);
        for (int i = 1; i <= numSlots; ++i)
        {
            lua_createtable(L, 3, 0); // 3 being the number of values in the inner table
            int subtable = lua_gettop(L);

            lua_pushnumber(L, affixGroup.GetAffixId(i));
            lua_rawseti(L, subtable, 1);

            lua_pushnumber(L, affixGroup.GetNumRolls(i));
            lua_rawseti(L, subtable, 2);

            lua_pushnumber(L, affixGroup.GetRank(i));
            lua_rawseti(L, subtable, 3);

            lua_rawseti(L, tbl, i);
        }
        lua_settop(L, tbl);
        return 1;
    }

    int SetAffixSlot(Eluna* E, Group* group)
    {
        uint8 slot = E->CHECKVAL<uint8>(2);
        uint32 affixId = E->CHECKVAL<uint32>(3);
        int rolls = E->CHECKVAL<int>(4);
        uint8 rank = E->CHECKVAL<uint8>(5);
        sAffixMgr->GetAffixGroup(group).SetSlot(slot, affixId, rolls, rank);
        return 0;
    }

    int GetDisenchantId(Eluna* E, Item* item)
    {
        E->Push(item->GetTemplate()->DisenchantID);
        return 1;
    }

    int GetDungeonLevel(Eluna* E, Map* map)
    {
        E->Push(map->GetDungeonLevel());
        return 1;
    }

    int GetCappedDungeonLevel(Eluna* E, Map* map)
    {
        E->Push(map->GetCappedDungeonLevel());
        return 1;
    }

    int UpdateDungeonLevel(Eluna* /*E*/, Map* map)
    {
        map->UpdateDungeonLevel();
        return 0;
    }

    int SetGraveyardOverride(Eluna* E, Map* map)
    {
        uint32 mapid = E->CHECKVAL<uint32>(2);
        float x = E->CHECKVAL<float>(3);
        float y = E->CHECKVAL<float>(4);
        float z = E->CHECKVAL<float>(5);
        float o = E->CHECKVAL<float>(6);
        map->graveyardOverride = WorldLocation(mapid, x, y, z, o);
        return 0;
    }

    int UpscaleMapIfNeeded(Eluna* /*E*/, Map* map)
    {
        map->UpscaleMapIfNeeded();
        return 0;
    }

    int GetAffixSlotData(Eluna* E, Map* map)
    {
        bool baseSpells = E->CHECKVAL<bool>(2, false);
        if (baseSpells)
        {
            E->Push(sAffixMgr->GetAffixEffect(map->GetAffixSlot(1)).GetBaseSpell());
            E->Push(sAffixMgr->GetAffixEffect(map->GetAffixSlot(2)).GetBaseSpell());
            E->Push(sAffixMgr->GetAffixEffect(map->GetAffixSlot(3)).GetBaseSpell());
            E->Push(sAffixMgr->GetAffixEffect(map->GetAffixSlot(4)).GetBaseSpell());
        }
        else
        {
            E->Push(map->GetAffixSlot(1));
            E->Push(map->GetAffixSlot(2));
            E->Push(map->GetAffixSlot(3));
            E->Push(map->GetAffixSlot(4));
        }
        return 4;
    }

    int ToTransport(Eluna* E, Object* obj)
    {
        E->Push(reinterpret_cast<Transport*>(obj));
        return 1;
    }

    int AddVirtualItem(Eluna* E, Player* player)
    {
        uint32 itemId = E->CHECKVAL<uint32>(2);
        uint32 itemCount = E->CHECKVAL<uint32>(3, 1);
        uint32 displayId = E->CHECKVAL<uint32>(4, 0);
        const char* name = E->CHECKVAL<const char*>(5, "");
        int8 quality = E->CHECKVAL<int8>(6, -1);
        int8 minQuality = E->CHECKVAL<int8>(7, -1);
        int8 statGroup = E->CHECKVAL<int8>(8, -1);
        bool isCrafted = E->CHECKVAL<bool>(9, false);
        uint32 ilevelBonus = E->CHECKVAL<uint32>(10, 0);

        VirtualModifier modifier;

        if(displayId > 0)
            modifier.displayId = displayId;

        if(strlen(name) > 0)
            modifier.nameOverride = name;

        if(quality > -1)
            modifier.quality = uint8(quality);

        if (minQuality > -1)
            modifier.minQuality = uint8(minQuality);

        if (statGroup > -1)
            modifier.statgroup = StatGroup(statGroup);

        modifier.isCrafted = isCrafted;

        // if item is a crafted item, flag it as low yield to prevent re-crafting into infinity
        if (isCrafted)
            modifier.lowYield = true;

        if (ilevelBonus > 0)
            modifier.ilevelBonus = ilevelBonus;

        uint32 noSpaceForCount = 0;
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, itemCount, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)
            itemCount -= noSpaceForCount;

        if (itemCount == 0 || dest.empty())
            return 1;

        Item* item = player->StoreNewItem3(dest, itemId, true, GenerateItemRandomPropertyId(itemId), GuidSet(), modifier);
        if (item)
        {
            if (isCrafted)
                item->SetGuidValue(ITEM_FIELD_CREATOR, player->GetGUID());

            player->SendNewItem(item, itemCount, true, false);
        }

        E->Push(item);
        return 1;
    }

    int SendUpdateWorldState(Eluna* E, Player* player)
    {
        uint32 StateId = E->CHECKVAL<uint32>(2);
        uint32 Value = E->CHECKVAL<uint32>(3);

        player->SendUpdateWorldState(StateId, Value);

        return 0;
    }

    /**
     * Returns players average item level.
     *
     * @return average item level.
     */
    int GetAvgItemLevel(Eluna* E, Player* player)
    {
        E->Push(std::floor(player->GetAverageItemLevel()));
        return 1;
    }

    int GetPlayerOrGroupItemLevel(Eluna* E, Player* player)
    {
        E->Push(player->GetGroupOrPlayerItemLevel());
        return 1;
    }

    int GetCappedPlayerOrGroupItemLevel(Eluna* E, Player* player)
    {
        E->Push(player->GetCappedGroupOrPlayerItemLevel());
        return 1;
    }

    int GetCappedAvgItemLevel(Eluna* E, Player* player)
    {
        E->Push(std::floor(player->GetCappedItemLevel()));
        return 1;
    }

    int QuestKillCredit(Eluna* E, Player* player)
    {
        uint32 id = E->CHECKVAL<uint32>(2);
        player->AdvanceQuestCredit(id);
        return 0;
    }

    int GetLFGRole(Eluna* E, Player* player)
    {
        if (player->GetGroup())
        {
            auto slots = player->GetGroup()->GetMemberSlots();
            for (auto it = slots.begin(); it != slots.end(); ++it)
            {
                if (it->guid == player->GetGUID())
                {
                    /*
                        PLAYER_ROLE_TANK                             = 0x02,
                        PLAYER_ROLE_HEALER                           = 0x04,
                        PLAYER_ROLE_DAMAGE                           = 0x08
                    */
                    auto roles = it->roles;
                    // Tank
                    if (roles & 0x02)
                    {
                        E->Push(0);
                    }
                    // Healer
                    else if (roles & 0x04)
                    {
                        E->Push(1);
                    }
                    // DPS
                    else if (roles & 0x08)
                    {
                        E->Push(2);
                    }
                    break;
                }
            }
        }
        return 1;
    }

    int UpdateAchievementCriteria(Eluna* E, Player* player)
    {
        uint32 type = E->CHECKVAL<uint32>(2);
        uint32 miscValue1 = E->CHECKVAL<uint32>(3, 0);
        uint32 miscValue2 = E->CHECKVAL<uint32>(4, 0);
        //WorldObject* ref = E->CHECKOBJ<WorldObject>(5);
        player->UpdateAchievementCriteria((AchievementCriteriaTypes)type, miscValue1, miscValue2);
        return 0;
    }

    int CompleteAchievement(Eluna* E, Player* player)
    {
        uint32 id = E->CHECKVAL<uint32>(2);
        auto achievement = AchievementGlobalMgr::instance()->GetAchievement(id);
        if (achievement)
        {
            player->CompletedAchievement(achievement);
        }
        return 0;
    }

    int FinishDungeon(Eluna* /*E*/, Player* player)
    {
        auto group = player->GetGroup();
        if (group)
        {
            sLFGMgr->FinishDungeon(group->GetGUID(), sLFGMgr->GetDungeon(group->GetGUID()), player->GetMap());
        }
        return 0;
    }

    int AdvanceQuestObjective(Eluna* E, Player* player)
    {
        uint32 entry = E->CHECKVAL<uint32>(2);
        uint32 objective = E->CHECKVAL<uint32>(3);
        player->AdvanceQuestObjective(entry, objective);
        return 0;
    }

    int GetRequiredQuestObjectiveCount(Eluna* E, Player* player)
    {
        uint32 entry = E->CHECKVAL<uint32>(2);
        uint32 npcId = E->CHECKVAL<uint32>(3);
        E->Push(player->GetReqKillOrCastCurrentCount(entry, npcId));
        return 1;
    }

    int GetItemLevelPayload(Eluna* E, Player* player)
    {
        auto payload = player->GetItemLevelPayload();
        for (auto &it : payload)
        {
            E->Push(it.first);
            E->Push(it.second);
        }
        return payload.size() * 2;
    }

    int UpdateTalentPassives(Eluna* /*E*/, Player* /*player*/)
    {
        //player->UpdateArmorPassives();
        return 0;
    }

    int RemoveTalentPassives(Eluna* /*E*/, Player* /*player*/)
    {
        //player->RemoveArmorPassives();
        return 0;
    }

    int IsStackingSpell(Eluna* E, Player* /*player*/)
    {
        uint32 spell = E->CHECKVAL<uint32>(2);
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell))
        {
            E->Push(spellInfo->StackAmount > 0);
        }
        return 1;
    }

    int IncreaseSpellAuraStack(Eluna* E, Player* player)
    {
        uint32 spell = E->CHECKVAL<uint32>(2);
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell))
        {
            if (auto aura = player->GetAura(spell))
            {
                aura->SetStackAmount(aura->GetStackAmount() + 1);
            }
            else
            {
                AuraCreateInfo createInfo(spellInfo, MAX_EFFECT_MASK, player);
                createInfo.SetCaster(player);
                if (auto applyAura = Aura::TryRefreshStackOrCreate(createInfo))
                {
                    applyAura->SetStackAmount(1);
                }
                else
                {
                    TC_LOG_ERROR("spells", "Error applying armor passive, broken spell %u?", spell);
                }
            }
        }
        return 0;
    }

    int IncreaseUsedTalentCount(Eluna* /*E*/, Player* player)
    {
        player->IncreaseUsedTalentCount();
        return 0;
    }

    int OverridePetSpells(Eluna* E, Player* player)
    {
        Unit* pet = E->CHECKOBJ<Creature>(2, false);

        uint8 cooldownCount = pet->GetSpellHistory()->GetCooldownsSizeForPacket();

        WorldPacket data(SMSG_PET_SPELLS, 8 + 2 + 4 + 4 + 4 * 10 + 1 + 1 + cooldownCount * (4 + 2 + 4 + 4));
        data << uint64(pet->GetGUID());                         // Guid
        data << uint16(0);                                      // Pet Family (0 for all vehicles)
        data << uint32(0);                                      // Duration
        // The following three segments are read by the client as one uint32
        data << uint8(0);                                       // React State
        data << uint8(0);                                       // Command State
        data << uint16(0);                                      // DisableActions (set for all vehicles)

        for (uint32 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 spellId = 11;//TODO: Get real spell Ids
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
            {
                data << uint16(0) << uint8(0) << uint8(i + 8);
                continue;
            }

            if (spellInfo->IsPassive())
                pet->CastSpell(pet, spellId, true);

            data << uint32(MAKE_UNIT_ACTION_BUTTON(spellId, i + 8));
        }

        for (uint32 i = MAX_CREATURE_SPELLS; i < MAX_SPELL_CONTROL_BAR; ++i)
            data << uint32(0);

        data << uint8(0); // Auras?

        // Cooldowns
        pet->GetSpellHistory()->WritePacket<Pet>(data);
        player->SendDirectMessage(&data);

        return 0;
    }

    int ResetInstances(Eluna* /*E*/, Player* player)
    {
        player->ResetInstances(INSTANCE_RESET_ALL, false);
        return 0;
    }

    int IsInstanceBound(Eluna* E, Player* player)
    {
        uint32 mapId = E->CHECKVAL<uint32>(2, false);

        E->Push(player->IsInstanceBound(mapId));

        return 1;
    }

    int GetTalentLevel(Eluna* E, Player* player)
    {
        E->Push(player->GetTalentLevel());
        return 1;
    }

    int ScrapItem(Eluna* E, Player* player)
    {
        Item* item = E->CHECKOBJ<Item>(2, false);
        if (item)
        {
            if (player->HasItemCount(item->GetEntry(), 1, true))
            {
                if (item->HasSocketedGems())
                {
                    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
                    {
                        uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                        if (!enchant_id)
                            continue;

                        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                        if (!enchantEntry)
                            continue;

                        ItemTemplate const* gemProto = sObjectMgr->GetItemTemplate(enchantEntry->SrcItemID);
                        if (!gemProto)
                            continue;

                        if (gemProto->DisenchantID == 0)
                            continue;

                        player->AutoStoreLootNonPersonal(gemProto->DisenchantID, LootTemplates_Disenchant, true, false, false);
                    }

                }

                if(item->GetTemplate()->DisenchantID > 0)
                    player->AutoStoreLootNonPersonal(item->GetTemplate()->DisenchantID, LootTemplates_Disenchant, true, false, false);

                player->DestroyItemCount(item->GetEntry(), 1, true);
            }
        }

        return 0;
    }

    int SetPortalLocation(Eluna* E, Player* player)
    {
        uint32 mapId = E->CHECKVAL<uint32>(2);
        float x = E->CHECKVAL<float>(3);
        float y = E->CHECKVAL<float>(4);
        float z = E->CHECKVAL<float>(5);
        float o = E->CHECKVAL<float>(6);
        player->SetPortalLocation(WorldLocation(mapId, x, y, z, o));
        return 0;
    }

    int GetPortalLocation(Eluna* E, Player* player)
    {
        WorldLocation loc = player->GetPortalLocation();
        E->Push(loc.GetMapId());
        E->Push(loc.GetPositionX());
        E->Push(loc.GetPositionY());
        E->Push(loc.GetPositionZ());
        E->Push(loc.GetOrientation());
        return 5;
    }

    int SetOverrideLight(Eluna* E, Player* player)
    {
        uint32 areaLightId = E->CHECKVAL<uint32>(2);
        uint32 overrideLightId = E->CHECKVAL<uint32>(3);
        uint32 transitionMilliseconds = E->CHECKVAL<uint32>(4);

        WorldPackets::Misc::OverrideLight overrideLight;
        overrideLight.AreaLightID = areaLightId;
        overrideLight.OverrideLightID = overrideLightId;
        overrideLight.TransitionMilliseconds = transitionMilliseconds;
        overrideLight.Write();

        player->SendDirectMessage(overrideLight.GetRawPacket());
        
        return 0;
    }

    int GetSubClass(Eluna* E, Player* player)
    {
        E->Push(player->GetSubClass());
        return 1;
    }

    int IsDeveloper(Eluna* E, Player* player)
    {
        E->Push(player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR);
        return 1;
    }

    int UpdateTimewalkerTabard(Eluna* E, Player* player)
    {
        uint32 talent = E->CHECKVAL<uint32>(2);

        // Reset
        if (talent == 0) {
            // Find any possible tabard
            if (Item* item = player->GetItemByEntry(82094)) // Warden
            {
                // Timewalker Tabard
                item->SetEntry(82093);
                if (item->IsEquipped())
                {
                    player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                    WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                    sWorld->SendGlobalMessage(&response);
                }
            }
            else if (Item* item = player->GetItemByEntry(82095)) // Weaver
            {
                // Timewalker Tabard
                item->SetEntry(82093);
                if (item->IsEquipped())
                {
                    player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                    WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                    sWorld->SendGlobalMessage(&response);
                }
            }
            if (Item* item = player->GetItemByEntry(82096)) // Ranger
            {
                // Timewalker Tabard
                item->SetEntry(82093);
                if (item->IsEquipped())
                {
                    player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                    WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                    sWorld->SendGlobalMessage(&response);
                }
            }
            if (Item* item = player->GetItemByEntry(82097)) // Watcher
            {
                // Timewalker Tabard
                item->SetEntry(82093);
                if (item->IsEquipped())
                {
                    player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                    WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                    sWorld->SendGlobalMessage(&response);
                }
            }
            if (Item* item = player->GetItemByEntry(82098)) // Historian
            {
                // Timewalker Tabard
                item->SetEntry(82093);
                if (item->IsEquipped())
                {
                    player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                    WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                    sWorld->SendGlobalMessage(&response);
                }
            }
        }
        // Learnt new starter talent
        else
        {
            auto itemId = 0;
            if (talent == 180000) // Warden
                itemId = 82094;
            else if (talent == 180001) // Historian
                itemId = 82098;
            else if (talent == 180002) // Weaver
                itemId = 82095;
            else if (talent == 180003) // Watcher
                itemId = 82097;
            else if (talent == 180004) // Ranger
                itemId = 82096;
            if (itemId > 0)
            {
                // Timewalker Tabard
                Item* item = player->GetItemByEntry(82093);
                if (item)
                {
                    item->SetEntry(itemId);
                    if (item->IsEquipped())
                    {
                        player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + (item->GetSlot() * 2), item->GetEntry());
                        WorldPacket response = item->GetTemplate()->BuildQueryData(LOCALE_enUS);
                        sWorld->SendGlobalMessage(&response);
                    }
                }
            }
        }
        return 0;
    }

    int GetMagicFind(Eluna* E, Player* player)
    {
        E->Push(player->GetMagicFind());
        return 1;
    }

    int LearnCustomTalent(Eluna* E, Player* player)
    {
        uint32 node = E->CHECKVAL<uint32>(2);
        player->LearnCustomTalent(node);
        return 0;
    }

    int UnlearnCustomTalent(Eluna* E, Player* player)
    {
        uint32 node = E->CHECKVAL<uint32>(2);
        player->UnlearnCustomTalent(node);
        return 0;
    }

    int ResetCustomTalent(Eluna* /*E*/, Player* player)
    {
        player->ResetCustomTalents();
        return 0;
    }

    int SetTalentLoadout(Eluna* E, Player* player)
    {
        uint32 loadout = E->CHECKVAL<uint32>(2);
        player->SetTalentLoadout(loadout);
        return 0;
    }

    int GetCustomTalents(Eluna* E, Player* player)
    {
        const std::vector<uint32> talents = player->GetCustomTalents();
        lua_State* L = E->L;
        lua_createtable(L, talents.size(), 0);
        int tbl = lua_gettop(L);
        uint32 i = 0;
        for (auto itr = talents.begin(); itr != talents.end(); ++itr)
        {
            E->Push(*itr);
            lua_rawseti(L, tbl, ++i);
        }
        lua_settop(L, tbl);
        // Dreams:
        //E->Push(player->GetCustomTalents());
        return 1;
    }

    int CanLearnCustomTalent(Eluna* E, Player* player)
    {
        uint32 id = E->CHECKVAL<uint32>(2);
        E->Push(player->CanLearnCustomTalent(id));
        return 1;
    }

    int SendItemQueryPacket(Eluna* E, Player* player)
    {
        uint32 entry = E->CHECKVAL<uint32>(2);
        if (const ItemTemplate* item_template = sObjectMgr->GetItemTemplate(entry))
        {
            player->SendDirectMessage(&item_template->QueryData[static_cast<uint32>(LOCALE_enUS)]);
        }
        return 0;
    }

    int SetLootPreference(Eluna* E, Player* player)
    {
        uint8 preference = E->CHECKVAL<uint8>(2);

        preference = preference < MAX_PREF ? preference : 0;
        player->SetLootPreference(preference);

        return 0;
    }

    int GetLootPreference(Eluna* E, Player* player)
    {
        E->Push(player->GetActiveLootPreference());
        return 1;
    }

    int QueueGroupWithAffixConfig(Eluna* E, Player* player)
    {
        if (player->GetGroup() && player->GetGroup()->GetMembersCount() == 5)
        {
            uint32 affix1 = E->CHECKVAL<uint8>(2);
            uint32 affix2 = E->CHECKVAL<uint8>(3);
            uint32 affix3 = E->CHECKVAL<uint8>(4);
            uint32 affix4 = E->CHECKVAL<uint8>(5);
            uint8 roles = lfg::LfgRoles::PLAYER_ROLE_ANY;
            std::set<uint32> dungeons;
            // Random 5 man dungeon
            dungeons.insert(301);
            int code = sLFGMgr->JoinLfg(player, roles, dungeons, "Affix", lfg::LfgGroupType::GROUP_5_MAN, affix1, affix2, affix3, affix4);
            E->Push(code);
            return 1;
        }
        E->Push(-1);
        return 1;
    }

    /**
     * Returns the stack of [Aura]s of the given spell entry on the [Unit] or nil.
     *
     * @param uint32 spellID : entry of the aura spell
     * @return uint8 number : Stack of spells or nil
     */
    int GetSpellStackAmount(Eluna* E, Unit* unit)
    {
        uint32 spellID = E->CHECKVAL<uint32>(2);
        Aura const* spell = unit->GetAura(spellID);

        if (!spell)
            return 1;

        E->Push(spell->GetStackAmount());

        return 1;
    }

    /**
     * Spawns a dynamic object
     *
     * @param uint32 spell id
     * @param float radius
     * @param uint8 type (only 0-2 valid)
     * @param uint32 duration (in seconds)
     * @param float x cord
     * @param float y cord
     * @param float z cord
     */
    int SpawnDynObject(Eluna* E, Unit* unit)
    {
        uint32 spellId = E->CHECKVAL<uint32>(2);
        float radius = E->CHECKVAL<float>(3);
        uint8 type = E->CHECKVAL<uint8>(4);
        uint32 duration = E->CHECKVAL<uint32>(5);
        float x = E->CHECKVAL<float>(6);
        float y = E->CHECKVAL<float>(7);
        float z = E->CHECKVAL<float>(8);

        lua_State* L = E->L;
        if(!sSpellMgr->GetSpellInfo(spellId))
            return luaL_argerror(L, 2, "invalid spell supplied .");
        if (type >= 3)
            return luaL_argerror(L, 4, "valid dynamic object type expected");
        DynamicObject* dynObj = new DynamicObject(false);
        if (dynObj->CreateDynamicObject(unit->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(), unit, spellId, Position(x, y, z), radius, DynamicObjectType(type)))
            dynObj->SetDuration(duration);
        else
        {
            delete dynObj;
            return luaL_error(L, "Unexpected error occured creating dynamic object.");
        }
          
        return 0;
    }

    int SendEncounterFrame(Eluna* E, Unit* unit)
    {
        uint32 type = E->CHECKVAL<uint32>(2);
        uint8 param1 = E->CHECKVAL<uint8>(3);
        uint8 param2 = E->CHECKVAL<uint8>(4);

        lua_State* L = E->L;
        if(type > 7)
            return luaL_error(L, "SendEncounterFrame type larger then expected ( > 7)");
        WorldPacket data(SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT, 15);
        data << uint32(type);

        switch (type)
        {
        case /*ENCOUNTER_FRAME_ENGAGE*/0:
        case /*ENCOUNTER_FRAME_DISENGAGE*/1:
        case /*ENCOUNTER_FRAME_UPDATE_PRIORITY*/2:
            data << unit->GetPackGUID();
            data << uint8(param1);
            break;
        case /*ENCOUNTER_FRAME_ADD_TIMER*/3:
        case /*ENCOUNTER_FRAME_ENABLE_OBJECTIVE*/4:
        case /*ENCOUNTER_FRAME_DISABLE_OBJECTIVE*/6:
            data << uint8(param1);
            break;
        case /*ENCOUNTER_FRAME_UPDATE_OBJECTIVE*/5:
            data << uint8(param1);
            data << uint8(param2);
            break;
        default:
            break;
        }
        unit->GetMap()->SendToPlayers(&data);
        return 0;
    }

    int SetCanSeePhaseOne(Eluna* E, Unit* unit)
    {
        bool canSee = E->CHECKVAL<bool>(2);
        unit->SetCanSeePhaseOne(canSee);
        return 0;
    }

    int GetCanSeePhaseOne(Eluna* E, Unit* unit)
    {
        E->Push(unit->CanSeePhaseOne());
        return 1;
    }

    int SetCanSeeUniquePhase(Eluna* E, Unit* unit)
    {
        bool canSee = E->CHECKVAL<bool>(2);
        unit->SetCanSeeUniquePhase(canSee);
        return 0;
    }

    int GetCanSeeUniquePhase(Eluna* E, Unit* unit)
    {
        E->Push(unit->CanSeeUniquePhase());
        return 1;
    }

    int RemoveMotion(Eluna* E, Unit* unit)
    {
        int moveType = E->CHECKVAL<int>(2);
        int moveSlot = E->CHECKVAL<int>(3, 0);


        unit->GetMotionMaster()->Remove((MovementGeneratorType)moveType, (MovementSlot)moveSlot);
        return 0;
    }

    int ClearMotion(Eluna* /*E*/, Unit* unit)
    {
        unit->GetMotionMaster()->Clear();
        return 0;
    }

    int CreateTransport(Eluna* E, WorldObject* obj)
    {
        uint32 objectId = E->CHECKVAL<uint32>(2);
        bool enableMovement = E->CHECKVAL<bool>(3, false);
        Transport* transport = TransportMgr::instance()->CreateTransport(objectId, 0, obj->GetMap());
        if (transport)
        {
            transport->EnableMovement(enableMovement);
        }
        E->Push(transport);
        return 1;
    }

    int SetZoneOverrideLight(Eluna* E, WorldObject* obj)
    {
        uint32 zoneId = E->CHECKVAL<uint16>(2);
        uint32 areaLightId = E->CHECKVAL<uint32>(3);
        uint32 overrideLightId = E->CHECKVAL<uint32>(4);
        uint32 transitionTimeInMS = E->CHECKVAL<uint32>(5);

        obj->GetMap()->SetZoneOverrideLight(zoneId, areaLightId, overrideLightId, Milliseconds(transitionTimeInMS));
        return 0;
    }

    int SetVisible(Eluna* E, WorldObject* obj)
    {
        bool visible = E->CHECKVAL<bool>(2);
        if (visible)
        {
            obj->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);
        }
        else
        {
            obj->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_GAMEMASTER);
        }
        obj->UpdateObjectVisibility(true);
        return 0;
    }
    
    /**
     * Sets the [WorldObject] to be active or not
     *
     * Loads the worldobject on the grid even though
     * it may not be on a grid where a player is.
     *
     *
     * @param bool active 
     */
    int SetActive(Eluna* E, WorldObject* obj)
    {
        bool active = E->CHECKVAL<bool>(2);
        obj->setActive(active);
        return 0;
    }

    int SetServersideVisibility(Eluna* E, WorldObject* obj)
    {
        ServerSideVisibilityType type = (ServerSideVisibilityType)E->CHECKVAL<uint16>(2);
        uint32 value = E->CHECKVAL<uint32>(3);
        obj->m_serverSideVisibility.SetValue(type, value);
        obj->UpdateObjectVisibility();
        return 0;
    }

    int SetServersideVisibilityDetection(Eluna* E, WorldObject* obj)
    {
        ServerSideVisibilityType type = (ServerSideVisibilityType)E->CHECKVAL<uint16>(2);
        uint32 value = E->CHECKVAL<uint32>(3);
        obj->m_serverSideVisibilityDetect.SetValue(type, value);
        obj->UpdateObjectVisibility();
        return 0;
    }

    enum SelectAggroTarget
    {
        SELECT_TARGET_RANDOM = 0,   // Just selects a random target
        SELECT_TARGET_TOPAGGRO,     // Selects targes from top aggro to bottom
        SELECT_TARGET_BOTTOMAGGRO,  // Selects targets from bottom aggro to top
        SELECT_TARGET_NEAREST,
        SELECT_TARGET_FARTHEST
    };

    /**
     * Returns a target from the [Creature]'s threat list based on the
     *   supplied arguments.
     *
     *     enum SelectAggroTarget
     *     {
     *         SELECT_TARGET_RANDOM = 0,  //Just selects a random target
     *         SELECT_TARGET_TOPAGGRO,    //Selects targets from top aggro to bottom
     *         SELECT_TARGET_BOTTOMAGGRO, //Selects targets from bottom aggro to top
     *         SELECT_TARGET_NEAREST,
     *         SELECT_TARGET_FARTHEST
     *     };
     *
     * For example, if you wanted to select the third-farthest [Player]
     *   within 50 yards that has the [Aura] "Corrupted Blood" (ID 24328),
     *   you could use this function like so:
     *
     *     target = creature:GetAITarget(4, true, 3, 50, 24328)
     *
     * @param [SelectAggroTarget] targetType : how the threat list should be sorted
     * @param bool playerOnly = false : if `true`, skips targets that aren't [Player]s
     * @param uint32 position = 0 : used as an offset into the threat list. If `targetType` is random, used as the number of players from top of aggro to choose from
     * @param float distance = 0.0 : if positive, the maximum distance for the target. If negative, the minimum distance
     * @param int32 aura = 0 : if positive, the target must have this [Aura]. If negative, the the target must not have this Aura
     * @return [Unit] target : the target, or `nil`
     */
    int GetAITarget(Eluna* E, Creature* creature)
    {
        uint32 targetType = E->CHECKVAL<uint32>(2);
        bool playerOnly = E->CHECKVAL<bool>(3, false);
        uint32 position = E->CHECKVAL<uint32>(4, 0);
        float dist = E->CHECKVAL<float>(5, 0.0f);
        int32 aura = E->CHECKVAL<int32>(6, 0);

        auto const& threatlist = creature->GetThreatManager().GetSortedThreatList();

        std::list<Unit*> targetList;
        for (ThreatReference const* itr : threatlist)
        {
            Unit* target = itr->GetVictim();
            if (!target || itr->IsOffline())
                continue;
            if (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                continue;
            if (aura > 0 && !target->HasAura(aura))
                continue;
            else if (aura < 0 && target->HasAura(-aura))
                continue;
            if (dist > 0.0f && !creature->IsWithinDist(target, dist))
                continue;
            else if (dist < 0.0f && creature->IsWithinDist(target, -dist))
                continue;
            targetList.push_back(target);
        }

        if (targetList.empty())
            return 1;
        if (position >= targetList.size())
            return 1;

        if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
            targetList.sort(ElunaUtil::ObjectDistanceOrderPred(creature));

        switch (targetType)
        {
        case SELECT_TARGET_NEAREST:
        case SELECT_TARGET_TOPAGGRO:
        {
            std::list<Unit*>::const_iterator itr = targetList.begin();
            if (position)
                std::advance(itr, position);
            E->Push(*itr);
        }
        break;
        case SELECT_TARGET_FARTHEST:
        case SELECT_TARGET_BOTTOMAGGRO:
        {
            std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
            if (position)
                std::advance(ritr, position);
            E->Push(*ritr);
        }
        break;
        case SELECT_TARGET_RANDOM:
        {
            std::list<Unit*>::const_iterator itr = targetList.begin();
            if (position)
                std::advance(itr, urand(0, position));
            else
                std::advance(itr, urand(0, targetList.size() - 1));
            E->Push(*itr);
        }
        break;
        default:
            luaL_argerror(E->L, 2, "SelectAggroTarget expected");
            break;
        }

        return 1;
    }
    
    /**
     * Adds an [Item] to a vendor and updates the world database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     * @param int32 maxcount : max [Item] stack count
     * @param uint32 incrtime : combined with maxcount, incrtime tells how often (in seconds) the vendor list is refreshed and the limited [Item] copies are restocked
     * @param uint32 extendedcost : unique cost of an [Item], such as conquest points for example
     */
    int AddVendorItem(Eluna* E)
    {
        uint32 entry = E->CHECKVAL<uint32>(1);
        uint32 item = E->CHECKVAL<uint32>(2);
        int maxcount = E->CHECKVAL<int>(3);
        uint32 incrtime = E->CHECKVAL<uint32>(4);
        uint32 extendedcost = E->CHECKVAL<uint32>(5);
        bool persist = E->CHECKVAL<bool>(6, true);

        if (!eObjectMgr->IsVendorItemValid(entry, item, maxcount, incrtime, extendedcost))
            return 0;

        eObjectMgr->AddVendorItem(entry, item, maxcount, incrtime, extendedcost, persist);

        return 0;
    }

    /**
     * Removes an [Item] from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     */
    int VendorRemoveItem(Eluna* E)
    {
        uint32 entry = E->CHECKVAL<uint32>(1);
        uint32 item = E->CHECKVAL<uint32>(2);
        bool persist = E->CHECKVAL<bool>(3, true);

        if (!eObjectMgr->GetCreatureTemplate(entry))
            return luaL_argerror(E->L, 1, "valid CreatureEntry expected");

        eObjectMgr->RemoveVendorItem(entry, item, persist);

        return 0;
    }

    /**
     * Returns the area ID of the [Map] at the specified X, Y, and Z coordinates.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param uint32 phasemask = PHASEMASK_NORMAL
     * @return uint32 areaId
     */
    int GetAreaId(Eluna* E, Map* map)
    {
        float x = E->CHECKVAL<float>(2);
        float y = E->CHECKVAL<float>(3);
        float z = E->CHECKVAL<float>(4);
        float phasemask = E->CHECKVAL<uint32>(5, PHASEMASK_NORMAL);

        E->Push(map->GetAreaId(phasemask, x, y, z));
        return 1;
    }

    /**
    * Returns a table with all the current [Player]s in the map
    *
    *     enum TeamId
    *     {
    *         TEAM_ALLIANCE = 0,
    *         TEAM_HORDE = 1,
    *         TEAM_NEUTRAL = 2
    *     };
    *
    * @param [TeamId] team : optional check team of the [Player], Alliance, Horde or Neutral (All)
    * @return table mapPlayers
    */
    int GetPlayers(Eluna* E, Map* map)
    {
        bool includeGMS = E->CHECKVAL<bool>(2, false);

        lua_newtable(E->L);
        int tbl = lua_gettop(E->L);
        uint32 i = 0;

        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player)
                continue;
            if (player->GetSession())
            {
                if (!includeGMS && player->IsGameMaster())
                    continue;

                E->Push(player);
                lua_rawseti(E->L, tbl, ++i);
            }
        }

        lua_settop(E->L, tbl);
        return 1;
    }

    /**
     * Sends a vendor window to the [Player] from the [WorldObject] specified.
     *
     * @param [WorldObject] sender
     */
    int SendListInventory(Eluna* E, Player* player)
    {
        WorldObject* obj = E->CHECKOBJ<WorldObject>(2);
        uint32 vendorId = E->CHECKVAL<uint32>(3, 0);

        player->GetSession()->SendListInventory(obj->GET_GUID(), vendorId);
        return 0;
    }

    /**
     * Makes the [Unit] jump to the coordinates
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float zSpeed : start velocity
     * @param float maxHeight : maximum height
     * @param uint32 id = 0 : unique movement Id
     * @param bool hasOrientation = false : whether to force a orientation during the jump
     * @param float o = 0 : orientation to use if hasOrientation is true
     */
    int MoveJump(Eluna* E, Unit* unit)
    {
        float x = E->CHECKVAL<float>(2);
        float y = E->CHECKVAL<float>(3);
        float z = E->CHECKVAL<float>(4);
        float zSpeed = E->CHECKVAL<float>(5);
        float maxHeight = E->CHECKVAL<float>(6);
        uint32 id = E->CHECKVAL<uint32>(7, 0);
        bool hasOrientation = E->CHECKVAL<bool>(8, false);
        float o = E->CHECKVAL<float>(9, 0);

        unit->GetMotionMaster()->MoveJump(x, y, z, o, zSpeed, maxHeight, id, hasOrientation);
        return 0;
    }

    /**
     * The [Unit] will say the message
     *
     * @param string msg : message for the [Unit] to say
     * @param uint32 language : language for the [Unit] to speak
     */
    int SendUnitSay(Eluna* E, Unit* unit)
    {
        const char* msg = E->CHECKVAL<const char*>(2);
        uint32 language = E->CHECKVAL<uint32>(3, 0);
        if (std::string(msg).length() > 0)
            unit->Say(msg, (Language)language, unit);

        return 0;
    }

    /**
     * The [Unit] will yell the message
     *
     * @param string msg : message for the [Unit] to yell
     * @param uint32 language : language for the [Unit] to speak
     */
    int SendUnitYell(Eluna* E, Unit* unit)
    {
        const char* msg = E->CHECKVAL<const char*>(2);
        uint32 language = E->CHECKVAL<uint32>(3, 0);
        if (std::string(msg).length() > 0)
            unit->Yell(msg, (Language)language, unit);

        return 0;
    }

    /**
     * Returns the nearest [Player] object in sight of the [WorldObject] or within the given range
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     * @param bool filterGM = true : true excludes GMs from the return result, false includes them.
     *
     * @return [Player] nearestPlayer
     */
    int GetNearestPlayer(Eluna* E, WorldObject* obj)
    {
        float range = E->CHECKVAL<float>(2, SIZE_OF_GRIDS);
        uint32 hostile = E->CHECKVAL<uint32>(3, 0);
        uint32 dead = E->CHECKVAL<uint32>(4, 1);
        bool filterGM = E->CHECKVAL<bool>(5, true);

        Unit* target = NULL;
        ElunaUtil::WorldObjectInRangeCheck checker(true, obj, range, TYPEMASK_PLAYER, 0, hostile, dead, filterGM);
        Trinity::UnitLastSearcher<ElunaUtil::WorldObjectInRangeCheck> searcher(obj, target, checker);
        Cell::VisitAllObjects(obj, searcher, range);

        E->Push(target);
        return 1;
    }

    /**
     * Returns a table of [Player] objects in sight of the [WorldObject] or within the given range
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     * @param bool filterGM = true : true excludes GMs from the return result, false includes them.
     *
     * @return table playersInRange : table of [Player]s
     */
    int GetPlayersInRange(Eluna* E, WorldObject* obj)
    {
        float range = E->CHECKVAL<float>(2, SIZE_OF_GRIDS);
        uint32 hostile = E->CHECKVAL<uint32>(3, 0);
        uint32 dead = E->CHECKVAL<uint32>(4, 1);
        bool filterGM = E->CHECKVAL<bool>(5, true);

        std::list<Player*> list;
        ElunaUtil::WorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_PLAYER, 0, hostile, dead, filterGM);

        Trinity::PlayerListSearcher<ElunaUtil::WorldObjectInRangeCheck> searcher(obj, list, checker);
        Cell::VisitAllObjects(obj, searcher, range);

        lua_createtable(E->L, list.size(), 0);
        int tbl = lua_gettop(E->L);
        uint32 i = 0;

        for (std::list<Player*>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
            auto player = *it;
            if (filterGM && player->IsGameMaster())
                continue;

            E->Push(*it);
            lua_rawseti(E->L, tbl, ++i);
        }

        lua_settop(E->L, tbl);
        return 1;
    }

    /**
     * Returns nearest [WorldObject] in sight of the [WorldObject].
     * The distance, type, entry and hostility requirements the [WorldObject] must match can be passed.
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param [TypeMask] type = 0 : the [TypeMask] that the [WorldObject] must be. This can contain multiple types. 0 will be ingored
     * @param uint32 entry = 0 : the entry of the [WorldObject], 0 will be ingored
     * @param uint32 hostile = 0 : specifies whether the [WorldObject] needs to be 1 hostile, 2 friendly or 0 either
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return [WorldObject] worldObject
     */
    int GetNearObject(Eluna* E, WorldObject* obj)
    {
        float range = E->CHECKVAL<float>(2, SIZE_OF_GRIDS);
        uint16 type = E->CHECKVAL<uint16>(3, 0); // TypeMask
        uint32 entry = E->CHECKVAL<uint32>(4, 0);
        uint32 hostile = E->CHECKVAL<uint32>(5, 0); // 0 none, 1 hostile, 2 friendly
        uint32 dead = E->CHECKVAL<uint32>(6, 1); // 0 both, 1 alive, 2 dead
        bool filterGM = E->CHECKVAL<bool>(7, true);

        float x, y, z;
        obj->GetPosition(x, y, z);
        ElunaUtil::WorldObjectInRangeCheck checker(true, obj, range, type, entry, hostile, dead, filterGM);

        WorldObject* target = NULL;

        Trinity::WorldObjectLastSearcher<ElunaUtil::WorldObjectInRangeCheck> searcher(obj, target, checker);
        Cell::VisitAllObjects(obj, searcher, range);

        E->Push(target);
        return 1;
    }

    /**
     * Spawns the creature at specified location.
     *
     *     enum TempSummonType
     *     {
     *         TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1, // despawns after a specified time OR when the creature disappears
     *         TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2, // despawns after a specified time OR when the creature dies
     *         TEMPSUMMON_TIMED_DESPAWN               = 3, // despawns after a specified time
     *         TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4, // despawns after a specified time after the creature is out of combat
     *         TEMPSUMMON_CORPSE_DESPAWN              = 5, // despawns instantly after death
     *         TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6, // despawns after a specified time after death
     *         TEMPSUMMON_DEAD_DESPAWN                = 7, // despawns when the creature disappears
     *         TEMPSUMMON_MANUAL_DESPAWN              = 8, // despawns when UnSummon() is called
     *         TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN = 9, // despawns after a specified time (OOC) OR when the creature dies
     *         TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN   = 10 // despawns after a specified time (OOC) OR when the creature disappears
     *     };
     *
     * @param uint32 entry : [Creature]'s entry ID
     * @param float x
     * @param float y
     * @param float z
     * @param float o
     * @param [TempSummonType] spawnType = MANUAL_DESPAWN : defines how and when the creature despawns
     * @param uint32 despawnTimer = 0 : despawn time in milliseconds
     * @return [Creature] spawnedCreature
     */
    int SpawnCreature(Eluna* E, WorldObject* obj)
    {
        uint32 entry = E->CHECKVAL<uint32>(2);
        float x = E->CHECKVAL<float>(3);
        float y = E->CHECKVAL<float>(4);
        float z = E->CHECKVAL<float>(5);
        float o = E->CHECKVAL<float>(6);
        uint32 spawnType = E->CHECKVAL<uint32>(7, 8);
        uint32 despawnTimer = E->CHECKVAL<uint32>(8, 0);
        int dungeonLevel = E->CHECKVAL<int>(9, 0);

        TempSummonType type;
        switch (spawnType)
        {
        case 1:
            type = TEMPSUMMON_TIMED_OR_DEAD_DESPAWN;
            break;
        case 2:
            type = TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN;
            break;
        case 3:
            type = TEMPSUMMON_TIMED_DESPAWN;
            break;
        case 4:
            type = TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;
            break;
        case 5:
            type = TEMPSUMMON_CORPSE_DESPAWN;
            break;
        case 6:
            type = TEMPSUMMON_CORPSE_TIMED_DESPAWN;
            break;
        case 7:
            type = TEMPSUMMON_DEAD_DESPAWN;
            break;
        case 8:
            type = TEMPSUMMON_MANUAL_DESPAWN;
            break;
        default:
            return luaL_argerror(E->L, 7, "valid SpawnType expected");
        }

        E->Push(obj->SummonCreature(entry, x, y, z, o, type, Milliseconds(despawnTimer), dungeonLevel));
        return 1;
    }

    
    
    // REGISTERS
    
    ElunaGlobal::ElunaRegister GlobalMethods[] =
    {
        { "GetCustomTalentStorage", &LuaCustom::GetCustomTalentStorage },
        { "GetCustomTalent", &LuaCustom::GetCustomTalent },
        { "LoadCustomTalentNode", &LuaCustom::LoadCustomTalentNode },
        { "DeleteCustomTalentNode", &LuaCustom::DeleteTalentNodeBecauseFoeisAMadMan },
        { "VendorRemoveItem", &LuaCustom::VendorRemoveItem },
        { "AddVendorItem", &LuaCustom::AddVendorItem },
        { "GetElunaMapId", &LuaCustom::GetElunaMapId },
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Object> ObjectMethods[] =
    {
        { "ToTransport", &LuaCustom::ToTransport },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<WorldObject> WorldObjectMethods[] =
    {
        { "CreateTransport", &LuaCustom::CreateTransport },
        { "SetZoneOverrideLight", &LuaCustom::SetZoneOverrideLight },
        { "SetVisible", &LuaCustom::SetVisible },
        { "SetActive", &LuaCustom::SetActive },
        { "SetServersideVisibility", &LuaCustom::SetServersideVisibility },
        { "SetServersideVisibilityDetection", &LuaCustom::SetServersideVisibilityDetection },
        { "SpawnCreature", &LuaCustom::SpawnCreature },
        { "GetNearObject", &LuaCustom::GetNearObject },
        { "GetPlayersInRange", &LuaCustom::GetPlayersInRange },
        { "GetNearestPlayer", &LuaCustom::GetNearestPlayer },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Unit> UnitMethods[] =
    {
        { "GetSpellStackAmount", &LuaCustom::GetSpellStackAmount },
        { "SpawnDynObject", &LuaCustom::SpawnDynObject },
        { "SendEncounterFrame", &LuaCustom::SendEncounterFrame },
        { "SetCanSeePhaseOne", &LuaCustom::SetCanSeePhaseOne },
        { "GetCanSeePhaseOne", &LuaCustom::GetCanSeePhaseOne },
        { "SetCanSeeUniquePhase", &LuaCustom::SetCanSeeUniquePhase },
        { "GetCanSeeUniquePhase", &LuaCustom::GetCanSeeUniquePhase },
        { "RemoveMotion", &LuaCustom::RemoveMotion },
        { "ClearMotion", &LuaCustom::ClearMotion },
        { "SendUnitSay", &LuaCustom::SendUnitSay },
        { "SendUnitYell", &LuaCustom::SendUnitYell },
        { "MoveJump", &LuaCustom::MoveJump },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Player> PlayerMethods[] =
    {
        { "AddVirtualItem", &LuaCustom::AddVirtualItem },
        { "SendUpdateWorldState", &LuaCustom::SendUpdateWorldState },
        { "GetAvgItemLevel", &LuaCustom::GetAvgItemLevel },
        { "GetCappedAvgItemLevel", &LuaCustom::GetCappedAvgItemLevel },
        { "GetPlayerOrGroupItemLevel", &LuaCustom::GetPlayerOrGroupItemLevel },
        { "GetCappedPlayerOrGroupItemLevel", &LuaCustom::GetCappedPlayerOrGroupItemLevel },
        { "QuestKillCredit", &LuaCustom::QuestKillCredit },
        { "GetLFGRole", &LuaCustom::GetLFGRole },
        { "UpdateAchievementCriteria", &LuaCustom::UpdateAchievementCriteria },
        { "CompleteAchievement", &LuaCustom::CompleteAchievement },
        { "FinishDungeon", &LuaCustom::FinishDungeon },
        { "AdvanceQuestObjective", &LuaCustom::AdvanceQuestObjective },
        { "GetRequiredQuestObjectiveCount", &LuaCustom::GetRequiredQuestObjectiveCount },
        { "GetItemLevelPayload", &LuaCustom::GetItemLevelPayload },
        { "UpdateTalentPassives", &LuaCustom::UpdateTalentPassives },
        { "RemoveTalentPassives", &LuaCustom::RemoveTalentPassives },
        { "IsStackingSpell", &LuaCustom::IsStackingSpell },
        { "IncreaseSpellAuraStack", &LuaCustom::IncreaseSpellAuraStack },
        { "IncreaseUsedTalentCount", &LuaCustom::IncreaseUsedTalentCount },
        { "OverridePetSpells", &LuaCustom::OverridePetSpells },
        { "ResetInstances", &LuaCustom::ResetInstances },
        { "IsInstanceBound", &LuaCustom::IsInstanceBound },
        { "GetTalentLevel", &LuaCustom::GetTalentLevel },
        { "ScrapItem", &LuaCustom::ScrapItem },
        { "GetPortalLocation", &LuaCustom::GetPortalLocation },
        { "SetPortalLocation", &LuaCustom::SetPortalLocation },
        { "SetOverrideLight", &LuaCustom::SetOverrideLight },
        { "GetSubClass", &LuaCustom::GetSubClass },
        { "IsDeveloper", &LuaCustom::IsDeveloper },
        { "UpdateTimewalkerTabard", &LuaCustom::UpdateTimewalkerTabard },
        { "LearnCustomTalent", &LuaCustom::LearnCustomTalent },
        { "UnlearnCustomTalent", &LuaCustom::UnlearnCustomTalent },
        { "ResetCustomTalent", &LuaCustom::ResetCustomTalent },
        { "SetTalentLoadout", &LuaCustom::SetTalentLoadout },
        { "GetCustomTalents", &LuaCustom::GetCustomTalents },
        { "CanLearnCustomTalent", &LuaCustom::CanLearnCustomTalent },
        { "GetMagicFind", &LuaCustom::GetMagicFind },
        { "SendItemQueryPacket", &LuaCustom::SendItemQueryPacket },
        { "SetLootPreference", &LuaCustom::SetLootPreference },
        { "GetLootPreference", &LuaCustom::GetLootPreference },
        { "QueueGroupWithAffixConfig", &LuaCustom::QueueGroupWithAffixConfig },
        { "SendListInventory", &LuaCustom::SendListInventory },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Creature> CreatureMethods[] =
    {
        { "SetRegenerateHealth", &LuaCustom::SetRegenerateHealth },
        { "SetReactState", &LuaCustom::SetReactState },
        { "SetWaypoint", &LuaCustom::SetWaypoint },
        { "MoveCircle", &LuaCustom::MoveCircle },
        { "ClearLoot", &LuaCustom::ClearLoot },
        { "AnimateAndSetFlyMode", &LuaCustom::AnimateAndSetFlyMode },
        { "AnimateAndSetLandMode", &LuaCustom::AnimateAndSetLandMode },
        { "RemoveQuest", &LuaCustom::RemoveQuest },
        { "AddQuest", &LuaCustom::AddQuest },
        { "SendMirrorImage", &LuaCustom::SendMirrorToPlayer },
        { "GetAITarget", &LuaCustom::GetAITarget },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Item> ItemMethods[] =
    {
        { "GetDisenchantId", &LuaCustom::GetDisenchantId },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Group> GroupMethods[] =
    {
        { "GetDungeonLevel", &LuaCustom::GetDungeonLevel },
        { "GetCappedDungeonLevel", &LuaCustom::GetCappedDungeonLevel },
        { "GetAffixGroup", &LuaCustom::GetAffixGroup },
        { "SetAffixSlot", &LuaCustom::SetAffixSlot },
        
        { NULL, NULL, METHOD_REG_NONE }
    };
    
    ElunaRegister<Map> MapMethods[] =
    {
        { "GetDungeonLevel", &LuaCustom::GetDungeonLevel },
        { "GetCappedDungeonLevel", &LuaCustom::GetCappedDungeonLevel },
        { "UpdateDungeonLevel", &LuaCustom::UpdateDungeonLevel },
        { "SetGraveyardOverride", &LuaCustom::SetGraveyardOverride },
        { "UpscaleMapIfNeeded", &LuaCustom::UpscaleMapIfNeeded},
        { "GetAffixSlotData", &LuaCustom::GetAffixSlotData },
        { "GetPlayers", &LuaCustom::GetPlayers },
        { "GetAreaId", &LuaCustom::GetAreaId },
        
        { NULL, NULL, METHOD_REG_NONE }
    };

    inline void RegisterCustomFunctions(Eluna* E)
    {
        ElunaGlobal::SetMethods(E, GlobalMethods);

        ElunaTemplate<Object>::SetMethods(E, ObjectMethods);

        ElunaTemplate<WorldObject>::SetMethods(E, ObjectMethods);
        ElunaTemplate<WorldObject>::SetMethods(E, WorldObjectMethods);

        ElunaTemplate<Unit>::SetMethods(E, ObjectMethods);
        ElunaTemplate<Unit>::SetMethods(E, WorldObjectMethods);
        ElunaTemplate<Unit>::SetMethods(E, UnitMethods);

        ElunaTemplate<Player>::SetMethods(E, ObjectMethods);
        ElunaTemplate<Player>::SetMethods(E, WorldObjectMethods);
        ElunaTemplate<Player>::SetMethods(E, UnitMethods);
        ElunaTemplate<Player>::SetMethods(E, PlayerMethods);

        ElunaTemplate<Creature>::SetMethods(E, ObjectMethods);
        ElunaTemplate<Creature>::SetMethods(E, WorldObjectMethods);
        ElunaTemplate<Creature>::SetMethods(E, UnitMethods);
        ElunaTemplate<Creature>::SetMethods(E, CreatureMethods);

        ElunaTemplate<GameObject>::SetMethods(E, ObjectMethods);
        ElunaTemplate<GameObject>::SetMethods(E, WorldObjectMethods);

        ElunaTemplate<Corpse>::SetMethods(E, ObjectMethods);
        ElunaTemplate<Corpse>::SetMethods(E, WorldObjectMethods);

        ElunaTemplate<Item>::SetMethods(E, ObjectMethods);
        ElunaTemplate<Item>::SetMethods(E, ItemMethods);

        ElunaTemplate<Group>::SetMethods(E, GroupMethods);

        ElunaTemplate<Map>::SetMethods(E, MapMethods);
    };
};
    
#endif
