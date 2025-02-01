#include "ScriptMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "ObjectMgr.h"
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
            WorldPacket data(SMSG_BUY_BANK_SLOT_RESULT, 4);
            data << uint32(ERR_BANKSLOT_FAILED_TOO_MANY);
            caster->GetSession()->SendPacket(&data);
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
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->GetSource() && itr->GetSource()->GetDistance2d(caster) > 50.0f)
            {
                ChatHandler(caster->GetSession()).PSendSysMessage("Your friend must be nearby.");
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }
        }

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        Group* group = caster->GetGroup();
        if (!group || group->GetMembersCount() != 2)
        {
            return;
        }
        // First pass to reset instances regardless of distance
        caster->ResetInstances(INSTANCE_RESET_ALL, false);
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->GetSource())
                itr->GetSource()->ResetInstances(INSTANCE_RESET_ALL, false);
        }
        // Second pass to actually do the teleport
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->GetSource() && itr->GetSource()->GetDistance2d(caster) < 100.0f)
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

class spell_evokers_intellect_aura : public AuraScript
{
    PrepareAuraScript(spell_evokers_intellect_aura);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        uint32 spell = eventInfo.GetSpellInfo()->Id;

        if (std::find(uniqueSpells.begin(), uniqueSpells.end(), spell) != uniqueSpells.end())
            uniqueSpells.clear();

        uniqueSpells.emplace_back(eventInfo.GetSpellInfo()->Id);

        if (Aura* evokers = GetCaster()->GetAura(450002))
            evokers->SetStackAmount(uniqueSpells.size());
        else
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(450002))
            {
                AuraCreateInfo createInfo(spellInfo, MAX_EFFECT_MASK, GetCaster());
                createInfo.SetCaster(GetCaster());

                if (Aura* evoke = Aura::TryRefreshStackOrCreate(createInfo))
                    evoke->SetStackAmount(uniqueSpells.size());
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_evokers_intellect_aura::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }

    std::vector<uint32> uniqueSpells;
};

class spell_item_rename_character : public SpellScript
{
    PrepareSpellScript(spell_item_rename_character);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->HasAtLoginFlag(AT_LOGIN_RENAME))
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You are already have a pending rename.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* plrTarget = GetCaster()->ToPlayer())
        {
            plrTarget->SetAtLoginFlag(AT_LOGIN_RENAME);
            ChatHandler(plrTarget->GetSession()).PSendSysMessage("Logout to apply your rename.");
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_rename_character::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_rename_character::CheckRequirement);
    }
};

class spell_item_customize_character : public SpellScript
{
    PrepareSpellScript(spell_item_customize_character);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->HasAtLoginFlag(AT_LOGIN_CUSTOMIZE))
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You are already have a pending customization.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* plrTarget = GetCaster()->ToPlayer())
        {
            plrTarget->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
            ChatHandler(plrTarget->GetSession()).PSendSysMessage("Logout to apply your recustomization.");
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_customize_character::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_customize_character::CheckRequirement);
    }
};

class spell_item_faction_change_character : public SpellScript
{
    PrepareSpellScript(spell_item_faction_change_character);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->HasAtLoginFlag(AT_LOGIN_CHANGE_FACTION))
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You are already have a pending faction change.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* plrTarget = GetCaster()->ToPlayer())
        {
            plrTarget->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
            ChatHandler(plrTarget->GetSession()).PSendSysMessage("Logout to apply your faction change.");
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_faction_change_character::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_faction_change_character::CheckRequirement);
    }
};

class spell_item_change_race_character : public SpellScript
{
    PrepareSpellScript(spell_item_change_race_character);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (caster->HasAtLoginFlag(AT_LOGIN_CHANGE_RACE))
        {
            ChatHandler(caster->GetSession()).PSendSysMessage("You are already have a pending race change.");
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* plrTarget = GetCaster()->ToPlayer())
        {
            plrTarget->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
            ChatHandler(plrTarget->GetSession()).PSendSysMessage("Logout to apply your race change.");
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_change_race_character::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_item_change_race_character::CheckRequirement);
    }
};

class spell_item_metamorph_gem : public AuraScript
{
    PrepareAuraScript(spell_item_metamorph_gem);

    void HandleApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* pCaster = GetCaster()->ToPlayer())
        {
            pCaster->ToggleTempSpell(47241, GetId(), true);
            pCaster->ToggleTempSpell(50581, GetId(), true);
            pCaster->ToggleTempSpell(59671, GetId(), true);
            pCaster->ToggleTempSpell(54785, GetId(), true);
            pCaster->ToggleTempSpell(50589, GetId(), true);
        }

    }

    void HandleRemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* pCaster = GetCaster()->ToPlayer())
        {
            pCaster->ToggleTempSpell(47241, GetId(), false);
            pCaster->ToggleTempSpell(50581, GetId(), false);
            pCaster->ToggleTempSpell(59671, GetId(), false);
            pCaster->ToggleTempSpell(54785, GetId(), false);
            pCaster->ToggleTempSpell(50589, GetId(), false);
        }
    }


    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_item_metamorph_gem::HandleApplyEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectRemove += AuraEffectApplyFn(spell_item_metamorph_gem::HandleRemoveEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

class spell_tobenamedlegendary_dodge_thing : public AuraScript
{
    PrepareAuraScript(spell_tobenamedlegendary_dodge_thing);


    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetHitMask() & PROC_HIT_DODGE;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        CastSpellExtraArgs args(aurEff);
        args.AddSpellBP0(eventInfo.GetDamageInfo()->GetDamage() * 0.2);
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        eventInfo.GetActor()->CastSpell(eventInfo.GetActor(), 97401, args);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_tobenamedlegendary_dodge_thing::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_tobenamedlegendary_dodge_thing::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_item_require_virtual_item : public SpellScript
{
    PrepareSpellScript(spell_item_require_virtual_item);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        const Item* it = GetExplTargetItem();
        if (!it)
            return SPELL_FAILED_NO_VALID_TARGETS;

        // prevent disenchanting in trade slot
        if (it->GetOwnerGUID() != GetCaster()->GetGUID())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (it->IsBroken())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(it->GetEntry()))
        {
            if (vTemp->HasFlag(VIRTUAL_ITEM_FLAG_STATIC))
                return SPELL_FAILED_NO_VALID_TARGETS;
        }
        else
            return SPELL_FAILED_NO_VALID_TARGETS;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_require_virtual_item::CheckRequirement);
    }
};

class spell_item_require_virtual_item_or_heirloom : public SpellScript
{
    PrepareSpellScript(spell_item_require_virtual_item_or_heirloom);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        const Item* it = GetExplTargetItem();
        if (!it)
            return SPELL_FAILED_NO_VALID_TARGETS;

        // prevent disenchanting in trade slot
        if (it->GetOwnerGUID() != GetCaster()->GetGUID())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (it->IsBroken())
            return SPELL_FAILED_NO_VALID_TARGETS;


        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(it->GetEntry()))
        {
            if (vTemp->HasFlag(VIRTUAL_ITEM_FLAG_STATIC))
                return SPELL_FAILED_NO_VALID_TARGETS;
        }
        else if(it->GetTemplate()->Quality != ITEM_QUALITY_HEIRLOOM)
            return SPELL_FAILED_NO_VALID_TARGETS;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_require_virtual_item_or_heirloom::CheckRequirement);
    }
};

class spell_item_grant_fishing_quest : public SpellScript
{
    PrepareSpellScript(spell_item_grant_fishing_quest);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* plrTarget = GetCaster()->ToPlayer())
        {
            if (plrTarget->HasItemCount(62900) && plrTarget->HasItemCount(62901) && plrTarget->HasItemCount(62902))
            {
                if (plrTarget->GetQuestStatus(62900) == QUEST_STATUS_NONE)
                    plrTarget->AddQuest(sObjectMgr->GetQuestTemplate(62900), GetCaster());
            }
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_grant_fishing_quest::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_item_cataclysm : public SpellScript
{
    PrepareSpellScript(spell_item_cataclysm);

    void HandleImmolate(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), this->GetSpellInfo()->GetEffect(EFFECT_1).TriggerSpell, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_cataclysm::HandleImmolate, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_talent_thunderous_roar: public AuraScript
{
    PrepareAuraScript(spell_talent_thunderous_roar);

    void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
    {
        PreventDefaultAction();
        amount = CalculatePct(GetCaster()->GetMaxHealth(), aurEff->GetMiscValue()) / (aurEff->GetAmplitude() / 1000);
        canBeRecalculated = false;
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_talent_thunderous_roar::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

class spell_item_reroll_legendary : public SpellScript
{
    PrepareSpellScript(spell_item_reroll_legendary);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        const Item* it = GetExplTargetItem();
        if (!it)
            return SPELL_FAILED_NO_VALID_TARGETS;

        // prevent disenchanting in trade slot
        if (it->GetOwnerGUID() != GetCaster()->GetGUID())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (it->IsBroken())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(it->GetEntry()))
        {
            if (vTemp->HasFlag(VIRTUAL_ITEM_FLAG_STATIC))
                return SPELL_FAILED_NO_VALID_TARGETS;

            if(!vTemp->legendaryId || vTemp->HasFlag(VIRTUAL_ITEM_FLAG_LEGENDARY_REMOVED))
                return SPELL_FAILED_NO_VALID_TARGETS;
        }
        else
            return SPELL_FAILED_NO_VALID_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Item* itemTarget = GetExplTargetItem();
        Player* player = GetCaster()->ToPlayer();
        if (!itemTarget || !player)
            return;

        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(itemTarget->GetEntry()))
        {
            itemTarget->ToogleStats(false);
            for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
            {
                uint32 enchant_id = itemTarget->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if (!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;

                player->ApplyEnchantment(itemTarget, EnchantmentSlot(enchant_slot), false);
                itemTarget->SetEnchantment(EnchantmentSlot(enchant_slot), 0, 0, 0, player->GetGUID());
            }
            player->ApplyEnchantment(itemTarget, PRISMATIC_ENCHANTMENT_SLOT, false);

            itemTarget->SetEnchantment(PRISMATIC_ENCHANTMENT_SLOT, 0, 0, 0, player->GetGUID());

            for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                vTemp->Spells[i].SpellId = 0;
            }

            for (uint8 i = 0; i < MAX_GEM_SOCKETS; ++i)
            {
                vTemp->Socket[i].Color = 0;
                vTemp->Socket[i].Content = 0;
            }

            uint32 oldLegId = vTemp->legendaryId;
            vTemp->legendaryId = 0;
            VirtualModifier modifier = sVirtualItemMgr.GetModifierFromTemplate(vTemp);;
            modifier.legendaryOverride = 0;
            modifier.legendarySeed = urand(std::numeric_limits<uint32>::min(), std::numeric_limits<uint32>::max());

            sVirtualItemMgr.InitSeedGen(modifier);
            sVirtualItemMgr.GenerateQuality(vTemp, modifier);
            sVirtualItemMgr.GenerateLegendaryItemEffect(vTemp, modifier, oldLegId);
            sVirtualItemMgr.GenerateStatGroup(vTemp, modifier);
            sVirtualItemMgr.GenerateBaseStats(vTemp, modifier);
            sVirtualItemMgr.GenerateItemStats(vTemp, modifier);
            sVirtualItemMgr.GenerateSockets(vTemp, modifier);
            sVirtualItemMgr.GenerateItemName(vTemp, modifier);
            sVirtualItemMgr.GenerateSpells(vTemp, modifier);
            sVirtualItemMgr.GenerateItemDisplay(vTemp, modifier);
            sVirtualItemMgr.UpdateDisenchantId(vTemp, modifier);
            if (vTemp->honeLevel > 0)
                sVirtualItemMgr.UpdateHoneDisplaySpell(vTemp);

            player->ApplyVirtualItemLegendayEffects(itemTarget);

            vTemp->seed = modifier.seed;
            vTemp->displaySeed = modifier.displaySeed;
            vTemp->nameSeed = modifier.nameSeed;
            vTemp->socketSeed = modifier.socketSeed;
            vTemp->spellSeed = modifier.spellSeed;
            vTemp->statSeed = modifier.statSeed;
            vTemp->statValueSeed = modifier.statValueSeed;
            vTemp->statGroupSeed = modifier.statGroupSeed;
            if(!vTemp->HasFlag(VIRTUAL_ITEM_FLAG_LEGENDARY_OVERRIDE))
                vTemp->customFlags &= VIRTUAL_ITEM_FLAG_LEGENDARY_OVERRIDE;
            vTemp->InitializeQueryData();
            WorldPacket response = vTemp->BuildQueryData(LOCALE_enUS);
            sWorld->SendGlobalMessage(&response);
            itemTarget->SaveVirtualItemInfo();
            itemTarget->ToogleStats(true);
        }

        
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_reroll_legendary::CheckRequirement);
        OnEffectHit += SpellEffectFn(spell_item_reroll_legendary::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_item_remove_legendary : public SpellScript
{
    PrepareSpellScript(spell_item_remove_legendary);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        const Item* it = GetExplTargetItem();
        if (!it)
            return SPELL_FAILED_NO_VALID_TARGETS;

        // prevent disenchanting in trade slot
        if (it->GetOwnerGUID() != GetCaster()->GetGUID())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (it->IsBroken())
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(it->GetEntry()))
        {
            if (vTemp->HasFlag(VIRTUAL_ITEM_FLAG_STATIC))
                return SPELL_FAILED_NO_VALID_TARGETS;

            if (!vTemp->legendaryId || vTemp->HasFlag(VIRTUAL_ITEM_FLAG_LEGENDARY_REMOVED))
                return SPELL_FAILED_NO_VALID_TARGETS;
        }
        else
            return SPELL_FAILED_NO_VALID_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Item* itemTarget = GetExplTargetItem();
        Player* player = GetCaster()->ToPlayer();
        if (!itemTarget || !player)
            return;

        if (VirtualItemTemplate* vTemp = sVirtualItemMgr.GetVirtualTemplate(itemTarget->GetEntry()))
        {
            itemTarget->ToogleStats(false);
            for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
            {
                uint32 enchant_id = itemTarget->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if (!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;

                player->ApplyEnchantment(itemTarget, EnchantmentSlot(enchant_slot), false);
                itemTarget->SetEnchantment(EnchantmentSlot(enchant_slot), 0, 0, 0, player->GetGUID());
            }
            player->ApplyEnchantment(itemTarget, PRISMATIC_ENCHANTMENT_SLOT, false);

            itemTarget->SetEnchantment(PRISMATIC_ENCHANTMENT_SLOT, 0, 0, 0, player->GetGUID());

            for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                vTemp->Spells[i].SpellId = 0;
            }

            for (uint8 i = 0; i < MAX_GEM_SOCKETS; ++i)
            {
                vTemp->Socket[i].Color = 0;
                vTemp->Socket[i].Content = 0;
            }
            uint32 oldLegId = vTemp->legendaryId;
            vTemp->legendaryId = 0;
            vTemp->customFlags &= VIRTUAL_ITEM_FLAG_LEGENDARY_REMOVED;
            VirtualModifier modifier = sVirtualItemMgr.GetModifierFromTemplate(vTemp);;
            modifier.legendaryOverride = 0;

            sVirtualItemMgr.InitSeedGen(modifier);
            sVirtualItemMgr.GenerateQuality(vTemp, modifier);
            sVirtualItemMgr.GenerateStatGroup(vTemp, modifier);
            sVirtualItemMgr.GenerateBaseStats(vTemp, modifier);
            sVirtualItemMgr.GenerateItemStats(vTemp, modifier);
            sVirtualItemMgr.GenerateSockets(vTemp, modifier);
            sVirtualItemMgr.GenerateItemName(vTemp, modifier);
            sVirtualItemMgr.GenerateSpells(vTemp, modifier);
            sVirtualItemMgr.GenerateItemDisplay(vTemp, modifier);
            sVirtualItemMgr.UpdateDisenchantId(vTemp, modifier);
            if (vTemp->honeLevel > 0)
                sVirtualItemMgr.UpdateHoneDisplaySpell(vTemp);

            vTemp->seed = modifier.seed;
            vTemp->displaySeed = modifier.displaySeed;
            vTemp->nameSeed = modifier.nameSeed;
            vTemp->socketSeed = modifier.socketSeed;
            vTemp->spellSeed = modifier.spellSeed;
            vTemp->statSeed = modifier.statSeed;
            vTemp->statValueSeed = modifier.statValueSeed;
            vTemp->statGroupSeed = modifier.statGroupSeed;

            vTemp->InitializeQueryData();
            WorldPacket response = vTemp->BuildQueryData(LOCALE_enUS);
            sWorld->SendGlobalMessage(&response);
            itemTarget->SaveVirtualItemInfo();
            itemTarget->ToogleStats(true);
            player->AddItem(62903, 1);
        }


    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_remove_legendary::CheckRequirement);
        OnEffectHit += SpellEffectFn(spell_item_remove_legendary::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_item_swapblaster : public SpellScript
{
    PrepareSpellScript(spell_item_swapblaster);

    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    SpellCastResult CheckRequirement()
    {
        if (Unit* u = GetExplTargetUnit())
            if (u->IsPlayer())
                return SPELL_CAST_OK;
        return SPELL_FAILED_TARGET_NOT_PLAYER;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Position targ = GetHitUnit()->GetPosition();
        Position cast = GetCaster()->GetPosition();
        GetHitUnit()->NearTeleportTo(cast);
        GetCaster()->NearTeleportTo(targ);
        //require double confirm?
        //GetHitUnit()->ToPlayer()->SendSummonRequestFrom(GetCaster());
        //GetCaster()->ToPlayer()->SendSummonRequestFrom(GetHitUnit());
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_swapblaster::CheckRequirement);
        OnEffectHit += SpellEffectFn(spell_item_swapblaster::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
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
    RegisterSpellScript(spell_evokers_intellect_aura);
    RegisterSpellScript(spell_item_rename_character);
    RegisterSpellScript(spell_item_customize_character);
    RegisterSpellScript(spell_item_faction_change_character);
    RegisterSpellScript(spell_item_change_race_character);
    RegisterSpellScript(spell_item_metamorph_gem);
    RegisterSpellScript(spell_tobenamedlegendary_dodge_thing);
    RegisterSpellScript(spell_item_require_virtual_item);
    RegisterSpellScript(spell_item_require_virtual_item_or_heirloom);
    RegisterSpellScript(spell_item_grant_fishing_quest);
    RegisterSpellScript(spell_item_cataclysm);
    RegisterSpellScript(spell_talent_thunderous_roar);
    RegisterSpellScript(spell_item_reroll_legendary);
    RegisterSpellScript(spell_item_remove_legendary);
}
