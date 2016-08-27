#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GossipDef.h"
#include "DCComposeModule.h"

DCComposeModule* DCComposeModule::instance()
{
    static DCComposeModule instance;
    return &instance;
}

DCComposeModule::DCComposeModule() :
    DCModule("compose")
{
    Initialize();
}

void DCComposeModule::ReloadDataBase()
{
    TC_LOG_INFO("dc.compose", "Reloading DCComposeModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.compose", "DCComposeModule reload complete...");
}

void DCComposeModule::Initialize()
{
    TC_LOG_INFO("dc.compose", "Initializing DCComposeModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.compose", "DCComposeModule initialized.");
}

void DCComposeModule::LoadDatabase()
{
    LoadComposeLoot();
    LoadComposeCategories();
    LoadComposeDefinitions();
}

void DCComposeModule::LoadComposeLoot()
{
    auto oldMSTime = getMSTime();

    m_composeLoot.clear();

    //                                         0     1       2          3         4         5      6
    auto result = WorldDatabase.Query("SELECT id, item, chance, lootGroup, minCount, maxCount, reqID FROM dc_compose_loot");

    if (!result)
    {
        TC_LOG_ERROR("dc.compose", ">> Loaded 0  compose loot. DB table `dc_compose_loot` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        ComposeLoot loot;

        loot.id        = fields[0].GetUInt32();
        loot.item      = fields[1].GetUInt32();
        loot.chance    = fields[2].GetFloat();
        loot.lootGroup = fields[3].GetUInt8();
        loot.minCount  = fields[4].GetUInt8();
        loot.maxCount  = fields[5].GetUInt8();
        loot.reqID     = fields[6].GetUInt32();

        if (!sObjectMgr->GetItemTemplate(loot.item))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_loot id %u item %u is using non-existing item %u, ignored.", loot.id, loot.item, loot.item);

            continue;
        }

        if (loot.reqID && !sDCMgr->GetDCRequirement(loot.reqID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_loot id %u item %u is using non-existing reqID %u, ignored.", loot.id, loot.item, loot.reqID);

            continue;
        }

        if (loot.chance == 0 && loot.lootGroup == 0)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_loot id %u item %u is zero chanced, but group not defined, ignored.", loot.id, loot.item);

            continue;
        }

        if (loot.chance != 0 && loot.chance <= 0.000001f)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_loot id %u is using invalid chance %f, use %f instead.", loot.id, loot.chance, 0.01f);

            loot.chance = 0.01f;
        }

        if (loot.minCount > loot.maxCount)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_loot id %u minCount(%u) > maxCount(%u), set minCount to %u.", loot.id, loot.minCount, loot.maxCount, loot.maxCount);

            loot.minCount = loot.maxCount;
        }

        m_composeLoot.insert(ComposeLootContainer::value_type(loot.id, loot));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.compose", ">> Loaded %u compose loot in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCComposeModule::LoadComposeCategories()
{
    auto oldMSTime = getMSTime();

    m_composeCategories.clear();

    //                                            0       1       2      3
    auto result = WorldDatabase.Query("SELECT entry, textID, costID, reqID FROM dc_compose_category");

    if (!result)
    {
        TC_LOG_ERROR("dc.compose", ">> Loaded 0  compose categories. DB table `dc_compose_category` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto& data = m_composeCategories[fields[0].GetUInt16()];

        data.entry  = fields[0].GetUInt16();
        data.textID = fields[1].GetUInt32();
        data.costID = fields[2].GetUInt32();
        data.reqID  = fields[3].GetUInt32();

        if (!sObjectMgr->GetGossipText(data.textID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_category entry %u is using non-existing textID %u, ignored.", data.entry, data.textID);

            m_composeLoot.erase(data.entry);

            continue;
        }

        if (data.costID && !sDCMgr->GetDCCost(data.costID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_category entry %u is using non-existing costID %u, use 0 instead.", data.entry, data.costID);

            data.costID = 0;
        }

        if (data.reqID && !sDCMgr->GetDCRequirement(data.reqID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_category entry %u is using non-existing reqID %u, use 0 instead.", data.entry, data.reqID);

            data.reqID = 0;
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.compose", ">> Loaded %u compose categories in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCComposeModule::LoadComposeDefinitions()
{
    auto oldMSTime = getMSTime();

    m_composeDefinitions.clear();

    auto result = WorldDatabase.Query(
        //             0   1       2           3     4       5       6      7               8                    9           10
        "SELECT category, id, iconID, iconExtend, text, lootID, costID, reqID, lootCountLimit, useCategorySettings, subCategory "
        "FROM dc_compose_definition ORDER BY category, id");

    if (!result)
    {
        TC_LOG_ERROR("dc.compose", ">> Loaded 0  compose definitions. DB table `dc_compose_definition` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        ComposeDefinition definition;

        definition.category             = fields[0].GetUInt16();
        definition.id                   = fields[1].GetUInt32();
        definition.iconID               = fields[2].GetUInt8();
        definition.iconExtend           = fields[3].GetString();
        definition.text                 = fields[4].GetString();
        definition.lootID               = fields[5].GetUInt32();
        definition.costID               = fields[6].GetUInt32();
        definition.reqID                = fields[7].GetUInt32();
        definition.lootCountLimit       = fields[8].GetUInt8();
        definition.useCategorySettings  = fields[9].GetBool();
        definition.subCategory          = fields[10].GetUInt16();

        if (m_composeLoot.find(definition.lootID) == m_composeLoot.end() && !definition.subCategory && definition.category != 1000)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_definition category %u id %u is using non-existing lootID %u, ignored.", definition.category, definition.id, definition.lootID);

            continue;
        }

        if (definition.costID && !sDCMgr->GetDCCost(definition.costID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_definition category %u id %u is using non-existing costID %u, use 0 instead.", definition.category, definition.id, definition.costID);

            definition.costID = 0;
        }

        if (definition.reqID && !sDCMgr->GetDCRequirement(definition.reqID))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_definition category %u id %u is using non-existing reqID %u, use 0 instead.", definition.category, definition.id, definition.reqID);

            definition.reqID = 0;
        }

        if (definition.iconID >= GOSSIP_ICON_MAX)
        {
            TC_LOG_ERROR("sql.sql", "Table dc_compose_definition for category %u, id %u has unknown icon id %u. Replacing with GOSSIP_ICON_CHAT", definition.category, definition.id, definition.iconID);
            definition.iconID = GOSSIP_ICON_CHAT;
        }

        if (definition.lootCountLimit > sDCMgr->getConfigInt(DC_CONFIG_COMPOSE_MAX_LOOT_LIMIT))
            definition.lootCountLimit = sDCMgr->getConfigInt(DC_CONFIG_COMPOSE_MAX_LOOT_LIMIT);

        char text[1024];
        auto size = definition.iconExtend.size();
        memcpy(&text, definition.iconExtend.c_str(), size);
        sprintf(&text[size], "%s", definition.text.c_str());
        definition.text = std::string(text);

        m_composeDefinitions.insert(ComposeDefinitionContainer::value_type(definition.category, definition));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.compose", ">> Loaded %u compose definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
