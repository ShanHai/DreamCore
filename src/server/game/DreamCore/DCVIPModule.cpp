#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GossipDef.h"
#include "DCVIPModule.h"

DCVIPModule* DCVIPModule::instance()
{
    static DCVIPModule instance;
    return &instance;
}

DCVIPModule::DCVIPModule() :
    DCModule("vip")
{
    Initialize();
}

void DCVIPModule::ReloadDataBase()
{
    TC_LOG_INFO("dc.vip", "Reloading DCVIPModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.vip", "DCVIPModule reload complete...");
}

void DCVIPModule::Initialize()
{
    TC_LOG_INFO("dc.vip", "Initializing DCVIPModule...");

    LoadDatabase();

    TC_LOG_INFO("dc.vip", "DCVIPModule initialized.");
}

void DCVIPModule::LoadDatabase()
{
    LoadVipVendors();
}

void DCVIPModule::LoadVipVendors()
{
    auto oldMSTime = getMSTime();

    m_vipVendors.clear();

    auto result = WorldDatabase.Query("SELECT entry FROM dc_vip_vendor");

    if (!result)
    {
        TC_LOG_ERROR("dc.vip", ">> Loaded 0  vip vendor entries. DB table `dc_vip_vendor` is empty!");
        return;
    }

    auto count = 0u;

    do
    {
        auto fields = result->Fetch();

        auto entry = fields[0].GetUInt32();

        auto creature = sObjectMgr->GetCreatureTemplate(entry);
        if (!creature || !(creature->npcflag & UNIT_NPC_FLAG_VENDOR))
        {
            TC_LOG_ERROR("sql.sql", "Table dc_vip_vendor entry %u is not a vendor or not exists, ignored.", entry);
            continue;
        }

        m_vipVendors.push_back(entry);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("dc.vip", ">> Loaded %u vip vendors in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void DCVIPModule::ChangeAccountVipLevel(uint32 accountID, uint32 vipLevel, int32 realmID)
{
    auto stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_VIPLEVEL_BY_REALMID);
    stmt->setUInt32(0, accountID);
    stmt->setUInt32(1, realmID);

    auto result = LoginDatabase.Query(stmt);
    if (!result)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_VIP_POINTS);
        stmt->setUInt32(0, accountID);
        stmt->setUInt32(1, vipLevel);
        stmt->setUInt32(2, 0);
        stmt->setUInt32(3, realmID);
    }
    else
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_VIPLEVEL);
        stmt->setUInt32(0, vipLevel);
        stmt->setUInt32(1, accountID);
    }

    LoginDatabase.Execute(stmt);
}

void DCVIPModule::ChangeAccountPoints(uint32 accountID, uint32 points, int32 realmID)
{
    auto stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_POINTS_BY_REALMID);
    stmt->setUInt32(0, accountID);
    stmt->setUInt32(1, realmID);

    auto result = LoginDatabase.Query(stmt);
    if (!result)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_VIP_POINTS);
        stmt->setUInt32(0, accountID);
        stmt->setUInt32(1, 0);
        stmt->setUInt32(2, points);
        stmt->setUInt32(3, realmID);
    }
    else
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_POINTS);
        stmt->setUInt32(0, points);
        stmt->setUInt32(1, accountID);
    }

    LoginDatabase.Execute(stmt);
}

void DCVIPModule::ModifyAccountPoints(uint32 accountID, int32 points, int32 realmID)
{
    auto stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_POINTS_BY_REALMID);
    stmt->setUInt32(0, accountID);
    stmt->setUInt32(1, realmID);

    auto result = LoginDatabase.Query(stmt);
    if (!result)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_VIP_POINTS);
        stmt->setUInt32(0, accountID);
        stmt->setUInt32(1, 0);
        stmt->setUInt32(2, points < 0 ? 0 : points);
        stmt->setUInt32(3, realmID);
    }
    else
    {
        auto fields = result->Fetch();
        auto newPoints = fields[0].GetInt32() + points;

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_POINTS);
        stmt->setUInt32(0, newPoints < 0 ? 0 : newPoints);
        stmt->setUInt32(1, accountID);
    }

    LoginDatabase.Execute(stmt);
}
