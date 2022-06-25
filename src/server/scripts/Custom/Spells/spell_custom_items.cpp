#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "World.h"
#include "VirtualItemMgr.h"
#include "WorldSession.h"
#include "Group.h"
#include "Guild.h"
#include "Chat.h"
#include "Item.h"
#include "BankPackets.h"

class spell_item_trinket_reset_cds : public SpellScript
{
    PrepareSpellScript(spell_item_trinket_reset_cds);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if(Player * plrTarget = GetHitPlayer())
            plrTarget->RemoveArenaSpellCooldowns();
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_trinket_reset_cds::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_item_unlock_bank_slot : public SpellScript
{
    PrepareSpellScript(spell_item_unlock_bank_slot);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }


    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        uint32 slot = caster->GetBankBagSlotCount() + 1;

        if (slot > 7)
        {
            WorldPackets::Bank::BuyBankSlotResult packet;
            packet.Result = ERR_BANKSLOT_FAILED_TOO_MANY;
            caster->GetSession()->SendPacket(packet.Write());
            ChatHandler(caster->GetSession()).PSendSysMessage("You are capped out on bank slots!");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        uint32 slots = caster->GetBankBagSlotCount() + 1;
        if (slots > 7)
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("Dummy Effect Bypassed CheckRequirement.");
            return;
        }
        ChatHandler(caster->GetSession()).PSendSysMessage("Your requisition form has been received and your additonal bank space is now available.");
        caster->SetBankBagSlotCount(slots);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_unlock_bank_slot::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_unlock_bank_slot::CheckRequirement);
    }
};

class spell_item_unlock_guild_bank_slot : public SpellScript
{
    PrepareSpellScript(spell_item_unlock_guild_bank_slot);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (Guild* g = caster->GetGuild())
        {
            if (g->_GetPurchasedTabsSize() >= GUILD_BANK_MAX_TABS)
            {
                ChatHandler(caster->GetSession()).PSendSysMessage("Your guild bank is at max tabs.");
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }

        }
        else
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You must be in a guild to use this item.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        if (Guild* g = caster->GetGuild())
        {
            g->_CreateNewBankTab();
            g->_BroadcastEvent(GE_BANK_TAB_PURCHASED, ObjectGuid::Empty);
            g->SendPermissions(caster->GetSession());
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_unlock_guild_bank_slot::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_unlock_guild_bank_slot::CheckRequirement);
    }
};

class spell_item_temporal_time_crystal : public SpellScript
{
    PrepareSpellScript(spell_item_temporal_time_crystal);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->GetMapId() == 35)
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You must exit The Vault before using another temporal time crystal.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        caster->ResetInstances(INSTANCE_RESET_ALL, false);
        caster->TeleportTo(35, -1.218f, 28.222f, -19.5f, 1.56996f);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_temporal_time_crystal::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_temporal_time_crystal::CheckRequirement);
    }
};

class spell_item_floating_cult_thesis : public SpellScript
{
    PrepareSpellScript(spell_item_floating_cult_thesis);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->GetMapId() == 769)
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You must exit the Floating Cult before using the Floating Cult Thesis.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        Group* group = caster->GetGroup();
        if (!group)
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You must be in a party to use this.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        if (group->GetMembersCount() != 2)
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("Your party must have only two players to use this.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        caster->ResetInstances(INSTANCE_RESET_ALL, false);
        Group* group = caster->GetGroup();
        if (!group || group->GetMembersCount() != 2)
        {
            return;
        }
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->GetSource() && itr->GetSource()->GetDistance2d(caster) < 50.0f)
            {
                itr->GetSource()->TeleportTo(769, 12163.0f, 15235.968f, 857.5f, 1.6f);
            }
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_floating_cult_thesis::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_floating_cult_thesis::CheckRequirement);
    }
};

class spell_item_transmog : public SpellScript
{
    PrepareSpellScript(spell_item_transmog);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();

        if (const Item* target = GetExplTargetItem())
        {
            if (target->IsEquipped())
                return SPELL_FAILED_BAD_TARGETS;

            if (sVirtualItemMgr.GetVirtualTemplate(target->GetEntry()))
            {
                if (Item* itemSlot = caster->GetItemByPos(INVENTORY_SLOT_BAG_0, caster->GetEquipSlot(target->GetTemplate())))
                {
                    if (target->GetTemplate()->Class == ITEM_CLASS_ARMOR && target->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD
                        && itemSlot->GetTemplate()->Class != ITEM_CLASS_ARMOR && itemSlot->GetTemplate()->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (target->GetTemplate()->Class != itemSlot->GetTemplate()->Class)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (!sVirtualItemMgr.GetVirtualTemplate(itemSlot->GetEntry()))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
                return SPELL_FAILED_BAD_TARGETS;
        }
        else
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        if (Item* source = GetExplTargetItem())
        {
            if (VirtualItemTemplate* copy = sVirtualItemMgr.GetVirtualTemplate(source->GetEntry()))
            {
                if (Item* transmogTarget = caster->GetItemByPos(INVENTORY_SLOT_BAG_0, caster->GetEquipSlot(source->GetTemplate())))
                {
                    if (VirtualItemTemplate* vTarget = sVirtualItemMgr.GetVirtualTemplate(transmogTarget->GetEntry()))
                    {
                        uint32 count = 1;
                        vTarget->DisplayInfoID = copy->DisplayInfoID;
                        vTarget->Sheath = copy->Sheath;
                        caster->DestroyItemCount(source, count, true);

                        if (!vTarget->HasFlag(VIRTUAL_ITEM_FLAG_DISPLAY_STATIC))
                            vTarget->customFlags |= VIRTUAL_ITEM_FLAG_DISPLAY_STATIC;

                        vTarget->InitializeQueryData();
                        WorldPacket response = vTarget->BuildQueryData(LOCALE_enUS);
                        sWorld->SendGlobalMessage(&response);
                        transmogTarget->SaveVirtualItemInfo();
                        ChatHandler(caster->GetSession()).SendSysMessage("Unequip and requip the item to apply it's new display.");
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_transmog::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_transmog::CheckRequirement);
    }
};

void AddSC_Spells_Custom_Items()
{
    RegisterSpellScript(spell_item_trinket_reset_cds);
    RegisterSpellScript(spell_item_unlock_bank_slot);
    RegisterSpellScript(spell_item_unlock_guild_bank_slot);
    RegisterSpellScript(spell_item_temporal_time_crystal);
    RegisterSpellScript(spell_item_floating_cult_thesis);
    RegisterSpellScript(spell_item_transmog);
}
