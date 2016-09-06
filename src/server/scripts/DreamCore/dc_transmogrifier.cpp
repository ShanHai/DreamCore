#include "ScriptPCH.h"
#include "Config.h"
#include "Language.h"
#include "DCMgr.h"
#include "DCTransmogModule.h"

#define GDCS session->GetDCString

class Transmogrifier : public CreatureScript
{
public:
    Transmogrifier() : CreatureScript("dc_transmogrify") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        auto session = player->GetSession();
        if (sDCTransmogModule->EnableTransmogInfo)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Misc_Book_11:30:30:-18:0|tHow transmogrification works", EQUIPMENT_SLOT_END + 9, 0);

        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (auto slotName = sDCTransmogModule->GetSlotName(slot, session))
            {
                auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                auto entry   = newItem ? newItem->GetFakeEntry() : 0;
                auto icon    = entry ? sDCTransmogModule->GetItemIcon(entry, 30, 30, -18, 0) : sDCTransmogModule->GetSlotIcon(slot, 30, 30, -18, 0);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, icon + std::string(slotName), EQUIPMENT_SLOT_END, slot);
            }
        }

        if (sDCTransmogModule->EnableSets)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/RAIDFRAME/UI-RAIDFRAME-MAINASSIST:30:30:-18:0|tManage sets", EQUIPMENT_SLOT_END + 4, 0);

        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Enchant_Disenchant:30:30:-18:0|tRemove all transmogrifications", EQUIPMENT_SLOT_END + 2, 0, "Remove transmogrifications from all equipped items?", 0, false);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30:30:-18:0|tUpdate menu", EQUIPMENT_SLOT_END + 1, 0);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        auto session = player->GetSession();

        switch (sender)
        {
            case EQUIPMENT_SLOT_END:     ShowTransmogItems(player, creature, action); break;  // Show items you can use
            case EQUIPMENT_SLOT_END + 1: OnGossipHello(player, creature); break;              // Main menu
            case EQUIPMENT_SLOT_END + 2: // Remove Transmogrifications
            {
                bool removed = false;
                for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    if (auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        if (!newItem->GetFakeEntry())
                            continue;
                        sDCTransmogModule->DeleteFakeEntry(player, newItem);
                        removed = true;
                    }
                }
                if (removed)
                    session->SendAreaTriggerMessage(GDCS(STRING_UNTRANSMOG_SUCCESS));
                else
                    session->SendNotification(GDCS(STRING_UNTRANSMOG_NO_TRANSMOGS));

                OnGossipHello(player, creature);

                break;
            }
            case EQUIPMENT_SLOT_END + 3: // Remove Transmogrification from single item
            {
                if (auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action))
                {
                    if (newItem->GetFakeEntry())
                    {
                        sDCTransmogModule->DeleteFakeEntry(player, newItem);
                        session->SendAreaTriggerMessage(GDCS(STRING_UNTRANSMOG_SUCCESS));
                    }
                    else
                        session->SendNotification(GDCS(STRING_UNTRANSMOG_NO_TRANSMOGS));
                }

                OnGossipSelect(player, creature, EQUIPMENT_SLOT_END, action);

                break;
            }
            case EQUIPMENT_SLOT_END + 4: // Presets menu
            {
                if (!sDCTransmogModule->EnableSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }
                if (sDCTransmogModule->EnableSetInfo)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Misc_Book_11:30:30:-18:0|tHow sets work", EQUIPMENT_SLOT_END + 10, 0);

                if (!player->presetMap.empty())
                {
                    for (auto it = player->presetMap.begin(); it != player->presetMap.end(); ++it)
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Misc_Statue_02:30:30:-18:0|t" + it->second.name, EQUIPMENT_SLOT_END + 6, it->first);

                    if (player->presetMap.size() < sDCTransmogModule->MaxSets)
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/GuildBankFrame/UI-GuildBankFrame-NewTab:30:30:-18:0|tSave set", EQUIPMENT_SLOT_END + 8, 0);
                }
                else
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/GuildBankFrame/UI-GuildBankFrame-NewTab:30:30:-18:0|tSave set", EQUIPMENT_SLOT_END + 8, 0);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 1, 0);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

                break;
            }
            case EQUIPMENT_SLOT_END + 5: // Use preset
            {
                if (!sDCTransmogModule->EnableSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                auto it = player->presetMap.find(action);
                if (it != player->presetMap.end())
                {
                    for (auto it2 = it->second.slotMap.begin(); it2 != it->second.slotMap.end(); ++it2)
                        if (auto item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, it2->first))
                            sDCTransmogModule->PresetTransmog(player, item, it2->second, it2->first);
                }

                OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 6, action);

                break;
            }
            case EQUIPMENT_SLOT_END + 6: // view preset
            {
                if (!sDCTransmogModule->EnableSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                auto it = player->presetMap.find(action);
                if (it == player->presetMap.end())
                {
                    OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
                    return true;
                }

                for (auto it2 = it->second.slotMap.begin(); it2 != it->second.slotMap.end(); ++it2)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, sDCTransmogModule->GetItemIcon(it2->second, 30, 30, -18, 0) + sDCTransmogModule->GetItemLink(it2->second, session), sender, action);

                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Misc_Statue_02:30:30:-18:0|tUse set", EQUIPMENT_SLOT_END + 5, action, "Using this set for transmogrify will bind transmogrified items to you and make them non-refundable and non-tradeable.\nDo you wish to continue?\n\n" + it->second.name, 0, false);
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "|TInterface/PaperDollInfoFrame/UI-GearManager-LeaveItem-Opaque:30:30:-18:0|tDelete set", EQUIPMENT_SLOT_END + 7, action, "Are you sure you want to delete " + it->second.name + "?", 0, false);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 4, 0);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

                break;
            }
            case EQUIPMENT_SLOT_END + 7: // Delete preset
            {
                if (!sDCTransmogModule->EnableSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                player->presetMap.erase(action);

                OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);

                break;
            }
            case EQUIPMENT_SLOT_END + 8: // Save preset
            {
                if (!sDCTransmogModule->EnableSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                if (player->presetMap.size() >= sDCTransmogModule->MaxSets)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                uint32 cost = 0;
                bool canSave = false;
                for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    if (!sDCTransmogModule->GetSlotName(slot, session))
                        continue;

                    if (auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        auto entry = newItem->GetFakeEntry();
                        if (!entry)
                            continue;
                        auto temp = sObjectMgr->GetItemTemplate(entry);
                        if (!temp)
                            continue;
                        if (!sDCTransmogModule->SuitableForTransmogrification(player, temp)) // no need to check?
                            continue;
                        cost += sDCTransmogModule->GetSpecialPrice(temp);
                        canSave = true;
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, sDCTransmogModule->GetItemIcon(entry, 30, 30, -18, 0) + sDCTransmogModule->GetItemLink(entry, session), EQUIPMENT_SLOT_END + 8, 0);
                    }
                }

                if (canSave)
                    player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "|TInterface/GuildBankFrame/UI-GuildBankFrame-NewTab:30:30:-18:0|tSave set", 0, 0, "Insert set name", cost*sDCTransmogModule->SetCostModifier + sDCTransmogModule->SetCopperCost, true);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30:30:-18:0|tUpdate menu", sender, action);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 4, 0);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

                break;
            }
            case EQUIPMENT_SLOT_END + 10: // Set info
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 4, 0);
                player->SEND_GOSSIP_MENU(sDCTransmogModule->SetNpcText, creature->GetGUID());

                break;
            }
            case EQUIPMENT_SLOT_END + 9: // Transmog info
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 1, 0);
                player->SEND_GOSSIP_MENU(sDCTransmogModule->TransmogNpcText, creature->GetGUID());

                break;
            }
            default: // Transmogrify
            {
                if (!sender && !action)
                {
                    OnGossipHello(player, creature);
                    return true;
                }

                auto res = sDCTransmogModule->Transmogrify(player, ObjectGuid(HighGuid::Item, 0, action), sender);
                if (res == STRING_TRANSMOG_SUCCESS)
                    session->SendAreaTriggerMessage(GDCS(STRING_TRANSMOG_SUCCESS));
                else
                    session->SendNotification(res);

                OnGossipSelect(player, creature, EQUIPMENT_SLOT_END, sender);

                break;
            }
        }

        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
    {
        player->PlayerTalkClass->ClearMenus();
        auto session = player->GetSession();

        if (sender || action) return true; // should never happen

        if (!sDCTransmogModule->EnableSets)
        {
            OnGossipHello(player, creature);
            return true;
        }

        // Allow only alnum
        std::string name = code;
        static auto allowedcharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz _.,'1234567890";

        if (!name.length() || name.find_first_not_of(allowedcharacters) != std::string::npos)
        {
            player->GetSession()->SendNotification(GDCS(STRING_TRANSMOG_INVALID_PRESET_NAME));
            OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
            return true;
        }

        int32 cost = 0;
        std::map<uint8, uint32> items;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (!sDCTransmogModule->GetSlotName(slot, player->GetSession()))
                continue;

            if (auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                auto entry = newItem->GetFakeEntry();
                if (!entry)
                    continue;
                auto temp = sObjectMgr->GetItemTemplate(entry);
                if (!temp)
                    continue;
                if (!sDCTransmogModule->SuitableForTransmogrification(player, temp))
                    continue;
                cost += sDCTransmogModule->GetSpecialPrice(temp);
                items[slot] = entry;
            }
        }

        if (!items.empty())
        {
            cost *= sDCTransmogModule->SetCostModifier;
            cost += sDCTransmogModule->SetCopperCost;

            if (!player->HasEnoughMoney(cost))
            {
                player->GetSession()->SendNotification(GDCS(STRING_TRANSMOG_NOT_ENOUGH_MONEY));
            }
            else
            {
                uint8 presetID = sDCTransmogModule->MaxSets;
                if (player->presetMap.size() < sDCTransmogModule->MaxSets)
                {
                    for (uint8 i = 0; i < sDCTransmogModule->MaxSets; ++i) // should never reach over max
                    {
                        if (player->presetMap.find(i) == player->presetMap.end())
                        {
                            presetID = i;
                            break;
                        }
                    }
                }

                if (presetID < sDCTransmogModule->MaxSets)
                {
                    // Make sure code doesnt mess up SQL!
                    player->presetMap[presetID].name = name;
                    player->presetMap[presetID].slotMap = items;

                    if (cost)
                        player->ModifyMoney(-cost);
                }
            }
        }

        OnGossipSelect(player, creature, EQUIPMENT_SLOT_END + 4, 0);
        return true;
    }

    void ShowTransmogItems(Player* player, Creature* creature, uint8 slot) // Only checks bags while can use an item from anywhere in inventory
    {
        auto session = player->GetSession();
        auto oldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (oldItem)
        {
            uint32 limit = 0;
            uint32 price = sDCTransmogModule->GetSpecialPrice(oldItem->GetTemplate());

            price *= sDCTransmogModule->ScaledCostModifier;
            price += sDCTransmogModule->CopperCost;
            std::ostringstream ss;
            ss << std::endl;
            if (sDCTransmogModule->RequireToken)
                ss << std::endl << std::endl << sDCTransmogModule->TokenAmount << " x " << sDCTransmogModule->GetItemLink(sDCTransmogModule->TokenEntry, session);

            for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            {
                if (limit >= MAX_PRESETS_LIMIT)
                    break;
                auto newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (!newItem)
                    continue;
                if (!sDCTransmogModule->CanTransmogrifyItemWithItem(player, oldItem->GetTemplate(), newItem->GetTemplate()))
                    continue;
                if (oldItem->GetFakeEntry() == newItem->GetEntry())
                    continue;
                ++limit;
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, sDCTransmogModule->GetItemIcon(newItem->GetEntry(), 30, 30, -18, 0) + sDCTransmogModule->GetItemLink(newItem, session), slot, newItem->GetGUID().GetCounter(), "Using this item for transmogrify will bind it to you and make it non-refundable and non-tradeable.\nDo you wish to continue?\n\n" + sDCTransmogModule->GetItemIcon(newItem->GetEntry(), 40, 40, -15, -10) + sDCTransmogModule->GetItemLink(newItem, session) + ss.str(), price, false);
            }

            for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                auto bag = player->GetBagByPos(i);
                if (!bag)
                    continue;
                for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                {
                    if (limit >= MAX_PRESETS_LIMIT)
                        break;
                    auto newItem = player->GetItemByPos(i, j);
                    if (!newItem)
                        continue;
                    if (!sDCTransmogModule->CanTransmogrifyItemWithItem(player, oldItem->GetTemplate(), newItem->GetTemplate()))
                        continue;
                    if (oldItem->GetFakeEntry() == newItem->GetEntry())
                        continue;
                    ++limit;
                    player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, sDCTransmogModule->GetItemIcon(newItem->GetEntry(), 30, 30, -18, 0) + sDCTransmogModule->GetItemLink(newItem, session), slot, newItem->GetGUID().GetCounter(), "Using this item for transmogrify will bind it to you and make it non-refundable and non-tradeable.\nDo you wish to continue?\n\n" + sDCTransmogModule->GetItemIcon(newItem->GetEntry(), 40, 40, -15, -10) + sDCTransmogModule->GetItemLink(newItem, session) + ss.str(), price, false);
                }
            }
        }

        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/INV_Enchant_Disenchant:30:30:-18:0|tRemove transmogrification", EQUIPMENT_SLOT_END + 3, slot, "Remove transmogrification from the slot?", 0, false);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30:30:-18:0|tUpdate menu", EQUIPMENT_SLOT_END, slot);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack..", EQUIPMENT_SLOT_END + 1, 0);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }
};

void AddSC_DC_transmog()
{
    new Transmogrifier();
}