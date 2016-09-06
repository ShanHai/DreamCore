#include "Log.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "DCConfig.h"
#include "DCTeleportModule.h"
#include "DCComposeModule.h"
#include "DCVIPModule.h"
#include "DCTransmogModule.h"
#include "DCMgr.h"

DCModule::DCModule(const char *name):
    m_name(name)
{
    sDCMgr->AddModule(this);
}

DCMgr* DCMgr::instance()
{
    static DCMgr instance;
    return &instance;
}

void DCMgr::Initialize()
{
    LoadConfig();
    LoadDCStrings();
    LoadDCCosts();
    LoadDCRequirements();
    LoadDCIcons();
    LoadDCItems();
}

void DCMgr::LoadConfig()
{
    if (!sDCConfigMgr->Load("dreamcore.conf"))
        exit(0);

    TC_LOG_INFO("dc", "Using configuration file dreamcore.conf.");

    m_config_int[DC_CONFIG_TELEPORT_MENUITEMS_PER_PAGE]    = sDCConfigMgr->GetIntDefault("DC.Teleport.MenuItemsPerPage",       5);
    m_config_bool[DC_CONFIG_TELEPORT_REMIND]               = sDCConfigMgr->GetBoolDefault("DC.Teleport.EnableRemind",          false);
    m_config_bool[DC_CONFIG_TELEPORT_CHECK_REQ]            = sDCConfigMgr->GetBoolDefault("DC.Teleport.CheckReq",              true);
    m_config_bool[DC_CONFIG_TELEPORT_CHECK_COST]           = sDCConfigMgr->GetBoolDefault("DC.Teleport.CheckCost",             true);
    m_config_bool[DC_CONFIG_TELEPORT_CHECK_VISIBLE]        = sDCConfigMgr->GetBoolDefault("DC.Teleport.CheckVisible",          true);

    m_config_int[DC_CONFIG_COMPOSE_MENUITEMS_PER_PAGE]     = sDCConfigMgr->GetIntDefault("DC.Compose.MenuItemsPerPage",        5);
    m_config_int[DC_CONFIG_COMPOSE_DEFAULT_LOOT_LIMIT]     = sDCConfigMgr->GetIntDefault("DC.Compose.DefaultLootLimit",        1);
    m_config_int[DC_CONFIG_COMPOSE_MAX_LOOT_LIMIT]         = sDCConfigMgr->GetIntDefault("DC.Compose.MaxLootLimit",            5);
    m_config_bool[DC_CONFIG_COMPOSE_CHECK_REQ]             = sDCConfigMgr->GetBoolDefault("DC.Compose.CheckReq",               true);
    m_config_bool[DC_CONFIG_COMPOSE_CHECK_COST]            = sDCConfigMgr->GetBoolDefault("DC.Compose.CheckCost",              true);

    m_config_int[DC_CONFIG_VIP_START_VIPLEVEL]             = sDCConfigMgr->GetIntDefault("DC.Vip.StartVipLevel",               0);
    m_config_int[DC_CONFIG_VIP_START_POINTS]               = sDCConfigMgr->GetIntDefault("DC.Vip.StartPoints",                 0);
    m_config_int[DC_CONFIG_VIP_POINTS_ITEMENTRY]           = sDCConfigMgr->GetIntDefault("DC.Vip.Points.ItemEntry",            0);

    m_config_bool[DC_CONFIG_ONLINE_REWARD_EXP]             = sDCConfigMgr->GetBoolDefault("DC.Online.Reward.Exp",              false);
    m_config_int[DC_CONFIG_ONLINE_REWARD_EXP_TIMER]        = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Exp.Timer",        1800000);
    m_config_int[DC_CONFIG_ONLINE_REWARD_EXP_COUNT]        = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Exp.Count",        10000);
    m_config_int[DC_CONFIG_ONLINE_REWARD_EXP_MINLEVEL]     = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Exp.MinLevel",     1);
    m_config_int[DC_CONFIG_ONLINE_REWARD_EXP_MAXLEVEL]     = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Exp.MaxLevel",     80);

    m_config_bool[DC_CONFIG_ONLINE_REWARD_POINTS]          = sDCConfigMgr->GetBoolDefault("DC.Online.Reward.Points",           false);
    m_config_int[DC_CONFIG_ONLINE_REWARD_POINTS_TIMER]     = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Points.Timer",     3600000);
    m_config_int[DC_CONFIG_ONLINE_REWARD_POINTS_COUNT]     = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Points.Count",     10);
    m_config_int[DC_CONFIG_ONLINE_REWARD_POINTS_MINLEVEL]  = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Points.MinLevel",  1);
    m_config_int[DC_CONFIG_ONLINE_REWARD_POINTS_MAXLEVEL]  = sDCConfigMgr->GetIntDefault ("DC.Online.Reward.Points.MaxLevel",  80);

    m_config_int[DC_CONFIG_MAX_PLAYER_TALENTS]             = sDCConfigMgr->GetIntDefault ("DC.Max.PlayerTalents",              0);

    m_config_float[DC_CONFIG_DIFF_NORMAL]                  = sDCConfigMgr->GetFloatDefault("DC.Diff.Normal",                   1.0f);
    m_config_float[DC_CONFIG_DIFF_DUNGEON]                 = sDCConfigMgr->GetFloatDefault("DC.Diff.Dungeon",                  1.0f);
    m_config_float[DC_CONFIG_DIFF_RAID]                    = sDCConfigMgr->GetFloatDefault("DC.Diff.Raid",                     1.0f);

    if (!m_config_int[DC_CONFIG_MAX_PLAYER_TALENTS] && sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) > 9)
        m_config_int[DC_CONFIG_MAX_PLAYER_TALENTS] = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) - 9;
}

void DCMgr::LoadDCStrings()
{
    auto oldMSTime = getMSTime();

    // cleanup affected map part for reloading case
    m_DCStrings.clear();

    auto result = WorldDatabase.PQuery("SELECT entry, content_default, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8 FROM dc_strings");

    if (!result)
    {
        TC_LOG_ERROR("dc", ">> Loaded 0 dc strings. DB table dc_strings is empty.");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_DCStrings[fields[0].GetUInt32()];
        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
        {
            if (data.strings.size() <= size_t(i))
                data.strings.resize(i + 1);

            data.strings[i] = fields[i + 1].GetString();
        }
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc", ">> Loaded %u dc strings from table dc_strings in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCMgr::LoadDCCosts()
{
    auto oldMSTime = getMSTime();

    // cleanup affected map part for reloading case
    m_DCCosts.clear();

    auto result = WorldDatabase.PQuery("SELECT "
                                       //   0      1      2           3      4      5      6      7      8      9     10     11     12     13      14          15          16          17          18          19
                                       "entry, money, honor, arenaPoint, point, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, itemCount1, itemCount2, itemCount3, itemCount4, itemCount5, "
                                       //       20          21          22          23           24
                                       "itemCount6, itemCount7, itemCount8, itemCount9, itemCount10 "
                                       "FROM dc_costs");

    if (!result)
    {
        TC_LOG_ERROR("dc", ">> Loaded 0 dc costs. DB table dc_costs is empty.");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_DCCosts[fields[0].GetUInt32()];

        data.entry      = fields[0].GetUInt32();
        data.money      = fields[1].GetUInt32();
        data.honor      = fields[2].GetUInt32();
        data.arenaPoint = fields[3].GetUInt32();
        data.point      = fields[4].GetUInt32();

        for (uint8 i = 0; i < 10; ++i)
        {
            data.items[i]      = fields[i + 5].GetUInt32();
            data.itemsCount[i] = fields[i + 15].GetUInt32();

            if (data.items[i] && !sObjectMgr->GetItemTemplate(data.items[i]))
            {
                TC_LOG_ERROR("sql.sql", "Table dc_costs entry %u items%u is using non-existing item %u, ignored.", data.entry, i, data.items[i]);

                data.items[i]      = 0;
                data.itemsCount[i] = 0;
            }

            if (data.items[i] && !data.itemsCount[i] || !data.items[i] && data.itemsCount[i])
            {
                TC_LOG_ERROR("sql.sql", "Table dc_costs entry %u %s%u is %u but %s is zero, ignored.",
                    data.entry, data.items[i] ? "item" : "itemsCount", i, data.items[i] ? data.items[i] : data.itemsCount[i], data.items[i] ? "itemsCount" : "item");

                data.items[i]      = 0;
                data.itemsCount[i] = 0;
            }

            if (data.items[i])
                data.lastItemIndex = i + 1;
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc", ">> Loaded %u dc costs from table dc_costs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCMgr::LoadDCRequirements()
{
    auto oldMSTime = getMSTime();

    // cleanup affected map part for reloading case
    m_DCRequirements.clear();

    auto result = WorldDatabase.PQuery("SELECT entry, faction, vip, level FROM dc_requirements");

    if (!result)
    {
        TC_LOG_ERROR("dc", ">> Loaded 0 dc requirements. DB table dc_requirements is empty.");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_DCRequirements[fields[0].GetUInt32()];

        data.entry   = fields[0].GetUInt32();
        data.faction = fields[1].GetUInt8();
        data.vip     = fields[2].GetUInt16();
        data.level   = fields[3].GetUInt8();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc", ">> Loaded %u dc requirements from table dc_requirements in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCMgr::LoadDCIcons()
{
    auto oldMSTime = getMSTime();

    // cleanup affected map part for reloading case
    m_DCIcons.clear();

    auto result = WorldDatabase.PQuery("SELECT displayID, displayName FROM dc_icons");

    if (!result)
    {
        TC_LOG_ERROR("dc", ">> Loaded 0 dc icons. DB table dc_icons is empty.");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_DCIcons[fields[0].GetUInt32()];

        data.displayID   = fields[0].GetUInt32();
        data.displayName = fields[1].GetString();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc", ">> Loaded %u dc icons from table dc_icons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCMgr::LoadDCItems()
{
    auto oldMSTime = getMSTime();

    // cleanup affected map part for reloading case
    m_DCItems.clear();

    auto result = WorldDatabase.PQuery("SELECT entry, type, data0, data1, data2, data3, data4 FROM dc_items");

    if (!result)
    {
        TC_LOG_ERROR("dc", ">> Loaded 0 dc items. DB table dc_items is empty.");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_DCItems[fields[0].GetUInt32()];

        data.entry = fields[0].GetUInt32();
        data.type  = DCItemType(fields[1].GetUInt8());
        data.data0 = fields[2].GetInt32();
        data.data1 = fields[3].GetInt32();
        data.data2 = fields[4].GetInt32();
        data.data3 = fields[5].GetInt32();
        data.data4 = fields[6].GetInt32();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc", ">> Loaded %u dc items from table dc_items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

bool DCMgr::ReloadModule(const std::string& name)
{
    auto module = GetModule(name);
    if (!module)
        return false;

    TC_LOG_INFO("dc.module", "Reloading module [%s]...", name.c_str());

    module->Reload();

    TC_LOG_INFO("dc.module", "Module [%s] reloaded!", name.c_str());

    return true;
}

void DCMgr::ReloadAllModules()
{
    TC_LOG_INFO("dc.module", "Reloading all modules...");

    for (auto module : m_modules)
        module->Reload();

    TC_LOG_INFO("dc.module", "All modules reloaded!");
}

void DCMgr::AddModule(DCModule *const newModule)
{
    ASSERT(newModule);

    if (std::find(m_modules.begin(), m_modules.end(), newModule) == m_modules.end())
        m_modules.push_back(newModule);
}

const char *DCMgr::GetDCString(uint32 entry, LocaleConstant locale) const
{
    if (auto str = GetDCString(entry))
    {
        if (str->strings.size() > size_t(locale) && !str->strings[locale].empty())
            return str->strings[locale].c_str();

        return str->strings[DEFAULT_LOCALE].c_str();
    }

    if (entry > 0)
        TC_LOG_ERROR("sql.sql", "Entry %i not found in `dc_string` table.", entry);
    else
        TC_LOG_ERROR("sql.sql", "DCString entry %i not found in DB.", entry);
    return "<error>";
}

std::string DCMgr::BuildDCText(uint32 entry, ...)
{
    char text[2048];

    va_list ap;
    va_start(ap, entry);
    vsnprintf(text, 2048, GetDCString(entry, LOCALE_zhCN), ap);
    va_end(ap);

    return std::string(text);
}

std::string DCMgr::BuildMoneyText(uint32 money, bool tag)
{
    std::string text("");

    if (!money)
        return text;

    auto gold   = money / 10000;
    auto silver = money % 10000 / 100;
    auto copper = money % 100;

    if (gold)    { text.append(BuildDCText(STRING_GOLD, gold));     if (silver || !silver && copper) text.append(" "); }
    if (silver)  { text.append(BuildDCText(STRING_SILVER, silver)); if (copper) text.append(" "); }
    if (copper)  { text.append(BuildDCText(STRING_COPPER, copper)); }

    return tag ? BuildDCText(STRING_TAG_BRACKETS, text.c_str()) : text;
}

std::string DCMgr::BuildFullCostText(uint32 iD, bool icon, bool prefix, LocaleConstant locale)
{
    auto cost = GetDCCost(iD);

    if (!cost)
        return "";

    return BuildFullCostText(cost, icon, prefix, locale);
}

std::string DCMgr::BuildFullCostText(const DCCost* cost, bool icon, bool prefix, LocaleConstant locale)
{
    ASSERT(cost);

    auto info = std::string("");

    if (prefix)
        info.append("\r\n");

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
            if (ItemLocale const* il = sObjectMgr->GetItemLocale(item->ItemId))
                ObjectMgr::GetLocaleString(il->Name, locale, itemName);
        }

        info.append(BuildDCText(STRING_ITEM_INFO_COUNT, "ffffff", sDCMgr->GetDCIconName(item->DisplayInfoID), 30, 30, 0, cost->lastItemIndex == 1 ? 6 : -2, itemName.c_str(), "00ff00", cost->itemsCount[i]));
        info.append("\r\n");
    }

    auto extendInfo = BuildCostText(cost, true, false);
    if (extendInfo != "")
    {
        info.append(BuildDCText(STRING_AND));
        info.append("\r\n");
        info.append(extendInfo);
    }

    return prefix ? BuildDCText(STRING_THIS_MAY_TAKE, info.c_str()) : info;
}

std::string DCMgr::BuildCostText(uint32 iD, bool buildGold, bool prefix)
{
    auto cost = GetDCCost(iD);

    if (!cost)
        return "";

    return BuildCostText(cost->money, cost->honor, cost->arenaPoint, cost->point, buildGold, prefix);
}

std::string DCMgr::BuildCostText(const DCCost* cost, bool buildGold, bool prefix)
{
    ASSERT(cost);

    return BuildCostText(cost->money, cost->honor, cost->arenaPoint, cost->point, buildGold, prefix);
}

std::string DCMgr::BuildCostText(uint32 money, uint32 honor, uint32 arenaPoint, uint32 point, bool buildGold, bool prefix)
{
    std::string text = "";

    if (point)      text.append(BuildDCText(STRING_TAG_BRACKETS, BuildDCText(STRING_POINT, point).c_str()));
    if (honor)      text.append(BuildDCText(STRING_TAG_BRACKETS, BuildDCText(STRING_HONOR, honor).c_str()));
    if (arenaPoint) text.append(BuildDCText(STRING_TAG_BRACKETS, BuildDCText(STRING_ARENA, arenaPoint).c_str()));

    if (money)
    {
        if (buildGold)
            text.append(BuildMoneyText(money, true));
        else
        {
            if (text != "")
                text.append(BuildDCText(STRING_AND));
            else if (prefix)
                return BuildDCText(STRING_THIS_MAY_TAKE, "");
        }
    }

    return text == "" || !prefix ? text : BuildDCText(STRING_THIS_MAY_TAKE, text.c_str());
}

bool DCMgr::CheckRequirement(const Player* player, const DCRequirement* req)
{
    ASSERT(player);
    ASSERT(req);

    if (player->IsGameMaster())
        return true;

    return player->getLevel() >= req->level && (req->faction == 2 || player->GetTeamId() == req->faction);
}

bool DCMgr::CheckRequirement(const Player* player, uint32 reqID)
{
    ASSERT(player);

    if (player->IsGameMaster() || !reqID)
        return true;

    auto req = GetDCRequirement(reqID);

    return req && CheckRequirement(player, req);
}

bool DCMgr::CheckCost(const Player* player, const DCCost* cost)
{
    ASSERT(player);
    ASSERT(cost);

    if (player->IsGameMaster())
        return true;

    if (player->GetMoney() < cost->money)
        return false;

    if (player->GetHonorPoints() < cost->honor)
        return false;

    if (player->GetArenaPoints() < cost->arenaPoint)
        return false;

    for (auto i = 0u; i < cost->lastItemIndex; ++i)
    {
        auto item = cost->items[i];
        if (item && player->GetItemCount(item) < cost->itemsCount[i])
            return false;
    }

    return true;
}

bool DCMgr::CheckCost(const Player* player, uint32 iD)
{
    ASSERT(player);

    if (player->IsGameMaster() || !iD)
        return true;

    auto cost = GetDCCost(iD);

    return cost && CheckCost(player, cost);
}

bool DCMgr::DoCost(Player* player, uint32 iD)
{
    auto cost = GetDCCost(iD);

    return cost && DoCost(player, cost);
}

bool DCMgr::DoCost(Player* player, const DCCost* cost)
{
    if (!cost || !CheckCost(player, cost))
        return false;

    player->ModifyMoney(-int32(cost->money));
    player->ModifyHonorPoints(-int32(cost->honor));
    player->ModifyArenaPoints(-int32(cost->arenaPoint));

    for (uint8 i = 0; i < cost->lastItemIndex; ++i)
    {
        auto item = cost->items[i];
        if (item)
            player->DestroyItemCount(item, cost->itemsCount[i], true);
    }

    return true;
}

DCModule * DCMgr::GetModule(const std::string&  name)
{
    for (auto module : m_modules)
        if (module->getName() == name)
            return module;

    return nullptr;
}
