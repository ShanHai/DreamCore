#include "dc_composer.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "DCMgr.h"
#include "DCComposeModule.h"
#include "DCHelper.h"

#define CHECK_REQUIREMENT(p, reqID) (!sDCMgr->getConfigBool(DC_CONFIG_COMPOSE_CHECK_REQ)  || sDCMgr->CheckRequirement(p, reqID))
#define CHECK_COST(p, cosID)        (!sDCMgr->getConfigBool(DC_CONFIG_COMPOSE_CHECK_COST) || sDCMgr->CheckCost(p, cosID))

void SendComposeInfo(Player* player, uint64 guid, uint32 textID, const ComposeDefinition* definition)
{
    auto costInfo = std::string("");
    auto lootInfo = std::string("");

    auto cost   = sDCMgr->GetDCCost(definition->costID);
    auto locale = player->GetSession()->GetSessionDbLocaleIndex();
    if (cost)
    {
        for (uint8 i = 0; i < cost->lastItemIndex; ++i)
        {
            if (!cost->items[i])
                continue;

            auto item = sObjectMgr->GetItemTemplate(cost->items[i]);

            if (!item)
                continue;

            auto itemName = item->Name1;
            if (locale >= 0)
            {
                if (auto il = sObjectMgr->GetItemLocale(item->ItemId))
                    ObjectMgr::GetLocaleString(il->Name, locale, itemName);
            }

            costInfo.append(" ");
            costInfo.append(sDCMgr->BuildDCText(STRING_ITEM_INFO_COUNT, "000000", sDCMgr->GetDCIconName(item->DisplayInfoID), 30, 30, 0, -18, itemName.c_str(), "00ff00", cost->itemsCount[i]));
            costInfo.append("\r\n");
        }
    }

    auto bounds = sDCComposeModule->GetComposeLootMapBounds(definition->lootID);
    if (bounds.first != bounds.second)
    {
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
        {
            auto item = sObjectMgr->GetItemTemplate(itr->second.item);
            if (!item)
                continue;

            auto itemName = item->Name1;
            if (locale >= 0)
            {
                if (auto il = sObjectMgr->GetItemLocale(item->ItemId))
                    ObjectMgr::GetLocaleString(il->Name, locale, itemName);
            }

            lootInfo.append(" ");
            lootInfo.append(sDCMgr->BuildDCText(STRING_ITEM_INFO, "000000", sDCMgr->GetDCIconName(item->DisplayInfoID), 30, 30, 0, cost ? -18 : -10, itemName.c_str()));
            lootInfo.append("\r\n");
        }
    }

    auto composeInfo = sDCMgr->BuildDCText(STRING_COMPOSE_INFO, costInfo.c_str(), lootInfo.c_str());

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 1500);
    data << textID;
    data << float(0);
    data << composeInfo.c_str();
    data << "";
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);

    player->GetSession()->SendPacket(&data);
}

bool SendComposeMenuItems(Player* player, uint16 category, ObjectGuid guid, uint32 page = 0)
{
    player->PlayerTalkClass->ClearMenus();

    auto bounds   = sDCComposeModule->GetComposeDefinitionsMapBounds(category);
    auto distance = std::distance(bounds.first, bounds.second);

    if (!distance)
        return false;

    auto countPerPage = sDCMgr->getConfigInt(DC_CONFIG_COMPOSE_MENUITEMS_PER_PAGE);
    if (countPerPage * page >= distance)
        page = distance / countPerPage;

    auto itr  = bounds.first;
    auto cost = sDCMgr->getConfigBool(DC_CONFIG_COMPOSE_CHECK_COST);
    auto info = DC::compressUint32(category, page);

    std::advance(itr, page * countPerPage);

    for (auto i = 0u; i < countPerPage && itr != bounds.second; ++i, ++itr)
        player->ADD_GOSSIP_ITEM_EXTENDED(itr->second.iconID, itr->second.text, DC::compressUint32(itr->second.category, itr->second.id), info, "", 0, 0);

    if ((page + 1) * countPerPage < distance)
    {
        auto def = sDCComposeModule->GetComposeDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_NEXT_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_NEXT_PAGE, info);
    }

    if (page)
    {
        auto def = sDCComposeModule->GetComposeDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_PREV_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE, info);
    }

    if (category)
    {
        auto def = sDCComposeModule->GetComposeDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_MAIN_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_MAIN_PAGE, page);
    }

    auto textID = DEFAULT_MENU_TEXTID;

    auto g = sDCComposeModule->GetComposeCategory(category);
    if (g && sObjectMgr->GetGossipText(g->textID))
        textID = g->textID;

    player->SEND_GOSSIP_MENU(textID, guid);

    return true;
}

bool SendComposeDefinitionInfo(Player* player, const ComposeDefinition* definition, ObjectGuid guid, uint16 category, uint32 page)
{
    player->PlayerTalkClass->ClearMenus();

    if (!definition->lootID)
        return false;

    auto bounds = sDCComposeModule->GetComposeLootMapBounds(definition->lootID);
    if (bounds.first == bounds.second)
    {
        TC_LOG_ERROR("dc.compose", "Client send unknow compose loot id. player [%s], category[%u], loot[%u]", player->GetName(), definition->category, definition->lootID);
        return false;
    }

    SendComposeInfo(player, guid, COMPOSE_INFO, definition);

    const ComposeDefinition *def = nullptr;
    def = sDCComposeModule->GetComposeDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_COMPOSE);
    if (def)
        player->ADD_GOSSIP_ITEM_EXTENDED(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_COMPOSE, DC::compressUint32(definition->category, definition->id), sDCMgr->BuildFullCostText(definition->costID), 0, 0);

    def = sDCComposeModule->GetComposeDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_CANCLE);
    if (def)
        player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_CANCLE, DC::compressUint32(category, page));

    player->SEND_GOSSIP_MENU(COMPOSE_INFO, guid);

    return true;
}

bool DoCompose(Player* player, const ComposeDefinition* definition)
{
    player->CLOSE_GOSSIP_MENU();

    auto bounds = sDCComposeModule->GetComposeLootMapBounds(definition->lootID);
    if (bounds.first == bounds.second)
    {
        TC_LOG_ERROR("dc.compose", "Client send unknow compose loot id. player [%s], category[%u], id[%u], lootID[%u]", player->GetName(), definition->category, definition->id, definition->lootID);
        return false;
    }

    auto chatHandler = ChatHandler(player->GetSession());
    auto msg         = "";
    auto limit       = definition->lootCountLimit ? definition->lootCountLimit : (uint8)sDCMgr->getConfigInt(DC_CONFIG_COMPOSE_DEFAULT_LOOT_LIMIT);

    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, ItemPosCountVec(), 25, limit) != EQUIP_ERR_OK)
    {
        msg = sDCMgr->GetDCString(STRING_NOT_ENOUGH_BAG_SPACE, LOCALE_zhCN);
        chatHandler.SendSysMessage(msg);

        player->GetSession()->SendNotification(msg);

        return false;
    }

    if (!CHECK_REQUIREMENT(player, definition->reqID) || !CHECK_COST(player, definition->costID))
    {
        msg = sDCMgr->GetDCString(STRING_COMPOSE_CONDITIONS, LOCALE_zhCN);
        chatHandler.SendSysMessage(msg);

        player->GetSession()->SendNotification(msg);

        return false;
    }

    if (definition->costID && sDCMgr->getConfigBool(DC_CONFIG_COMPOSE_CHECK_COST))
    {
        auto cost = sDCMgr->GetDCCost(definition->costID);
        if (!sDCMgr->DoCost(player, cost))
            return false;
    }

    auto selectedResults = std::unordered_map<uint8, std::unordered_map<uint32, uint8>>();
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (!sDCMgr->CheckRequirement(player, itr->second.reqID))
            continue;

        if (itr->second.chance >= 100.0f || itr->second.chance >= (float)rand_chance())
            selectedResults[itr->second.lootGroup][itr->second.item] = urand(itr->second.minCount, itr->second.maxCount);
    }

    if (!selectedResults.size())
    {
        msg = sDCMgr->GetDCString(STRING_COMPOSE_FAILED, LOCALE_zhCN);
        chatHandler.SendSysMessage(msg);

        player->GetSession()->SendNotification(msg);

        return true;
    }

    for (uint8 i = 0; i < limit; ++i)
    {
        if (!selectedResults.size())
            break;

        auto itrGroup = Trinity::Containers::SelectRandomContainerElement(selectedResults);
        if (!itrGroup.second.size())
        {
            selectedResults.erase(itrGroup.first);
            continue;
        }

        auto itrItem  = Trinity::Containers::SelectRandomContainerElement(itrGroup.second);

        if (itrGroup.first)
            selectedResults.erase(itrGroup.first);
        else
            selectedResults[0].erase(itrItem.first);

        player->AddItem(itrItem.first, itrItem.second);
    }

    selectedResults.clear();

    return true;
}

bool OnComposeGossIpSelect(Player* player, ObjectGuid guid, uint32 sender, uint32 action)
{
    auto category = 0u;
    auto page     = 0u;
    DC::decompressUint32(action, category, page);

    switch (sender)
    {
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE:
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_NEXT_PAGE:
        {
            page += (sender == GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE ? (page ? -1 : 0) : 1);

            return SendComposeMenuItems(player, category, guid, page);
        }
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_MAIN_PAGE:
            return SendComposeMenuItems(player, DEFAULT_MENU_CATEGORY, guid);
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_COMPOSE:
        {
            auto def = sDCComposeModule->GetComposeDefinition(category, page);
            if (!def)
                return false;

            return DoCompose(player, def);
        }
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_CANCLE:
        {
            return SendComposeMenuItems(player, category, guid, page);
        }
        default:
            break;
    }

    player->PlayerTalkClass->ClearMenus();

    auto id = 0u;
    DC::decompressUint32(sender, category, id);
    auto def = sDCComposeModule->GetComposeDefinition(category, id);
    if (!def)
    {
        TC_LOG_ERROR("dc.compose", "Client send unknow compose category. player [%s], category[%u], id[%u]", player->GetName(), category, id);
        return false;
    }

    if (def->subCategory)
        return SendComposeMenuItems(player, def->subCategory, guid);

    return SendComposeDefinitionInfo(player, def, guid, category, page);
}

class ComposerCreature : public CreatureScript
{
    public:
        ComposerCreature() :
            CreatureScript("dc_composer_creature")
        {
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            return SendComposeMenuItems(player, DEFAULT_MENU_CATEGORY, creature->GetGUID());
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
        {
            return OnComposeGossIpSelect(player, creature->GetGUID(), sender, action);
        }

        bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
        {
            return true;
        }

    private:
};

class ComposerItem : public ItemScript
{
    public:
        ComposerItem() :
            ItemScript("dc_composer_item")
        {
        }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
        {
            return SendComposeMenuItems(player, DEFAULT_MENU_CATEGORY, item->GetGUID());
        }

        bool OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
        {
            return OnComposeGossIpSelect(player, item->GetGUID(), sender, action);
        }

        bool OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) override
        {
            return true;
        }
};

class ComposerGameObject : public GameObjectScript
{
    public:
        ComposerGameObject() :
            GameObjectScript("dc_composer_gameobject")
        {

        }
};

void AddSC_DC_composer()
{
    new ComposerCreature();
    new ComposerItem();
    new ComposerGameObject();
}