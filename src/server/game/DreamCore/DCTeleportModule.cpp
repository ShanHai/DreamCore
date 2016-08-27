#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GossipDef.h"
#include "DCTeleportModule.h"

DCTeleportModule* DCTeleportModule::instance()
{
    static DCTeleportModule instance;
    return &instance;
}

DCTeleportModule::DCTeleportModule():
    DCModule("teleport")
{
    TC_LOG_INFO("dc.teleport", "Initializing DCTeleportModule...");

    Initialize();

    TC_LOG_INFO("dc.teleport", "DCTeleportModule initialized.");
}

void DCTeleportModule::ReloadDataBase()
{
    TC_LOG_INFO("dc.teleport", "Reloading DCTeleportModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.teleport", "DCTeleportModule reload complete...");
}

void DCTeleportModule::Initialize()
{
    LoadDatabase();
}

void DCTeleportModule::LoadDatabase()
{
    LoadTeleportCategories();
    LoadTeleportDefinitions();
}

void DCTeleportModule::LoadTeleportCategories()
{
    auto oldMSTime = getMSTime();

    m_teleportCategories.clear();

    auto result = WorldDatabase.Query("SELECT entry, textID FROM dc_teleport_category");

    if (!result)
    {
        TC_LOG_ERROR("dc.teleport", ">> Loaded 0  teleport category entries. DB table `dc_teleport_category` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& category = m_teleportCategories[fields[0].GetUInt16()];

        category.entry  = fields[0].GetUInt16();
        category.textID = fields[1].GetUInt32();

        if (!sObjectMgr->GetGossipText(category.textID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_teleport_category entry %u is using non-existing textID %u, ignored.", category.entry, category.textID);
            continue;
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.teleport", ">> Loaded %u teleport categories in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCTeleportModule::LoadTeleportDefinitions()
{
    auto oldMSTime = getMSTime();

    m_teleportDefinitions.clear();

    auto result = WorldDatabase.Query(
        //             0   1       2           3     4       5            6        7       8      9
        "SELECT category, id, iconID, iconExtend, text, teleID, subCategory, boxText, costID, reqID "
        "FROM dc_teleport_definition ORDER BY category, id");

    if (!result)
    {
        TC_LOG_ERROR("dc.teleport", ">> Loaded 0 teleport definition entries. DB table `dc_teleport_definition` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        TeleportDefinition definition;

        definition.category     = fields[0].GetUInt16();
        definition.id           = fields[1].GetUInt16();
        definition.iconID       = fields[2].GetUInt8();
        definition.iconExtend   = fields[3].GetString();
        definition.text         = fields[4].GetString();
        definition.teleID       = fields[5].GetUInt32();
        definition.subCategory  = fields[6].GetUInt16();
        definition.boxText      = fields[7].GetString();
        definition.costID       = fields[8].GetUInt32();
        definition.reqID        = fields[9].GetUInt32();
        definition.customText   = definition.boxText != "";

        auto tele = sObjectMgr->GetGameTele(definition.teleID);

        if (!tele && !definition.subCategory && definition.category != 1000)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_teleport_definition id %u has unknown tele id %u. ignored", definition.id, definition.teleID);
            continue;
        }

        if (definition.costID && !sDCMgr->GetDCCost(definition.costID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_teleport_definition id %u has unknown cost id %u. ignored", definition.id, definition.costID);
            continue;
        }

        if (definition.reqID && !sDCMgr->GetDCRequirement(definition.reqID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_teleport_definition id %u has unknown requirement id %u. ignored", definition.id, definition.reqID);
            continue;
        }

        if (definition.iconID >= GOSSIP_ICON_MAX)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_teleport_definition for category %u, id %u has unknown icon id %u. Replacing with GOSSIP_ICON_CHAT", definition.category, definition.id, definition.iconID);
            definition.iconID = GOSSIP_ICON_CHAT;
        }

        char text[1024];
        auto size = definition.iconExtend.size();
        memcpy(&text, definition.iconExtend.c_str(), size);
        sprintf(&text[size], definition.text.c_str(), tele ? tele->name.c_str() : "");
        definition.text = std::string(text);

        if (!definition.subCategory && definition.category != 1000 && definition.boxText == "")
        {
            definition.boxText = sDCMgr->BuildDCText(STRING_TELEPORT_REMIND, tele->name.c_str());
            definition.boxText.append(sDCMgr->BuildCostText(definition.costID, false));
        }

        m_teleportDefinitions.insert(TeleportDefinitionContainer::value_type(definition.category, definition));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.teleport", ">> Loaded %u teleport definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
