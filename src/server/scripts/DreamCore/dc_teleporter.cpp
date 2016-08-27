#include "dc_teleporter.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "DCMgr.h"
#include "DCTeleportModule.h"
#include "DCHelper.h"

#define CHECK_REQUIREMENT(p, reqID) (!sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_REQ)  || sDCMgr->CheckRequirement(p, reqID))
#define CHECK_COST(p, cosID)        (!sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_COST) || sDCMgr->CheckCost(p, cosID))

bool CheckVisible(const Player* player, const TeleportDefinition* definition)
{
    if (!sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_VISIBLE) || player->IsGameMaster() || !definition->reqID)
        return true;

    return CHECK_REQUIREMENT(player, definition->reqID);
}

auto GetVisibleDefinitions(TeleportDefinitionContainer& container, const Player* player, uint32 category = 0)
{
    auto bounds = sDCTeleportModule->GetTeleportDefinitionsMapBounds(category);
    if (!sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_VISIBLE) || player->IsGameMaster())
        return bounds;

    for (auto itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (CheckVisible(player, &itr->second))
            container.insert(TeleportDefinitionContainer::value_type(category, itr->second));
    }

    bounds = container.equal_range(category);
    return bounds;
}

bool SendTeleportMenuItems(Player* player, uint32 category, ObjectGuid guid, uint32 page = 0)
{
    player->PlayerTalkClass->ClearMenus();

    TeleportDefinitionContainer container;
    auto bounds   = GetVisibleDefinitions(container, player, category);
    auto distance = std::distance(bounds.first, bounds.second);

    if (!distance)
        return false;

    auto countPerPage = sDCMgr->getConfigInt(DC_CONFIG_TELEPORT_MENUITEMS_PER_PAGE);
    if (countPerPage * page >= distance)
        page = distance / countPerPage;

    auto itr       = bounds.first;
    auto checkCost = sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_COST);

    std::advance(itr, page * countPerPage);

    for (auto i = 0u; i < countPerPage && itr != bounds.second; ++i, ++itr)
    {
        auto boxText = itr->second.boxText;
        if (!sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_REMIND) && (!checkCost || !itr->second.costID))
            boxText = "";
        else if (!itr->second.customText && !checkCost && itr->second.costID)
        {
            auto tele = sObjectMgr->GetGameTele(itr->second.teleID);
            boxText = tele ? sDCMgr->BuildDCText(STRING_TELEPORT_REMIND, tele->name.c_str()) : "";
        }
        auto cost = sDCMgr->GetDCCost(itr->second.costID);
        player->ADD_GOSSIP_ITEM_EXTENDED(itr->second.iconID, itr->second.text, itr->second.category, itr->second.id, boxText, checkCost && cost ? cost->money : 0, 0);
    }

    if ((page + 1) * countPerPage < distance)
    {
        auto def = sDCTeleportModule->GetTeleportDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_NEXT_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_NEXT_PAGE, DC::compressUint32(category, page));
    }

    if (page)
    {
        auto def = sDCTeleportModule->GetTeleportDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_PREV_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE, DC::compressUint32(category, page));
    }

    if (category)
    {
        auto def = sDCTeleportModule->GetTeleportDefinition(GLOBAL_ITEM_CATEGORY, MENU_SENDER_MAIN_PAGE);
        if (def)
            player->ADD_GOSSIP_ITEM(def->iconID, def->text, GLOBAL_ITEM_CATEGORY + MENU_SENDER_MAIN_PAGE, page);
    }

    auto textID = DEFAULT_MENU_TEXTID;

    auto cat = sDCTeleportModule->GetTeleportCategory(category);
    if (cat && sObjectMgr->GetGossipText(cat->textID))
        textID = cat->textID;

    player->SEND_GOSSIP_MENU(textID, guid);

    return true;
}

bool DoTeleport(Player* player, const TeleportDefinition* definition)
{
    if (!player->IsInWorld())
        return false;

    auto tele = sObjectMgr->GetGameTele(definition->teleID);
    if (!tele)
    {
        TC_LOG_ERROR("dc.teleport", "Client send unknow teleport target id. player [%s], category[%u], teleID[%u]", player->GetName(), definition->category, definition->teleID);
        return false;
    }

    auto chatHandler = ChatHandler(player->GetSession());
    auto msg         = "";

    if (player->IsInCombat() || player->IsFlying())
    {
        msg = sDCMgr->GetDCString(STRING_TELEPORT_CONDITIONS, LOCALE_zhCN);

        chatHandler.SendSysMessage(sDCMgr->BuildDCText(STRING_YOU_ARE_IN_COMBAT_OR_FLYING).c_str());

        player->GetSession()->SendNotification(msg);

        return false;
    }

    if (!CHECK_REQUIREMENT(player, definition->reqID) || !CHECK_COST(player, definition->costID) || !CheckVisible(player, definition))
    {
        msg = sDCMgr->GetDCString(STRING_TELEPORT_CONDITIONS, LOCALE_zhCN);
        chatHandler.SendSysMessage(msg);

        player->GetSession()->SendNotification(msg);

        return false;
    }

    if (definition->costID && sDCMgr->getConfigBool(DC_CONFIG_TELEPORT_CHECK_COST))
    {
        auto cost = sDCMgr->GetDCCost(definition->costID);

        if (!sDCMgr->DoCost(player, cost))
            return false;

        chatHandler.SendSysMessage(sDCMgr->BuildDCText(STRING_COST_INFO, sDCMgr->BuildCostText(cost, true, false).c_str()).c_str());
    }

    player->SaveRecallPosition();
    player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);

    return true;
}

bool OnTeleportGossIpSelect(Player* player, ObjectGuid guid, uint32 sender, uint32 action)
{
    switch (sender)
    {
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE:
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_NEXT_PAGE:
        {
            auto category = 0u;
            auto page     = 0u;
            DC::decompressUint32(action, category, page);

            page += (sender == GLOBAL_ITEM_CATEGORY + MENU_SENDER_PREV_PAGE ? (page ? -1 : 0) : 1);

            return SendTeleportMenuItems(player, category, guid, page);
        }
        case GLOBAL_ITEM_CATEGORY + MENU_SENDER_MAIN_PAGE:
            return SendTeleportMenuItems(player, DEFAULT_MENU_CATEGORY, guid);
        default:
            break;
    }

    player->CLOSE_GOSSIP_MENU();
    player->PlayerTalkClass->ClearMenus();

    auto def = sDCTeleportModule->GetTeleportDefinition(sender, action);
    if (!def)
    {
        TC_LOG_ERROR("dc.teleport", "Client send unknow teleport category. player [%s], category[%u]", player->GetName(), sender);
        return false;
    }

    if (def->subCategory)
        return SendTeleportMenuItems(player, def->subCategory, guid);

    return DoTeleport(player, def);
}

class TeleporterCreature : public CreatureScript
{
    public:
        TeleporterCreature():
            CreatureScript("dc_teleporter_creature")
        {
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            return SendTeleportMenuItems(player, DEFAULT_MENU_CATEGORY, creature->GetGUID());
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
        {
            return OnTeleportGossIpSelect(player, creature->GetGUID(), sender, action);
        }

        bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
        {
            return true;
        }

    private:
};

class TeleporterItem : public ItemScript
{
    public:
        TeleporterItem() :
            ItemScript("dc_teleporter_item")
        {
        }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
        {
            return SendTeleportMenuItems(player, DEFAULT_MENU_CATEGORY, item->GetGUID());
        }

        bool OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
        {
            return OnTeleportGossIpSelect(player, item->GetGUID(), sender, action);
        }

        bool OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) override
        {
            return true;
        }
};

class TeleporterGameObject : public GameObjectScript
{
    public:
        TeleporterGameObject() :
            GameObjectScript("dc_teleporter_gameobject")
        {

        }
};

void AddSC_DC_teleporter()
{
    new TeleporterCreature();
    new TeleporterItem();
    new TeleporterGameObject();
}